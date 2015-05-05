/** @file shaderobject.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.03.2015</p>
*/

#include "shaderobject.h"
#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "param/parameterint.h"
#include "param/parameterselect.h"
#include "param/parametertext.h"
#include "util/useruniformsetting.h"
#include "gl/context.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "geom/geometry.h"
#include "math/vector.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(ShaderObject)

ShaderObject::ShaderObject(QObject *parent)
    : ObjectGl      (parent)
    , swapTex_      (0)
    , alphaBlend_   (this)
    , userUniforms_ (new UserUniformSetting(this))
{
    setName("Shader");

    setCreateRenderSettings(false);
}

ShaderObject::~ShaderObject()
{
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
                , true, false);

        p_width_ = params()->createIntParameter("fbowidth", tr("width"), tr("Width of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);
        p_height_ = params()->createIntParameter("fboheight", tr("height"), tr("Height of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);

        p_split_ = params()->createIntParameter("split", tr("segments"),
                                    tr("Split rendering of the output into separate regions for better gui response"),
                                    1, 1, 4096, 1, true, false);


    params()->endParameterGroup();

    params()->beginParameterGroup("useruniforms", tr("user uniforms"));

        userUniforms_->createParameters("");

    params()->endParameterGroup();

    params()->beginParameterGroup("output", tr("output"));

        p_out_r_ = params()->createFloatParameter("red", "red", tr("Red amount of output color"), 1.0, 0.1);
        p_out_g_ = params()->createFloatParameter("green", "green", tr("Green amount of output color"), 1.0, 0.1);
        p_out_b_ = params()->createFloatParameter("blue", "blue", tr("Blue amount of output color"), 1.0, 0.1);
        p_out_a_ = params()->createFloatParameter("alpha", tr("alpha"),
                      tr("Defines the opaqueness/transparency of the output [0,1]"),
                      1.0,
                      0.0, 1.0, 0.05);

        alphaBlend_.createParameters(AlphaBlendSetting::M_MIX, false, "_out");

        p_magInterpol_ = params()->createBooleanParameter("cammaginterpol", tr("interpolation"),
                                                tr("The interpolation mode for pixel magnification"),
                                                tr("No interpolation"),
                                                tr("Linear interpolation"),
                                                true,
                                                true, false);

        p_aa_ = params()->createIntParameter("outaa", tr("anti-aliasing"),
                      tr("Sets the super-sampling when drawing the rendered frame onto the output"),
                      1,
                      1, 16, 1, true, false);

    params()->endParameterGroup();
}

void ShaderObject::onParameterChanged(Parameter * p)
{
    ObjectGl::onParameterChanged(p);

    if (p == p_width_ || p == p_height_
        || p == p_fragment_
        || p == p_aa_
        || p == p_split_
        || userUniforms_->needsReinit(p))
        requestReinitGl();
}

void ShaderObject::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();

}

void ShaderObject::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    userUniforms_->updateParameterVisibility();
}


void ShaderObject::initGl(uint )
{
    // size of ShaderObject frame

    int width = p_width_->baseValue(),
        height = p_height_->baseValue();

    aspectRatio_ = (Float)width/std::max(1, height);

    // shader-quad

    auto src = new GL::ShaderSource();
    src->loadVertexSource(":/shader/shaderobject.vert");
    src->setFragmentSource(p_fragment_->baseValue());
    // declare user uniforms
    src->replace("//%user_uniforms%", userUniforms_->getDeclarations(), true);
    // resolve includes
    src->replaceIncludes([this](const QString& url, bool do_search)
    {
        QString inc = getGlslInclude(url, do_search);
        return inc.isEmpty() ? inc : ("// ----- include '" + url + "' -----\n" + inc);
    });

    // create quad and compile shader
    shaderQuad_ = new GL::ScreenQuad(idName() + "_shaderquad", GL::ER_THROW);
    try
    {
        shaderQuad_->create(src, 0);
    }
    catch (Exception& e)
    {
        // send errors to editor widget
        for (const GL::Shader::CompileMessage & msg : shaderQuad_->shader()->compileMessages())
        {
            if (msg.program == GL::Shader::P_FRAGMENT
                || msg.program == GL::Shader::P_LINKER)
            {
                p_fragment_->addErrorMessage(msg.line, msg.text);
            }
        }
        throw;
    }

    // shader-quad uniforms
    u_resolution_ = shaderQuad_->shader()->getUniform("u_resolution", false);
    if (u_resolution_)
        u_resolution_->setFloats(width, height,
                             1.f / std::max(1, width),
                             1.f / std::max(1, height));
    u_time_ = shaderQuad_->shader()->getUniform("u_time", false);
    u_transformation_ = shaderQuad_->shader()->getUniform("u_transformation", false);
    if (u_transformation_)
        u_transformation_->setAutoSend(true);

    userUniforms_->tieToShader(shaderQuad_->shader());


    // screen-quad

    int aa = p_aa_->baseValue();
    const bool doAa = aa > 1;

    QString defines;
    defines += QString("#define MO_USE_COLOR");
    if (doAa)
        defines += QString("\n#define MO_ANTIALIAS (%1)").arg(aa);

    screenQuad_ = new GL::ScreenQuad(idName() + "_outquad", GL::ER_THROW);
    screenQuad_->create(
                ":/shader/framebufferdraw.vert",
                ":/shader/framebufferdraw.frag",
                defines);

    // uniforms

    u_out_color_ = screenQuad_->shader()->getUniform("u_color", true);
    u_out_color_->setFloats(1,1,1,1);
    if (doAa)
    {
        u_out_resolution_ = screenQuad_->shader()->getUniform("u_resolution", true);
    }
    else
        u_out_resolution_ = 0;

    // create framebuffer

    fbo_ = new GL::FrameBufferObject(
                width,
                height,
                gl::GL_RGBA,
                gl::GL_FLOAT,
                0,//GL::FrameBufferObject::A_DEPTH,
                false,
                GL::ER_THROW);

    fbo_->create();
    fbo_->unbind();

    /// @todo maybe input projector slice somehow to ShaderObject?
}

void ShaderObject::releaseGl(uint )
{
    userUniforms_->releaseGl();

    screenQuad_->release();
    delete screenQuad_;
    screenQuad_ = 0;

    shaderQuad_->release();
    delete shaderQuad_;
    shaderQuad_ = 0;

    if (swapTex_)
        swapTex_->release();
    delete swapTex_;
    swapTex_ = 0;

    fbo_->release();
    delete fbo_;
    fbo_ = 0;
}

void ShaderObject::renderGl(const GL::RenderSettings & , uint thread, Double time)
{
    // --- prepare fbo ---

    const int w = p_width_->baseValue(),
              h = p_height_->baseValue();

    fbo_->bind();

    // exchange render target
    if (1)
    {
        // create a swap buffer
        if (!swapTex_)
        {
            swapTex_ = GL::Texture::constructFrom(fbo_->colorTexture());
            swapTex_->create();
        }
        swapTex_ = fbo_->swapColorTexture(swapTex_);
        swapTex_->bind();
    }

    MO_CHECK_GL( gl::glViewport(0, 0, w, h) );
    MO_CHECK_GL( gl::glClearColor(0,0,0,0) );
    MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );

    // --- set shader uniforms ---

    if (u_time_)
        u_time_->floats[0] = time;

    if (u_transformation_)
        u_transformation_->set(transformation());

    userUniforms_->updateUniforms(time, thread);

    // --- render ---

    shaderQuad_->draw(w, h, p_split_->baseValue());

    gl::glFlush();
    gl::glFinish();

    fbo_->unbind();
}


void ShaderObject::drawFramebuffer(uint thread, Double time, int width, int height)
{
    gl::glFinish();

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

    // set blendmode
    alphaBlend_.apply(time, thread);

    // bind the color texture from the fbo
    MO_CHECK_GL( glActiveTexture(GL_TEXTURE0) );
    fbo_->colorTexture()->bind();

    // set interpolation mode
    if (p_magInterpol_->baseValue())
        fbo_->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    else
        fbo_->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));

    // set edge-clamp
    // XXX not sure if needed or if should be parameterized..
    fbo_->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
    fbo_->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));

    // draw the texture
    screenQuad_->drawCentered(width, height, aspectRatio_);

    fbo_->colorTexture()->unbind();

    gl::glFinish();
}


} // namespace MO
