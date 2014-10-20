/** @file camera.cpp

    @brief Camera Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

//#include <QDebug>

#include "camera.h"
#include "gl/context.h"
#include "io/datastream.h"
#include "gl/cameraspace.h"
#include "gl/framebufferobject.h"
#include "io/log.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "scene.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "param/parameterint.h"
#include "math/cubemapmatrix.h"
#include "math/vector.h"
#include "io/settings.h"
#include "projection/projectionsystemsettings.h"
#include "projection/projectormapper.h"
#include "geom/geometry.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(Camera)

Camera::Camera(QObject *parent) :
    ObjectGl        (parent),
    renderMode_     (RM_FULLDOME_CUBE),
    sliceCameraSettings_(new CameraSettings())
{
    setName("Camera");

    setCreateRenderSettings(false);
}

Camera::~Camera()
{
    delete sliceCameraSettings_;

    for (auto i : fbo_)
        delete i;
    for (auto i : screenQuad_)
        delete i;
}

void Camera::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);
    io.writeHeader("cam", 1);
}

void Camera::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);
    io.readHeader("cam", 1);
}

void Camera::createParameters()
{
    ObjectGl::createParameters();

    beginParameterGroup("camera", tr("camera settings"));

    p_cameraMode_ = createSelectParameter("cammode", tr("render mode"),
                                        tr("Selects the display/render mode for this camera"),
    { "ortho", "persp", "fdcube", "slice" },
    { tr("orthographic"), tr("perspective"), tr("fulldome cube"), tr("projector slice") },
    { tr("Orthographic projection for flat screens"),
      tr("Perspective projection for flat screens"),
      tr("Fulldome projection by means of 5 (or 6) cameras"),
      tr("Warped projector slice - for multi-machine/projector setups")},
    { RM_ORTHOGRAPHIC, RM_PERSPECTIVE, RM_FULLDOME_CUBE, RM_PROJECTOR_SLICE },
                                        RM_FULLDOME_CUBE,
                                        true, false
                                        );
    p_cameraAngle_ = createFloatParameter("camangle", tr("field of view"),
                                        tr("Specifies the vertical openening angle in degree"),
                                        180.f,
                                        1.f, 360.f, 1.f);

    p_cameraOrthoScale_ = createFloatParameter("camorthosc", tr("orthographic scale"),
                                        tr("Scale (+/-) of the orthographic projection on x,y axis"),
                                        10.f,
                                        0.0001f, 1000000.f, 0.1f);

    p_width_ = createIntParameter("fbowidth", tr("width"), tr("Width of rendered frame in pixels"),
                                  1024, 16, 4096*4, 16, true, false);
    p_height_ = createIntParameter("fboheight", tr("height"), tr("Height of rendered frame in pixels"),
                                  1024, 16, 4096*4, 16, true, false);
    p_cubeRes_ = createIntParameter("fbocuberes", tr("size of cube map"), tr("Width and height of the rendered frame per cube map"),
                                  1024, 16, 4096*4, 16, true, false);

    endParameterGroup();

    beginParameterGroup("camback", tr("background"));

        p_backR_ = createFloatParameter("cambackr", tr("red"),
                                      tr("Red amount of background color"),
                                      0.0, 0.0, 1.0, 0.1);
        p_backG_ = createFloatParameter("cambackg", tr("green"),
                                      tr("Red amount of background color"),
                                      0.0, 0.0, 1.0, 0.1);
        p_backB_ = createFloatParameter("cambackb", tr("blue"),
                                      tr("Red amount of background color"),
                                      0.0, 0.0, 1.0, 0.1);
        p_backA_ = createFloatParameter("cambacka", tr("red"),
                                      tr("Alpha amount of background color"),
                                      1.0, 0.0, 1.0, 0.1);

    endParameterGroup();

    beginParameterGroup("output", tr("output"));

    p_cameraMix_ = createFloatParameter("cammix", tr("camera mix"),
                          tr("Defines the volume and visibility of the camera [0,1]"),
                          1.f,
                          0.f, 1.f, 0.05f);
}

void Camera::onParameterChanged(Parameter * p)
{
    ObjectGl::onParameterChanged(p);

    if (p == p_cameraMode_)
    {
        renderMode_ = (RenderMode)p_cameraMode_->baseValue();
        requestReinitGl();
    }

    if (p == p_width_ || p == p_height_ || p == p_cubeRes_)
        requestReinitGl();
}

void Camera::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();
    renderMode_ = (RenderMode)p_cameraMode_->baseValue();
}

void Camera::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    const bool
            isortho = p_cameraMode_->baseValue() == RM_ORTHOGRAPHIC,
            isslice = p_cameraMode_->baseValue() == RM_PROJECTOR_SLICE,
            iscube = p_cameraMode_->baseValue() == RM_FULLDOME_CUBE;

    p_cameraOrthoScale_->setVisible( isortho );
    p_cameraAngle_->setVisible( !isortho && !isslice );
    p_width_->setVisible( !iscube );
    p_height_->setVisible( !iscube );
    p_cubeRes_->setVisible( iscube );
}

void Camera::setNumberThreads(uint num)
{
    ObjectGl::setNumberThreads(num);

    uint oldnum = fbo_.size();

    fbo_.resize(num);
    screenQuad_.resize(num);

    for (uint i=oldnum; i<num; ++i)
    {
        fbo_[i] = 0;
        screenQuad_[i] = 0;
    }
}

void Camera::setBufferSize(uint bufferSize, uint thread)
{
    ObjectGl::setBufferSize(bufferSize, thread);
}

void Camera::initGl(uint thread)
{
    const Scene * scene = sceneObject();
    MO_ASSERT(scene, "Camera::initGl() without scene object");

    const bool
            cubeMapped = renderMode_ == RM_FULLDOME_CUBE,
            sliced = renderMode_ == RM_PROJECTOR_SLICE;

    *sliceCameraSettings_ = settings->cameraSettings();

    int width = p_width_->baseValue(),
        height = p_height_->baseValue();

    if (cubeMapped)
    {
        width = height = p_cubeRes_->baseValue();
    }
    else if (sliced)
    {
        width = sliceCameraSettings_->width();
        height = sliceCameraSettings_->height();
    }

    //MO_DEBUG("Camera fbo = " << width << "x" << height);

    // projection matrix

    aspectRatio_ = (Float)width/std::max(1, height);

    // warped quad for slice

    GEOM::Geometry * warped_quad = 0;
    if (sliced)
    {
        warped_quad = new GEOM::Geometry();
        ProjectorMapper m;
        m.setSettings(settings->domeSettings(), settings->projectorSettings());
        m.getWarpGeometry(settings->cameraSettings(), warped_quad);
    }

    // screen-quad

    screenQuad_[thread] = new GL::ScreenQuad(idName() + "_quad", GL::ER_THROW);
    screenQuad_[thread]->create(
                ":/shader/framebuffercamera.vert",
                ":/shader/framebuffercamera.frag",
                cubeMapped? "#define MO_FULLDOME_CUBE" : "",
                warped_quad);

    // uniforms

    uColor_ = screenQuad_[thread]->shader()->getUniform("u_color", true);
    uColor_->setFloats(1,1,1,1);
    if (cubeMapped)
        uAngle_ = screenQuad_[thread]->shader()->getUniform("u_angle", true);

    // create framebuffer

    fbo_[thread] = new GL::FrameBufferObject(
                width,
                height,
                gl::GLenum(scene->frameBufferFormat()),
                gl::GL_FLOAT,
                GL::FrameBufferObject::A_DEPTH,
                cubeMapped,
                GL::ER_THROW);

    fbo_[thread]->create();
    fbo_[thread]->unbind();

    if (renderMode_ == RM_PROJECTOR_SLICE)
    {
        sliceMatrix_ = settings->cameraSettings().getViewMatrix();
    }

    if (sceneObject())
        emit sceneObject()->CameraFboChanged(this);
}

void Camera::releaseGl(uint thread)
{
    screenQuad_[thread]->release();
    delete screenQuad_[thread];
    screenQuad_[thread] = 0;

    fbo_[thread]->release();
    delete fbo_[thread];
    fbo_[thread] = 0;
}

void Camera::initCameraSpace(GL::CameraSpace &cam, uint thread, Double time) const
{
    cam.setSize(fbo_[thread]->width(), fbo_[thread]->height());

    if (renderMode_ == RM_FULLDOME_CUBE)
    {
        cam.setFieldOfView(90.f);
        cam.setProjectionMatrix(
                    MATH::perspective(90.f, aspectRatio_, 0.01f, 1000.0f)
                    );

        cam.setIsCubemap(true);
    }
        else cam.setIsCubemap(false);

    if (renderMode_ == RM_PERSPECTIVE)
    {
        const Float angle = std::min((Double)179, p_cameraAngle_->value(time, thread));
        cam.setFieldOfView(angle);
        cam.setProjectionMatrix(
                    MATH::perspective(angle, aspectRatio_, 0.01f, 1000.0f));
    }

    if (renderMode_ == RM_ORTHOGRAPHIC)
    {
        const Float sc = p_cameraOrthoScale_->value(time, thread);
        cam.setFieldOfView(90.); // XXX ???
        cam.setProjectionMatrix(
                    glm::ortho(-sc * aspectRatio_, sc * aspectRatio_, -sc, sc, 0.001f, 1000.f));
    }

    if (renderMode_ == RM_PROJECTOR_SLICE)
    {
        cam.setFieldOfView(sliceCameraSettings_->fov());
        cam.setProjectionMatrix(sliceCameraSettings_->getProjectionMatrix());
    }
}

uint Camera::numCubeTextures(uint thread, Double time) const
{
    if (renderMode_ != RM_FULLDOME_CUBE)
        return 1;

    return (p_cameraAngle_->value(time, thread) >= 250.f)
               ? 6 : 5;

}

const Mat4& Camera::cameraViewMatrix(uint index) const
{
    if (renderMode_ == RM_PROJECTOR_SLICE)
        return sliceMatrix_;

    if (renderMode_ != RM_FULLDOME_CUBE)
        return MATH::CubeMapMatrix::identity;

    switch (index)
    {
        case 0:  return MATH::CubeMapMatrix::positiveX; break;
        case 1:  return MATH::CubeMapMatrix::negativeX; break;
        case 2:  return MATH::CubeMapMatrix::positiveY; break;
        case 3:  return MATH::CubeMapMatrix::negativeY; break;
        case 4:  return MATH::CubeMapMatrix::negativeZ; break;
        default: return MATH::CubeMapMatrix::positiveZ; break;
    }
}

void Camera::startGlFrame(uint thread, Double time, uint cubeMapIndex)
{
    GL::FrameBufferObject * fbo = fbo_[thread];
    fbo->bind();

    if (renderMode_ == RM_FULLDOME_CUBE)
    {
        switch (cubeMapIndex)
        {
            case 0:  fbo->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X); break;
            case 1:  fbo->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_X); break;
            case 2:  fbo->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Y); break;
            case 3:  fbo->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y); break;
            case 4:  fbo->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z); break;
            default: fbo->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Z); break;
        }
    }

    //int pixelsize = 1; //devicePixelRatio(); // Retina support
    MO_DEBUG_GL("Camera::startGlFrame(uint thread, Double time, uint cubeMapIndex)")
    MO_CHECK_GL( glViewport(0, 0, fbo->width(), fbo->height()) );

    MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );
    MO_CHECK_GL( glDepthMask(GL_TRUE) );

    MO_CHECK_GL( glClearColor(p_backR_->value(time, thread),
                              p_backG_->value(time, thread),
                              p_backB_->value(time, thread),
                              p_backA_->value(time, thread)) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

}

void Camera::finishGlFrame(uint thread, Double)
{
    fbo_[thread]->unbind();
    //MO_CHECK_GL( glViewport(0, 0, glContext(thread)->size().width(), glContext(thread)->size().height()) );
}

GL::FrameBufferObject * Camera::fbo(uint thread) const
{
    if (thread < fbo_.size())
    {
        if (!fbo_[thread])
            MO_WARNING("request for camera fbo [" << thread << "], but fbo is not created yet");
        return fbo_[thread];
    }
    else
    {
        MO_WARNING("request for camera framebuffer[" << thread << "] "
                   "is out of range (" << fbo_.size() << ")");
        return 0;
    }
}

void Camera::drawFramebuffer(uint thread, Double time)
{
    const GL::FrameBufferObject * scenefbo = sceneObject()->fboMaster(thread);

    GL::FrameBufferObject * fbo = fbo_[thread];

    uColor_->floats[3] = p_cameraMix_->value(time, thread);
    if (renderMode_ == RM_FULLDOME_CUBE)
        uAngle_->floats[0] = p_cameraAngle_->value(time, thread);

    fbo->colorTexture()->bind();
    MO_CHECK_GL( glEnable(GL_BLEND) );
    MO_CHECK_GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
    screenQuad_[thread]->drawCentered(scenefbo->width(), scenefbo->height(), aspectRatio_);
    fbo->colorTexture()->unbind();
}


} // namespace MO
