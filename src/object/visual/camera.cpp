/** @file camera.cpp

    @brief Camera Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

//#include <QDebug>

#include "camera.h"
#include "gl/context.h"
#include "gl/cameraspace.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "object/scene.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterint.h"
#include "object/param/parametertext.h"
#include "object/util/scenesignals.h"
#include "object/util/objecteditor.h"
#include "object/util/objecttreesearch.h"
#include "math/cubemapmatrix.h"
#include "math/vector.h"
#include "projection/projectionsystemsettings.h"
#include "projection/projectormapper.h"
#include "projection/projectorblender.h"
#include "geom/geometry.h"
#include "io/datastream.h"
#include "io/log.h"
#include "io/error.h"

#undef near
#undef far  // windows..

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(Camera)

Camera::Camera()
    : ObjectGl              ()
    , fbo_                  (0)
    , msFbo_                (0)
    , screenQuad_           (0)
    , renderMode_           (RM_FULLDOME_CUBE)
    , alphaBlend_           (this)
    , useOverrideMatrix_    (false)
    , wildcardChanged_      (true)
    , blendTexture_         (0)
{
    setName("Camera");

    initCreateRenderSettings(false);
    setNumberOutputs(ST_TEXTURE, 2);
}

Camera::~Camera()
{
    delete fbo_;
    delete screenQuad_;
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
                                            0.05,
                                            0.00001, 100000.0, 0.05);
        p_far_ = params()->createFloatParameter("camfar", tr("far plane"),
                                            tr("Far plane of camera frustum - everything farther away can not be drawn"),
                                            1000.0,
                                            0.00002, 100000.0, 1.0);
        params()->beginEvolveGroup(false);

        p_tex_format_ = params()->createTextureFormatParameter("fbo_format", tr("framebuffer format"),
                                                    tr("The channel format of the framebuffer"));
        p_tex_type_ = params()->createTextureTypeParameter("fbo_type", tr("framebuffer type"),
                                                    tr("The type-per-channel of the framebuffer"));

        p_width_ = params()->createIntParameter(
                    "fbowidth", tr("width"), tr("Width of rendered frame in pixels"),
                                      1024, 1, 4096*4, 16, true, false);
        p_height_ = params()->createIntParameter(
                    "fboheight", tr("height"), tr("Height of rendered frame in pixels"),
                                      1024, 1, 4096*4, 16, true, false);
        p_cubeRes_ = params()->createIntParameter(
                    "fbocuberes", tr("resolution of cube map"),
                    tr("Width and height of the rendered frame per cube face"),
                    1024, 4, 4096*4, 16, true, false);

        p_multiSample_ = params()->createIntParameter(
                    "fbo_msaa",
                    tr("anti-aliasing (multi-sampling)"),
                    tr("Number of multi-samples in framebuffer to avoid aliasing"),
                    0, 0, 64, 1,
                    true, false);

    params()->endEvolveGroup();
    params()->endParameterGroup();


    params()->beginParameterGroup("camobjects", tr("rendered objects"));

        p_wcInclude_ = params()->createTextParameter(
                    "cam_include", tr("include objects"),
                 tr("Wildcard expressions of which objects should be rendered"),
                 TT_OBJECT_WILDCARD,
                 "*", true, false);

        p_wcIgnore_ = params()->createTextParameter(
                    "cam_ignore", tr("ignore objects"),
                 tr("Wildcard expressions of which objects should not be rendered"),
                 TT_OBJECT_WILDCARD,
                 "", true, false);

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
        p_backA_->setDefaultEvolvable(false);

    params()->endParameterGroup();

    params()->beginParameterGroup("output", tr("master output"));
    params()->beginEvolveGroup(false);

        p_enableOut_ = params()->createBooleanParameter("master_out", tr("enable"),
                       tr("Enables or disables sampling the output to the main framebuffer"),
                       tr("The camera will render internally but not contribute to the main framebuffer"),
                       tr("The camera will render it's output ontop the main framebuffer"),
                       true, true, true);

        p_cameraMix_ = params()->createFloatParameter("cammix", tr("camera mix"),
                          tr("Defines the opaqueness/transparency of the camera [0,1]"),
                          1.0,
                          0.0, 1.0, 0.05);

        alphaBlend_.createParameters(AlphaBlendSetting::M_MIX, false, "_", "_camout");

        p_magInterpol_ = params()->createBooleanParameter("cammaginterpol", tr("interpolation"),
                                                tr("The interpolation mode for pixel magnification"),
                                                tr("No interpolation"),
                                                tr("Linear interpolation"),
                                                true,
                                                true, false);
    params()->endEvolveGroup();
    params()->endParameterGroup();
}

void Camera::onParameterChanged(Parameter * p)
{
    ObjectGl::onParameterChanged(p);

    if (p == p_cameraMode_)
    {
        renderMode_ = (RenderMode)p_cameraMode_->baseValue();
        requestReinitGl();
    }

    if (p == p_width_ || p == p_height_ || p == p_cubeRes_
        || p_tex_format_ || p == p_tex_type_
        || p == p_multiSample_)
        requestReinitGl();

    if (p == p_wcIgnore_ || p == p_wcInclude_)
        wildcardChanged_ = true;
}

void Camera::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();
    renderMode_ = (RenderMode)p_cameraMode_->baseValue();
    wildcardChanged_ = true;
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
    p_multiSample_->setVisible( !iscube );
}

QString Camera::getOutputName(SignalType st, uint channel) const
{
    if (st == ST_TEXTURE)
    {
        if (channel == 0)
            return tr("color");
        if (channel == 1)
            return tr("depth");
    }
    return ObjectGl::getOutputName(st, channel);
}

void Camera::setNumberThreads(uint num)
{
    ObjectGl::setNumberThreads(num);

    cheat_.resize(num);
}

bool Camera::isMultiSampling() const
{
    return p_multiSample_->baseValue() > 0
          && (renderMode() != RM_FULLDOME_CUBE);
}

bool Camera::isMasterOutputEnabled() const
{
    return p_enableOut_->baseValue();
}

void Camera::setMasterOutputEnabled(bool enable, bool sendGui)
{
    if (sendGui)
    {
        if (auto e = editor())
        {
            e->setParameterValue(p_enableOut_, enable);
            return;
        }
        MO_WARNING("Can't set master-out parameter via GUI as expected");
    }
    p_enableOut_->setValue(enable);
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

    screenQuad_ = new GL::ScreenQuad(idName() + "_quad");
    try
    {
        screenQuad_->create(
                    ":/shader/framebuffercamera.vert",
                    ":/shader/framebuffercamera.frag",
                    defines,
                    warped_quad);

        // uniforms

        uColor_ = screenQuad_->shader()->getUniform("u_color", true);
        uColor_->setFloats(1,1,1,1);
        if (cubeMapped)
            uAngle_ = screenQuad_->shader()->getUniform("u_angle", true);

        // set edgeblend-texture slot
        if (sliced)
        {
            auto btex = screenQuad_->shader()->getUniform("tex_blend", true);
            btex->ints[0] = 1;
        }
    }
    catch (Exception & e)
    {
        releaseGl(thread);
        throw;
    }

    // create framebuffer

    fbo_ = new GL::FrameBufferObject(
                width,
                height,
                gl::GLenum(Parameters::getTexFormat(p_tex_format_->baseValue(),
                                                    p_tex_type_->baseValue())),
                gl::GLenum(p_tex_format_->baseValue()),
                gl::GL_FLOAT,
                GL::FrameBufferObject::A_DEPTH,
                cubeMapped);
    fbo_->setName(name());

    try
    {
        fbo_->create();
        fbo_->unbind();
    }
    catch (Exception& e)
    {
        e << "\nin creating camera's framebuffer";
        releaseGl(thread);
        throw;
    }

    // multi-sampling framebuffer
    if (isMultiSampling())
    {
        msFbo_ = new GL::FrameBufferObject(
                    width,
                    height,
                    gl::GLenum(Parameters::getTexFormat(p_tex_format_->baseValue(),
                                                        p_tex_type_->baseValue())),
                    gl::GLenum(p_tex_format_->baseValue()),
                    gl::GL_FLOAT,
                    GL::FrameBufferObject::A_DEPTH,
                    cubeMapped,
                    p_multiSample_->baseValue());
        msFbo_->setName(name() + "_ms");

        try
        {
            msFbo_->create();
            msFbo_->unbind();
        }
        catch (Exception& e)
        {
            e << "\nin creating camera's ms-framebuffer";
            releaseGl(thread);
            throw;
        }
    }
    else
        msFbo_ = 0;

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

    if (auto s = sceneObject())
        emit s->sceneSignals()->CameraFboChanged(this);
}

void Camera::releaseGl(uint )
{
    if (blendTexture_)
    {
        blendTexture_->release();
        delete blendTexture_;
        blendTexture_ = 0;
    }
    if (screenQuad_)
    {
        screenQuad_->release();
        delete screenQuad_;
        screenQuad_ = 0;
    }
    if (fbo_)
    {
        fbo_->release();
        delete fbo_;
        fbo_ = 0;
    }
    if (msFbo_)
    {
        msFbo_->release();
        delete msFbo_;
        msFbo_ = 0;
    }
}

void Camera::initCameraSpace(GL::CameraSpace &cam, const RenderTime& time) const
{
    cam.setSize(fbo_->width(), fbo_->height());

    const Float
            near = p_near_->value(time),
            far = std::max(near+0.0001f, (Float)p_far_->value(time));

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
        const Float angle = p_cameraAngle_->value(time);
        cam.setFieldOfView(angle);

        const Mat4 mat1 = MATH::perspective(angle, aspectRatio_, near, far);

        // mix with orthographic matrix
        const Float mix = p_cameraOrthoMix_->value(time);
        if (mix > 0.f)
        {
            const Float sc = p_cameraOrthoScale_->value(time);
            const Mat4 mat2 = glm::ortho(
                        -sc * aspectRatio_, sc * aspectRatio_, -sc, sc, near, far);

            cam.setProjectionMatrix(mat1 + std::min(1.f, mix) * (mat2 - mat1));
        }
        else
            cam.setProjectionMatrix(mat1);
    }


    if (renderMode_ == RM_ORTHOGRAPHIC)
    {
        const Float sc = p_cameraOrthoScale_->value(time);
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

uint Camera::numCubeTextures(const RenderTime & time) const
{
    if (renderMode_ != RM_FULLDOME_CUBE)
        return 1;

    return (p_cameraFdAngle_->value(time) >= 250.f)
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

void Camera::setOverrideMatrix(const Mat4 &m)
{
    overrideMatrix_ = m;
    //overrideMatrixTime_ = time;
    useOverrideMatrix_ = true;
}

void Camera::calculateTransformation(Mat4& matrix, const RenderTime& time) const
{
    if (useOverrideMatrix_)
    {
        // XXX Just for the moment...

        Float inc = 1.f / 6.f;
        if (time.thread() == MO_AUDIO_THREAD)
            inc = 6.f*sampleRateInv();

        cheat_[time.thread()] += inc * (overrideMatrix_ - cheat_[time.thread()]);

        // normalize matrix components
        // to avoid too much deformation
        // XXX still hacky
        Vec3 v = glm::normalize(Vec3(cheat_[time.thread()][0]));
        cheat_[time.thread()][0].x = v.x;
        cheat_[time.thread()][0].y = v.y;
        cheat_[time.thread()][0].z = v.z;
        v = glm::normalize(Vec3(cheat_[time.thread()][1]));
        cheat_[time.thread()][1].x = v.x;
        cheat_[time.thread()][1].y = v.y;
        cheat_[time.thread()][1].z = v.z;
        v = glm::normalize(Vec3(cheat_[time.thread()][2]));
        cheat_[time.thread()][2].x = v.x;
        cheat_[time.thread()][2].y = v.y;
        cheat_[time.thread()][2].z = v.z;

        bool playing = false;
        if (auto s = sceneObject())
            playing = s->isPlaying();

        // don't smooth matrix for graphics in non-playback
        if (time.thread() != MO_AUDIO_THREAD && !playing)
            matrix = overrideMatrix_;
        else
            matrix = cheat_[time.thread()];
    }
    else
    {
        Object::calculateTransformation(matrix, time);
        cheat_[time.thread()] = matrix;
    }

    //matrix = MATH::rotate(matrix, -90.f, Vec3(1,0,0));
}

void Camera::attachCubeTexture(uint cubeMapIndex)
{
    using namespace gl;

    GL::FrameBufferObject * fbo = msFbo_ ? msFbo_ : fbo_;

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
}

void Camera::startGlFrame(const RenderTime& time, uint cubeMapIndex)
{
    GL::FrameBufferObject * fbo = msFbo_ ? msFbo_ : fbo_;

    fbo->bind();

    attachCubeTexture(cubeMapIndex);

    fbo->setViewport();

    // --- set default render state ---

    MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );
    MO_CHECK_GL( glDepthMask(GL_TRUE) );

    // --- background ---

    MO_CHECK_GL( glClearColor(p_backR_->value(time),
                              p_backG_->value(time),
                              p_backB_->value(time),
                              p_backA_->value(time)) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

    fbo->setChanged();
}

void Camera::finishGlFrame(const RenderTime &)
{
    if (msFbo_)
    {
        // copy multi-sample fbo to fbo
        // (automatically applies anti-aliasing)
        MO_CHECK_GL( gl::glBindFramebuffer(gl::GL_DRAW_FRAMEBUFFER, fbo_->handle()) );
        MO_CHECK_GL( gl::glBindFramebuffer(gl::GL_READ_FRAMEBUFFER, msFbo_->handle()) );
        MO_CHECK_GL( gl::glBlitFramebuffer(0, 0, fbo_->width(), fbo_->height(),
                                           0, 0, fbo_->width(), fbo_->height(),
                                             gl::GL_COLOR_BUFFER_BIT
                                           | gl::GL_DEPTH_BUFFER_BIT
                                           , gl::GL_NEAREST) );
        msFbo_->unbind();
    }

    fbo_->unbind();
}


const GL::Texture * Camera::valueTexture(uint channel, const RenderTime& ) const
{
    if (!fbo_ || channel > 1)
        return 0;
    if (channel == 1)
        return fbo_->depthTexture();
    else
        return fbo_->colorTexture();
}


void Camera::drawFramebuffer(const RenderTime& time)
{
    if (p_enableOut_->value(time) == 0)
        return;

    // -- shader uniforms --

    uColor_->floats[3] = p_cameraMix_->value(time);

    if (renderMode_ == RM_FULLDOME_CUBE)
        uAngle_->floats[0] = p_cameraFdAngle_->value(time);

    // -- render camera frame onto current context --

    // camera's framebuffer
    GL::FrameBufferObject * fbo = fbo_;

    // final framebuffer
    const GL::FrameBufferObject
            * scenefbo = sceneObject()->fboMaster(time.thread());

    // set blendmode
    alphaBlend_.apply(time);

    // bind the color texture from the fbo
    MO_CHECK_GL( glActiveTexture(GL_TEXTURE0) );
    fbo->colorTexture()->bind();

    // set interpolation mode
    if (p_magInterpol_->baseValue())
        fbo->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    else
        fbo->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));

    // set edge-clamp
    // (mainly important for cube maps, so the seams disappear
    //  as opposed to GL_REPEAT mode)
    fbo->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
    fbo->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));

    // bind blend texture
    if (blendTexture_)
    {
        GL::Texture::setActiveTexture(1);
        blendTexture_->bind();
        GL::Texture::setActiveTexture(0);
    }

    /** @todo hack to stretch into projector slice -
        for some very unknown reason the size was off on Hamburg clients... */
    if (isClient() && renderMode() == RM_PROJECTOR_SLICE)
        screenQuad_->draw(scenefbo->width(), scenefbo->height());
    else
        screenQuad_->drawCentered(scenefbo->width(), scenefbo->height(), aspectRatio_);
}




QList<ObjectGl*> Camera::getRenderObjects()
{
    wildcardChanged_ = false;

    QList<ObjectGl*> list;

    auto s = sceneObject();
    if (!s)
        return list;

    const QString
            inc = p_wcInclude_->baseValue(),
            ign = p_wcIgnore_->baseValue();

    ObjectTreeSearch::getObjectsWildcard(s, list, inc, ign);

    return list;
}

} // namespace MO
