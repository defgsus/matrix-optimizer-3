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
#include "math/cubemapmatrix.h"

namespace MO {

MO_REGISTER_OBJECT(Camera)

Camera::Camera(QObject *parent) :
    ObjectGl        (parent),
    renderMode_     (RM_FULLDOME_CUBE)
{
    setName("Camera");

    setCreateRenderSettings(false);
}

Camera::~Camera()
{
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

    cameraMode_ = createSelectParameter("cammode", tr("render mode"),
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
    cameraAngle_ = createFloatParameter("camangle", tr("field of view"),
                                        tr("Specifies the vertical openening angle in degree"),
                                        180.f,
                                        1.f, 360.f, 1.f);

    cameraOrthoScale_ = createFloatParameter("camorthosc", tr("orthographic scale"),
                                        tr("Scale (+/-) of the orthographic projection on x,y axis"),
                                        10.f,
                                        0.0001f, 1000000.f, 0.1f);

    endParameterGroup();

    beginParameterGroup("output", tr("output"));

    cameraMix_ = createFloatParameter("cammix", tr("camera mix"),
                          tr("Defines the volume and visibility of the camera [0,1]"),
                          1.f,
                          0.f, 1.f, 0.05f);
}

void Camera::onParameterChanged(Parameter * p)
{
    ObjectGl::onParameterChanged(p);

    if (p == cameraMode_)
    {
        renderMode_ = (RenderMode)cameraMode_->baseValue();
        requestReinitGl();
    }
}

void Camera::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();
    renderMode_ = (RenderMode)cameraMode_->baseValue();
}

void Camera::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    cameraOrthoScale_->setVisible( cameraMode_->baseValue() == RM_ORTHOGRAPHIC );
    cameraAngle_->setVisible( cameraMode_->baseValue() != RM_ORTHOGRAPHIC &&
                              cameraMode_->baseValue() != RM_PROJECTOR_SLICE );
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

    bool cubeMapped = renderMode_ == RM_FULLDOME_CUBE;

    const int width = cubeMapped?
                          scene->frameBufferCubeMapWidth()
                        : scene->frameBufferWidth();
    const int height = cubeMapped?
                          scene->frameBufferCubeMapHeight()
                        : scene->frameBufferHeight();

    // projection matrix

    aspectRatio_ = (Float)width/std::max(1, height);

    // screen-quad

    screenQuad_[thread] = new GL::ScreenQuad(idName() + "_quad", GL::ER_THROW);
    screenQuad_[thread]->create(
                ":/shader/framebuffercamera.vert",
                ":/shader/framebuffercamera.frag",
                cubeMapped? "#define MO_FULLDOME_CUBE" : "");

    // uniforms

    uColor_ = screenQuad_[thread]->shader()->getUniform("u_color", true);
    uColor_->setFloats(1,1,1,1);
    if (cubeMapped)
        uAngle_ = screenQuad_[thread]->shader()->getUniform("u_angle", true);

    // create framebuffer

    fbo_[thread] = new GL::FrameBufferObject(
                width,
                height,
                scene->frameBufferFormat(),
                GL_FLOAT,
                cubeMapped,
                GL::ER_THROW);

    fbo_[thread]->create();
    fbo_[thread]->unbind();

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

    Float angle = 90.f;
    if (renderMode_ == RM_PERSPECTIVE)
    {
        angle = std::min((Double)179, cameraAngle_->value(time, thread));
        cam.setIsCubemap(false);
    }
    else cam.setIsCubemap(true);

    cam.setFieldOfView(angle);

    if (renderMode_ == RM_ORTHOGRAPHIC)
    {
        const Float sc = cameraOrthoScale_->value(time, thread);
        cam.setProjectionMatrix(
                    glm::ortho(-sc * aspectRatio_, sc * aspectRatio_, -sc, sc, 0.001f, 1000.f));
    }
    else
    cam.setProjectionMatrix(
                glm::perspective(angle,
                                aspectRatio_,
                                0.01f, 1000.0f)
                );
}

uint Camera::numCubeTextures(uint thread, Double time) const
{
    if (renderMode_ != RM_FULLDOME_CUBE)
        return 1;

    return (cameraAngle_->value(time, thread) >= 250.f)
               ? 6 : 5;

}

const Mat4& Camera::cubeMapMatrix(uint index) const
{
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

void Camera::startGlFrame(uint thread, Double , uint cubeMapIndex)
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

    MO_CHECK_GL( glViewport(0, 0, fbo->width(), fbo->height()) );

    MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );
    MO_CHECK_GL( glDepthMask(true) );

    MO_CHECK_GL( glClearColor(0,0.2,0.2,1) );
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
    GL::FrameBufferObject * fbo = fbo_[thread];

    uColor_->floats[3] = cameraMix_->value(time, thread);
    if (renderMode_ == RM_FULLDOME_CUBE)
        uAngle_->floats[0] = cameraAngle_->value(time, thread);

    fbo->colorTexture()->bind();
    MO_CHECK_GL( glEnable(GL_BLEND) );
    MO_CHECK_GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
    screenQuad_[thread]->draw(fbo->width(), fbo->height());
    fbo->colorTexture()->unbind();
}


} // namespace MO
