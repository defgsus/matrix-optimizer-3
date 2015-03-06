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
#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "param/parameterint.h"
#include "math/cubemapmatrix.h"
#include "math/vector.h"
#include "projection/projectionsystemsettings.h"
#include "projection/projectormapper.h"
#include "projection/projectorblender.h"
#include "geom/geometry.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(Camera)

Camera::Camera(QObject *parent) :
    ObjectGl        (parent),
    renderMode_     (RM_FULLDOME_CUBE),
    alphaBlend_     (this),
    blendTexture_   (0)
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

    params()->beginParameterGroup("camera", tr("camera settings"));
    initParameterGroupExpanded("camera");

        p_cameraMode_ = params()->createSelectParameter("cammode", tr("render mode"),
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
        p_cameraAngle_ = params()->createFloatParameter("camangle", tr("field of view"),
                                            tr("Specifies the vertical openening angle in degree"),
                                            60.0,
                                            0.001, 179.999, 1.0);

        p_cameraFdAngle_ = params()->createFloatParameter("camanglefd", tr("field of view"),
                                            tr("Specifies the fisheye opening angle in degree"),
                                            180.0,
                                            1.0, 360.0, 1.0);

        p_cameraOrthoScale_ = params()->createFloatParameter("camorthosc", tr("orthographic scale"),
                                            tr("Visible area of the orthographic projection on x,y plane"),
                                            10.0,
                                            0.0001, 1000000.0, 0.1);

        p_cameraOrthoMix_ = params()->createFloatParameter("camorthomix", tr("orthographic mix"),
                                            tr("Mix between perspective and orthographic projection, range [0,1]"),
                                            0.0,
                                            0.0, 1.0, 0.0125);

        p_near_ = params()->createFloatParameter("camnear", tr("near plane"),
                                            tr("Near plane of camera frustum - everything closer can not be drawn"),
                                            0.001,
                                            0.00001, 100000.0, 0.05);
        p_far_ = params()->createFloatParameter("camfar", tr("far plane"),
                                            tr("Far plane of camera frustum - everything farther away can not be drawn"),
                                            1000.0,
                                            0.00002, 100000.0, 1.0);


        p_width_ = params()->createIntParameter("fbowidth", tr("width"), tr("Width of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);
        p_height_ = params()->createIntParameter("fboheight", tr("height"), tr("Height of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);
        p_cubeRes_ = params()->createIntParameter("fbocuberes", tr("size of cube map"), tr("Width and height of the rendered frame per cube map"),
                                      1024, 16, 4096*4, 16, true, false);

    params()->endParameterGroup();

    params()->beginParameterGroup("camback", tr("background"));

        p_backR_ = params()->createFloatParameter("cambackr", tr("red"),
                                      tr("Red amount of background color"),
                                      0.0, 0.0, 1.0, 0.1);
        p_backG_ = params()->createFloatParameter("cambackg", tr("green"),
                                      tr("Red amount of background color"),
                                      0.0, 0.0, 1.0, 0.1);
        p_backB_ = params()->createFloatParameter("cambackb", tr("blue"),
                                      tr("Red amount of background color"),
                                      0.0, 0.0, 1.0, 0.1);
        p_backA_ = params()->createFloatParameter("cambacka", tr("alpha"),
                                      tr("Alpha amount of background color"),
                                      1.0, 0.0, 1.0, 0.1);

    params()->endParameterGroup();

    params()->beginParameterGroup("output", tr("output"));

        p_cameraMix_ = params()->createFloatParameter("cammix", tr("camera mix"),
                          tr("Defines the volume and visibility of the camera [0,1]"),
                          1.0,
                          0.0, 1.0, 0.05);

        alphaBlend_.createParameters(AlphaBlendSetting::M_MIX, false, "_camout");

        p_magInterpol_ = params()->createBooleanParameter("cammaginterpol", tr("interpolation"),
                                                tr("The interpolation mode for pixel magnification"),
                                                tr("No interpolation"),
                                                tr("Linear interpolation"),
                                                true,
                                                true, false);
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
            ispersp = p_cameraMode_->baseValue() == RM_PERSPECTIVE,
            isslice = p_cameraMode_->baseValue() == RM_PROJECTOR_SLICE,
            iscube = p_cameraMode_->baseValue() == RM_FULLDOME_CUBE;

    p_cameraOrthoScale_->setVisible( isortho );
    p_cameraOrthoMix_->setVisible( ispersp );
    p_cameraAngle_->setVisible( !isortho && !isslice && !iscube);
    p_cameraFdAngle_->setVisible( iscube );
    p_width_->setVisible( !iscube && !isslice);
    p_height_->setVisible( !iscube && !isslice );
    p_cubeRes_->setVisible( iscube );
    p_cameraMix_->setVisible( alphaBlend_.hasAlpha() );
    p_near_->setVisible( !isslice );
    p_far_->setVisible( !isslice );
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

void Camera::initGl(uint thread)
{
    const Scene * scene = sceneObject();
    MO_ASSERT(scene, "Camera::initGl() without scene object");

    const bool
            cubeMapped = renderMode_ == RM_FULLDOME_CUBE,
            sliced = renderMode_ == RM_PROJECTOR_SLICE;

    const ProjectionSystemSettings& projset = scene->projectionSettings();
    const ProjectorSettings& proj = projset.projectorSettings(scene->projectorIndex());
    const CameraSettings& projcam = projset.cameraSettings(scene->projectorIndex());

    // size of camera frame

    int width = p_width_->baseValue(),
        height = p_height_->baseValue();

    if (cubeMapped)
    {
        width = height = p_cubeRes_->baseValue();
    }
    else if (sliced)
    {
        width = projcam.width();
        height = projcam.height();
    }

    aspectRatio_ = (Float)width/std::max(1, height);

    // warped quad for slice

    GEOM::Geometry * warped_quad = 0;
    if (sliced)
    {
        warped_quad = new GEOM::Geometry();
        ProjectorMapper m;
        m.setSettings(projset.domeSettings(), proj);
        m.getWarpGeometry(projcam, warped_quad);

        // and blend texture

        ProjectorBlender blend;
        blend.setSettings(projset);
        blendTexture_ = blend.renderBlendTexture(scene->projectorIndex());
    }

    // screen-quad

    QString defines;
    if (cubeMapped)
        defines += "#define MO_FULLDOME_CUBE";
    if (sliced)
        defines += "\n#define MO_BLEND_TEXTURE";

    screenQuad_[thread] = new GL::ScreenQuad(idName() + "_quad", GL::ER_THROW);
    screenQuad_[thread]->create(
                ":/shader/framebuffercamera.vert",
                ":/shader/framebuffercamera.frag",
                defines,
                warped_quad);

    // uniforms

    uColor_ = screenQuad_[thread]->shader()->getUniform("u_color", true);
    uColor_->setFloats(1,1,1,1);
    if (cubeMapped)
        uAngle_ = screenQuad_[thread]->shader()->getUniform("u_angle", true);

    // set edgeblend-texture slot
    if (sliced)
    {
        auto btex = screenQuad_[thread]->shader()->getUniform("tex_blend", true);
        btex->ints[0] = 1;
    }

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
        // the matrix that transforms the camera's viewspace
        // (which is identity normally)
        sliceMatrix_ = projcam.getViewMatrix();
        // but we need to turn it because the setup was done
        // assuming a dome with it's top/middle in the y direction
        // while the camera usually points -z
        sliceMatrix_ = MATH::rotate(sliceMatrix_, 90.f, Vec3(1.f, 0.f, 0.f));
    }

    if (sceneObject())
        emit sceneObject()->CameraFboChanged(this);
}

void Camera::releaseGl(uint thread)
{
    if (blendTexture_)
        blendTexture_->release();
    blendTexture_ = 0;

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

    const Float
            near = p_near_->value(time, thread),
            far = p_far_->value(time, thread);

    if (renderMode_ == RM_FULLDOME_CUBE)
    {
        cam.setFieldOfView(90.f);
        cam.setProjectionMatrix(
                    MATH::perspective(90.f, aspectRatio_, near, far)
                    );

        cam.setIsCubemap(true);
    }
        else cam.setIsCubemap(false);


    if (renderMode_ == RM_PERSPECTIVE)
    {
        const Float angle = p_cameraAngle_->value(time, thread);
        cam.setFieldOfView(angle);

        const Mat4 mat1 = MATH::perspective(angle, aspectRatio_, near, far);

        // mix with orthographic matrix
        const Float mix = p_cameraOrthoMix_->value(time, thread);
        if (mix > 0.f)
        {
            const Float sc = p_cameraOrthoScale_->value(time, thread);
            const Mat4 mat2 = glm::ortho(-sc * aspectRatio_, sc * aspectRatio_, -sc, sc, near, far);

            cam.setProjectionMatrix(mat1 + std::min(1.f, mix) * (mat2 - mat1));
        }
        else
            cam.setProjectionMatrix(mat1);
    }


    if (renderMode_ == RM_ORTHOGRAPHIC)
    {
        const Float sc = p_cameraOrthoScale_->value(time, thread);
        cam.setFieldOfView(90.); // XXX ???
        cam.setProjectionMatrix(
                    glm::ortho(-sc * aspectRatio_, sc * aspectRatio_, -sc, sc, near, far));
    }


    if (renderMode_ == RM_PROJECTOR_SLICE)
    {
        const Scene * scene = sceneObject();
        MO_ASSERT(scene, "No Scene in Camera::initCameraSpace()");

        const CameraSettings& projcam
                = scene->projectionSettings().cameraSettings(scene->projectorIndex());

        cam.setFieldOfView(projcam.fov());
        cam.setProjectionMatrix(projcam.getProjectionMatrix());
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

    // attach each cubemap texture
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

    // --- set default render state ---

    MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );
    MO_CHECK_GL( glDepthMask(GL_TRUE) );

    // --- background ---

    MO_CHECK_GL( glClearColor(p_backR_->value(time, thread),
                              p_backG_->value(time, thread),
                              p_backB_->value(time, thread),
                              p_backA_->value(time, thread)) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

}

void Camera::finishGlFrame(uint thread, Double)
{
    fbo_[thread]->unbind();
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
    // -- shader uniforms --

    uColor_->floats[3] = p_cameraMix_->value(time, thread);

    if (renderMode_ == RM_FULLDOME_CUBE)
        uAngle_->floats[0] = p_cameraFdAngle_->value(time, thread);


    // -- render camera frame onto current context --

    // cameras framebuffer
    GL::FrameBufferObject * fbo = fbo_[thread];

    // final framebuffer
    const GL::FrameBufferObject
            * scenefbo = sceneObject()->fboMaster(thread);

    // set blendmode
    alphaBlend_.apply(time, thread);

    fbo->colorTexture()->bind();

    // set interpolation mode
    if (p_magInterpol_->baseValue())
        fbo->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    else
        fbo->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));

    // set edge-clamp
    // (mainly important for cube maps, so the seams disappear as opposed to GL_REPEAT mode)
    fbo->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
    fbo->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));

    // bind blend texture
    if (blendTexture_)
    {
        MO_CHECK_GL( glActiveTexture(GL_TEXTURE1) );
        blendTexture_->bind();
        MO_CHECK_GL( glActiveTexture(GL_TEXTURE0) );
    }

    screenQuad_[thread]->drawCentered(scenefbo->width(), scenefbo->height(), aspectRatio_);
    fbo->colorTexture()->unbind();
}


} // namespace MO
