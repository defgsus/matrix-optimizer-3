/** @file shaderobject.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.03.2015</p>
*/

#include "shaderobject.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/util/useruniformsetting.h"
#include "object/util/objecteditor.h"
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

ShaderObject::ShaderObject()
    : ObjectGl      ()
    , fbo_          (0)
    , shaderQuad_   (0)
    , screenQuad_   (0)
    , swapTex_      (0)
    , alphaBlend_   (this)
    , userUniforms_ (new UserUniformSetting(this))
{
    setName("Shader");
    initCreateRenderSettings(false);
    setNumberOutputs(ST_TEXTURE, 1);
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

    params()->beginParameterGroup("res", tr("resolution and format"));
    params()->beginEvolveGroup(false);

        p_width_ = params()->createIntParameter("fbowidth", tr("width"), tr("Width of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);
        p_height_ = params()->createIntParameter("fboheight", tr("height"), tr("Height of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);

        p_split_ = params()->createIntParameter("split", tr("segments"),
                                    tr("Split rendering of the output into separate regions for better gui response"),
                                    1, 1, 4096, 1, true, false);

        p_texFormat_ = params()->createTextureFormatParameter("texture_format", tr("texture format"),
                                                    tr("The channel format of the output texture"));
        p_texType_ = params()->createTextureTypeParameter("texture_type", tr("texture type"),
                                                    tr("The type-per-channel of the output texture"));

    params()->endEvolveGroup();
    params()->endParameterGroup();

    params()->beginParameterGroup("shader", tr("shader"));
    initParameterGroupExpanded("shader");

        GL::ShaderSource tmp;
        tmp.loadFragmentSource(":/shader/shaderobject.frag");

        p_fragment_ = params()->createTextParameter("glsl_fragment", tr("glsl fragment shader"),
                tr("A piece of glsl code to set the output fragment color"),
                TT_GLSL,
                tmp.fragmentSource()
                , true, false);

        p_passes_ = params()->createIntParameter("passes", tr("number of passes"),
                                    tr("Runs the shader multiple times, reusing the previous output as input for each pass"),
                                    1,  1, 4096,  1, true, true);
        p_passes_->setDefaultEvolvable(false);

    params()->endParameterGroup();


    params()->beginParameterGroup("useruniforms", tr("user uniforms"));

        userUniforms_->createParameters("");

    params()->endParameterGroup();


    params()->beginParameterGroup("output", tr("output"));
    params()->beginEvolveGroup(false);

        p_enableOut_ = params()->createBooleanParameter("master_out", tr("enable"),
                           tr("Enables or disables sampling the output to the main framebuffer"),
                           tr("The shader will render internally but not contribute to the main framebuffer"),
                           tr("The shader will render it's output ontop the main framebuffer"),
                            true, true, true);

        p_out_r_ = params()->createFloatParameter("red", "red", tr("Red amount of output color"), 1.0, 0.1);
        p_out_g_ = params()->createFloatParameter("green", "green", tr("Green amount of output color"), 1.0, 0.1);
        p_out_b_ = params()->createFloatParameter("blue", "blue", tr("Blue amount of output color"), 1.0, 0.1);
        p_out_a_ = params()->createFloatParameter("alpha", tr("alpha"),
                      tr("Defines the opaqueness/transparency of the output [0,1]"),
                      1.0,
                      0.0, 1.0, 0.05);
        p_out_a_->setDefaultEvolvable(false);

        alphaBlend_.createParameters(AlphaBlendSetting::M_MIX, false, "_", "_out");

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

    params()->endEvolveGroup();
    params()->endParameterGroup();
}

void ShaderObject::onParameterChanged(Parameter * p)
{
    ObjectGl::onParameterChanged(p);

    if (p == p_width_ || p == p_height_
        || p == p_fragment_
        || p == p_aa_
        || p == p_split_
        || p == p_texFormat_
        || p == p_texType_
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

QSize ShaderObject::resolution() const
{
    return fbo_ ? QSize(fbo_->width(), fbo_->height())
                : QSize();
}

bool ShaderObject::isMasterOutputEnabled() const
{
    return p_enableOut_->baseValue();
}

void ShaderObject::setMasterOutputEnabled(bool enable, bool sendGui)
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

const GL::Texture * ShaderObject::valueTexture(uint chan, const RenderTime& ) const
{
    return chan == 0 && fbo_ ? fbo_->colorTexture() : 0;
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
    shaderQuad_ = new GL::ScreenQuad(idName() + "_shaderquad");
    try
    {
        shaderQuad_->create(src, 0);

        // shader-quad uniforms
        u_resolution_ = shaderQuad_->shader()->getUniform("u_resolution", false);
        u_time_ = shaderQuad_->shader()->getUniform("u_time", false);
        u_transformation_ = shaderQuad_->shader()->getUniform("u_transformation", false);
        u_fb_tex_ = shaderQuad_->shader()->getUniform("u_feedback", false);
        u_pass_ = shaderQuad_->shader()->getUniform("u_pass", false);

        if (u_resolution_)
            u_resolution_->setFloats(width, height,
                                 1.f / std::max(1, width),
                                 1.f / std::max(1, height));
        if (u_transformation_)
            u_transformation_->setAutoSend(true);

        userUniforms_->tieToShader(shaderQuad_->shader());
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
        setErrorMessage(e.what());

        releaseGl(thread);
        throw;
    }

    // screen-quad

    int aa = p_aa_->baseValue();
    const bool doAa = aa > 1;

    QString defines;
    defines += QString("#define MO_USE_COLOR");
    if (doAa)
        defines += QString("\n#define MO_ANTIALIAS (%1)").arg(aa);

    screenQuad_ = new GL::ScreenQuad(idName() + "_outquad");
    try
    {
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
    }
    catch (Exception & e)
    {
        releaseGl(thread);
        throw;
    }

    // create framebuffer

    int format = p_texFormat_->baseValue(),
        type = p_texType_->baseValue();
    fbo_ = new GL::FrameBufferObject(
                width,
                height,
                gl::GLenum(Parameters::getTexFormat(format, type)),
                gl::GLenum(format),
                gl::GL_FLOAT,
                0,//GL::FrameBufferObject::A_DEPTH,
                false, false);
    fbo_->setName(name());

    try
    {
        fbo_->create();
        fbo_->unbind();
    }
    catch (Exception & e)
    {
        releaseGl(thread);
        throw;
    }

    /// @todo maybe input projector slice somehow to ShaderObject?
}

void ShaderObject::releaseGl(uint)
{
    userUniforms_->releaseGl();

    if (screenQuad_)
    {
        screenQuad_->release();
        delete screenQuad_;
        screenQuad_ = 0;
    }

    if (shaderQuad_)
    {
        shaderQuad_->release();
        delete shaderQuad_;
        shaderQuad_ = 0;
    }

    if (swapTex_)
    {
        swapTex_->release();
        delete swapTex_;
        swapTex_ = 0;
    }

    if (fbo_)
    {
        fbo_->release();
        delete fbo_;
        fbo_ = 0;
    }
}

void ShaderObject::renderGl(const GL::RenderSettings & , const RenderTime& time)
{
    // --- prepare fbo ---

    const int w = p_width_->baseValue(),
              h = p_height_->baseValue();

    fbo_->bind();

    // --- set shader uniforms ---

    if (u_time_)
        u_time_->floats[0] = time.second();

    if (u_transformation_)
        u_transformation_->set(transformation());

    uint texSlot = 0, swapTexSlot = 0;
    userUniforms_->updateUniforms(time, &texSlot);

    // exchange render target and bind previous frame
    if (u_fb_tex_)
    {
        // create a swap buffer
        if (!swapTex_)
        {
            swapTex_ = GL::Texture::constructFrom(fbo_->colorTexture());
            swapTex_->create();
        }
        swapTex_ = fbo_->swapColorTexture(swapTex_);

        // bind previous frame
        GL::Texture::setActiveTexture(texSlot);
        swapTex_->bind();
        u_fb_tex_->ints[0] = swapTexSlot = texSlot;
    }


    // --- render ---

    MO_CHECK_GL( glDisable(GL_BLEND) )

    MO_CHECK_GL( gl::glViewport(0, 0, w, h) );
    MO_CHECK_GL( gl::glClearColor(0,0,0,0) );

    int numPasses = p_passes_->value(time);
    for (int i=0; i<numPasses; ++i)
    {
        // swap texture for each pass
        if (u_fb_tex_ && i > 0)
        {
            swapTex_ = fbo_->swapColorTexture(swapTex_);
            swapTex_->bind();
        }

        // update a view uniforms
        if (u_pass_)
            u_pass_->setFloats(i, Float(i)/numPasses, Float(i)/std::max(1, numPasses-1), numPasses);

        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );

        MO_EXTEND_EXCEPTION(
            shaderQuad_->draw(w, h, p_split_->baseValue())
                    , "in ShaderObject::renderGl()");
    }

    gl::glFlush();
    gl::glFinish();

    fbo_->setChanged();
    fbo_->unbind();
}


void ShaderObject::drawFramebuffer(const RenderTime & time, int width, int height)
{
    if (p_enableOut_->value(time) == 0)
        return;

    gl::glFinish();

    // -- shader uniforms --

    if (u_out_color_)
        u_out_color_->setFloats(
                    p_out_r_->value(time),
                    p_out_g_->value(time),
                    p_out_b_->value(time),
                    p_out_a_->value(time));

    if (u_out_resolution_)
        u_out_resolution_->setFloats(width, height,
                             1.f / std::max(1, width),
                             1.f / std::max(1, height));

    // -- render fbo frame onto current context --

    // bind the color texture from the fbo
    GL::Texture::setActiveTexture(0);
    fbo_->colorTexture()->bind();

    // set blendmode
    alphaBlend_.apply(time);

    // set interpolation mode
    if (p_magInterpol_->baseValue())
        fbo_->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    else
        fbo_->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));

    // set edge-clamp
    // XXX not sure if needed or if should be parameterized..
    fbo_->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
    fbo_->colorTexture()->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));

    MO_EXTEND_EXCEPTION(
        // draw the texture
        screenQuad_->drawCentered(width, height, aspectRatio_)
                , "in ShaderObject::drawFrameBuffer()");

    gl::glFinish();
}


} // namespace MO
