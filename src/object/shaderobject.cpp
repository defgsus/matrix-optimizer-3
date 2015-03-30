/** @file shaderobject.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.03.2015</p>
*/

#include "shaderobject.h"
#include "gl/context.h"
#include "io/datastream.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "geom/geometry.h"
#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "param/parameterint.h"
#include "param/parameterselect.h"
#include "param/parametertext.h"
#include "math/vector.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(ShaderObject)

ShaderObject::ShaderObject(QObject *parent)
    : ObjectGl      (parent)
    , alphaBlend_   (this)
{
    setName("Shader");

    setCreateRenderSettings(false);
}

ShaderObject::~ShaderObject()
{
    for (auto i : fbo_)
        delete i;
    for (auto i : screenQuad_)
        delete i;
}

void ShaderObject::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);
    io.writeHeader("glsl", 1);
}

void ShaderObject::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);
    io.readHeader("glsl", 1);
}

void ShaderObject::createParameters()
{
    ObjectGl::createParameters();

    params()->beginParameterGroup("shader", tr("shader"));
    initParameterGroupExpanded("shader");

        GL::ShaderSource tmp;
        tmp.loadFragmentSource(":/shader/shaderobject.frag");

        p_fragment_ = params()->createTextParameter("glsl_fragment", tr("glsl fragment shader"),
                tr("A piece of glsl code to set the output fragment color"),
                TT_GLSL,
                    tmp.fragmentSource()
                   /*"// " + tr("Please be aware that this interface is likely to change in the future!") +
                   "\n\n"
                   "// " + tr("You have access to these values (! means: if available)") + ":\n"
                   "// -- uniforms --\n"
                   "// float u_time\n"
                   "// vec3 u_cam_pos\n"
                   "// float u_bump_scale\n"
                   "// sampler2D tex_0 !\n"
                   "// sampler2D tex_norm_0 !\n"
                   "// -- input from vertex stage --\n"
                   "// vec3 v_pos\n"
                   "// vec3 v_pos_world\n"
                   "// vec3 v_pos_eye\n"
                   "// vec3 v_normal\n"
                   "// vec3 v_normal_eye\n"
                   "// vec3 v_texCoord\n"
                   "// vec3 v_cam_dir\n"
                   "// vec4 v_color\n"
                   "// vec4 v_ambient_color\n"
                   "// -- lighting --\n"
                   "// vec3 mo_normal()\n"
                   "// ... todo\n"
                   "// -- output to rasterizer --\n"
                   "// vec4 out_color\n"
                   "\n"
                   "void mo_modify_fragment_output()\n{\n\t\n}\n"
                   "void main()\n{\n\tv_color = vec4(1., 0. , 0., 1.);\n}\n"*/
                , true, false);

        p_width_ = params()->createIntParameter("fbowidth", tr("width"), tr("Width of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);
        p_height_ = params()->createIntParameter("fboheight", tr("height"), tr("Height of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);

    params()->endParameterGroup();

    params()->beginParameterGroup("output", tr("output"));

        p_out_r_ = params()->createFloatParameter("red", "red", tr("Red amount of output color"), 1.0, 0.1);
        p_out_g_ = params()->createFloatParameter("green", "green", tr("Green amount of output color"), 1.0, 0.1);
        p_out_b_ = params()->createFloatParameter("blue", "blue", tr("Blue amount of output color"), 1.0, 0.1);
        p_out_a_ = params()->createFloatParameter("alpha", tr("alpha"),
                      tr("Defines the opaqueness/transparency of the output [0,1]"),
                      1.0,
                      0.0, 1.0, 0.05);

        alphaBlend_.createParameters(AlphaBlendSetting::M_MIX, false, "_camout");

        p_magInterpol_ = params()->createBooleanParameter("cammaginterpol", tr("interpolation"),
                                                tr("The interpolation mode for pixel magnification"),
                                                tr("No interpolation"),
                                                tr("Linear interpolation"),
                                                true,
                                                true, false);

        p_aa_ = params()->createIntParameter("outaa", tr("anti-aliasing"),
                      tr("Sets the super-sampling of the rendered frame"),
                      1,
                      1, 8, 1, true, false);

    params()->endParameterGroup();
}

void ShaderObject::onParameterChanged(Parameter * p)
{
    ObjectGl::onParameterChanged(p);

    if (p == p_width_ || p == p_height_
        || p == p_fragment_
        || p == p_aa_)
        requestReinitGl();
}

void ShaderObject::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();

}

void ShaderObject::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

}

void ShaderObject::setNumberThreads(uint num)
{
    ObjectGl::setNumberThreads(num);

    uint oldnum = fbo_.size();

    fbo_.resize(num);
    shaderQuad_.resize(num);
    screenQuad_.resize(num);

    for (uint i=oldnum; i<num; ++i)
    {
        fbo_[i] = 0;
        shaderQuad_[i] = 0;
        screenQuad_[i] = 0;
    }
}

void ShaderObject::initGl(uint thread)
{
    // size of ShaderObject frame

    int width = p_width_->baseValue(),
        height = p_height_->baseValue();

    aspectRatio_ = (Float)width/std::max(1, height);

    // shader-quad

    auto src = new GL::ShaderSource();
    src->loadVertexSource(":/shader/shaderobject.vert");
    //src->loadFragmentSource(":/shader/shaderobject.frag");
    src->setFragmentSource(p_fragment_->baseValue());


    shaderQuad_[thread] = new GL::ScreenQuad(idName() + "_shaderquad", GL::ER_THROW);
    shaderQuad_[thread]->create(src);

    u_resolution_ = shaderQuad_[thread]->shader()->getUniform("u_resolution", false);
    if (u_resolution_)
        u_resolution_->setFloats(width, height,
                             1.f / std::max(1, width),
                             1.f / std::max(1, height));
    u_time_ = shaderQuad_[thread]->shader()->getUniform("u_time", false);
    u_transformation_ = shaderQuad_[thread]->shader()->getUniform("u_transformation", false);
    if (u_transformation_)
        u_transformation_->setAutoSend(true);

    // screen-quad

    int aa = p_aa_->baseValue();
    const bool doAa = aa > 1;

    QString defines;
    defines += QString("#define MO_USE_COLOR");
    if (doAa)
        defines += QString("\n#define MO_ANTIALIAS (%1)").arg(aa);

    screenQuad_[thread] = new GL::ScreenQuad(idName() + "_outquad", GL::ER_THROW);
    screenQuad_[thread]->create(
                ":/shader/framebufferdraw.vert",
                ":/shader/framebufferdraw.frag",
                defines);

    // uniforms

    u_out_color_ = screenQuad_[thread]->shader()->getUniform("u_color", true);
    u_out_color_->setFloats(1,1,1,1);
    if (doAa)
    {
        u_out_resolution_ = screenQuad_[thread]->shader()->getUniform("u_resolution", true);
    }
    else
        u_out_resolution_ = 0;

    // create framebuffer

    fbo_[thread] = new GL::FrameBufferObject(
                width,
                height,
                gl::GL_RGBA,
                gl::GL_FLOAT,
                0,//GL::FrameBufferObject::A_DEPTH,
                false,
                GL::ER_THROW);

    fbo_[thread]->create();
    fbo_[thread]->unbind();
    /*
    if (renderMode_ == RM_PROJECTOR_SLICE)
    {
        // the matrix that transforms the ShaderObject's viewspace
        // (which is identity normally)
        sliceMatrix_ = projcam.getViewMatrix();
        // but we need to turn it because the setup was done
        // assuming a dome with it's top/middle in the y direction
        // while the ShaderObject usually points -z
        sliceMatrix_ = MATH::rotate(sliceMatrix_, 90.f, Vec3(1.f, 0.f, 0.f));
    }

    if (sceneObject())
        emit sceneObject()->cameraFboChanged(this);
    */
}

void ShaderObject::releaseGl(uint thread)
{
    screenQuad_[thread]->release();
    delete screenQuad_[thread];
    screenQuad_[thread] = 0;

    fbo_[thread]->release();
    delete fbo_[thread];
    fbo_[thread] = 0;
}

void ShaderObject::renderGl(const GL::RenderSettings & , uint thread, Double time)
{
    // --- prepare fbo ---

    const int w = p_width_->baseValue(),
              h = p_height_->baseValue();

    fbo_[thread]->bind();

    MO_CHECK_GL( gl::glViewport(0, 0, w, h) );
    MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );

    // --- set shader uniforms ---

    if (u_time_)
        u_time_->floats[0] = time;

    if (u_transformation_)
        u_transformation_->setMatrix(transformation());

    // --- render ---

    shaderQuad_[thread]->draw(w, h);

    fbo_[thread]->unbind();
}


void ShaderObject::drawFramebuffer(uint thread, Double time, int width, int height)
{
    // -- shader uniforms --

    if (u_out_color_)
        u_out_color_->setFloats(
                    p_out_r_->value(time, thread),
                    p_out_g_->value(time, thread),
                    p_out_b_->value(time, thread),
                    p_out_a_->value(time, thread));

    if (u_out_resolution_)
        u_out_resolution_->setFloats(width, height,
                             1.f / std::max(1, width),
                             1.f / std::max(1, height));

    // -- render fbo frame onto current context --

    // our framebuffer
    GL::FrameBufferObject * fbo = fbo_[thread];

    // set blendmode
    alphaBlend_.apply(time, thread);

    // bind the color texture from the fbo
    MO_CHECK_GL( glActiveTexture(GL_TEXTURE0) );
    fbo->colorTexture()->bind();

    // set interpolation mode
    if (p_magInterpol_->baseValue())
        fbo->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    else
        fbo->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));

    // set edge-clamp
    // XXX not sure if needed or if should be parameterized..
    fbo->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
    fbo->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));

    screenQuad_[thread]->drawCentered(width, height, aspectRatio_);

    fbo->colorTexture()->unbind();
}


} // namespace MO
