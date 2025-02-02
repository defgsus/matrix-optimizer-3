/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/7/2015</p>
*/

#include <QDate>
#include <QDebug>
#include "ShaderTO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterText.h"
#include "object/param/ParameterTexture.h"
#include "object/util/UserUniformSetting.h"
#include "gl/ScreenQuad.h"
#include "gl/Shader.h"
#include "gl/ShaderSource.h"
#include "gl/Texture.h"
#include "math/functions.h"
#include "io/MouseState.h"
#include "io/DataStream.h"
#include "io/log.h"

#undef near
#undef far // windows..

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(ShaderTO)

struct ShaderTO::Private
{
    Private(ShaderTO * to)
        : to            (to)
        , uniformSetting(new UserUniformSetting(to))
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    ShaderTO * to;

    ParameterText
            * p_glsl;
    ParameterFloat
            * p_r, * p_g, * p_b, * p_a;
    UserUniformSetting
            * uniformSetting;
    GL::Uniform
            * u_color,
            * u_res,
            * u_chan_res,
            * u_date,
            * u_mouse,
            * u_samplerate,
            * u_frame;
};


ShaderTO::ShaderTO()
    : TextureObjectBase ()
    , p_                (new Private(this))
{
    setName("Shader");
    initDefaultUpdateMode(UM_ALWAYS);
    initMaximumTextureInputs(4);
    initAllowMultiPass(true);
}

ShaderTO::~ShaderTO()
{
    delete p_;
}

void ShaderTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("toshader", 1);
}

void ShaderTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("toshader", 1);
}

void ShaderTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void ShaderTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void ShaderTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void ShaderTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void ShaderTO::Private::createParameters()
{
    to->params()->beginParameterGroup("shader", tr("shader"));
    to->initParameterGroupExpanded("shader");

        p_glsl = to->params()->createTextParameter("glsl", tr("glsl source"),
               tr("A piece of glsl code to calculate the pixel output"),
               TT_GLSL,
    "/* Shadertoy compatible!\n"
    "\n"
    " uniforms:\n"
    "   vec3  iResolution;              // resolution of output texture in pixels\n"
    "   float iGlobalTime;              // scene time in seconds\n"
    "   float iGlobalDelta;             // seconds between last and this frame\n"
    "   uniform float iFrame;           // the current frame (independent of time)\n"
    "   float iChannelTime[4];          // playback of channel in seconds "
                                                                "(not defined yet)\n"
    "   vec3  iChannelResolution[4];    // resolution per channel in pixels\n"
    "   vec4  iMouse;                   // xy=mouse position in pixels, zw = click\n"
    "   vec4  iDate;                    // year, month, day, time in seconds\n"
    "   float iSampleRate;              // sound sampling rate in Hertz\n"
    "   sampler2D/Cube iChannel0-3;     // input textures\n"
    "   sampler2D u_tex_feedback;       // the previous output frame\n"
    "*/\n"
    "\n"
    "// fragCoord is in pixels\n"
    "void mainImage(out vec4 fragColor, in vec2 fragCoord)\n"
    "{\n"
    "    // get texture map coord [0,1] from pixel position\n"
    "    vec2 uv = fragCoord / iResolution.y;\n"
    "    // use uv as output color\n"
    "    fragColor = vec4(uv,\n"
    "    // blue is some function of x-axis and time\n"
    "        .5 + .5 * sin(iGlobalTime + uv.x), 1.)\n"
    "    // add the input texture\n"
    "        + texture(iChannel0, uv);\n"
    "}\n\n"
               , true, false);

        p_r = to->params()->createFloatParameter(
            "red", tr("red"), tr("Red amount of output"), 1.0,  0.,1.,  0.025);
        p_g = to->params()->createFloatParameter(
            "green", tr("green"), tr("Green amount of output"), 1.0,  0.,1.,  0.025);
        p_b = to->params()->createFloatParameter(
            "blue", tr("blue"), tr("Blue amount of output"), 1.0,  0.,1.,  0.025);
        p_a = to->params()->createFloatParameter(
            "alpha", tr("alpha"), tr("Alpha amount of output"), 1.0,  0.,1.,  0.025);
        p_a->setDefaultEvolvable(false);

    to->params()->endParameterGroup();

    to->params()->beginParameterGroup("useruniforms", tr("user uniforms"));

        uniformSetting->createParameters("g");

    to->params()->endParameterGroup();

}

void ShaderTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_glsl
        || p_->uniformSetting->needsReinit(p))
        requestReinitGl();

}

void ShaderTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void ShaderTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    p_->uniformSetting->updateParameterVisibility();
}




void ShaderTO::Private::initGl()
{
    // shader-quad

    QStringList texnames = { "iChannel0", "iChannel1", "iChannel2", "iChannel3" };

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/shader.frag");
        src.pasteDefaultIncludes();

        QString code = uniformSetting->getDeclarations();
        code += "\n#line 1\n" + p_glsl->baseValue();

        src.replace("//%mo_user_code%", code);
    }
    catch (Exception& e)
    {
        to->setErrorMessage(e.what());
        throw;
    }

    GL::Shader * shader;
    try
    {
        shader = to->createShaderQuad(src, texnames)->shader();
    }
    catch (Exception& e)
    {
        // copy error messages
        for (const GL::Shader::CompileMessage & msg : to->compileMessages())
            p_glsl->addErrorMessage(msg.line, msg.text);
        throw;
    }

    // uniforms

    u_color = shader->getUniform("u_color", false);
    u_res = shader->getUniform("iResolution");
    u_chan_res = shader->getUniform("iChannelResolution[0]");
    u_date = shader->getUniform("iDate");
    u_samplerate = shader->getUniform("iSampleRate");
    u_mouse = shader->getUniform("iMouse");
    u_frame = shader->getUniform("iFrame");
    uniformSetting->tieToShader(shader);
}

void ShaderTO::Private::releaseGl()
{

}

void ShaderTO::Private::renderGl(
        const GL::RenderSettings& , const RenderTime& time)
{
    // update uniforms

    if (u_color)
        u_color->setFloats(
            p_r->value(time),
            p_g->value(time),
            p_b->value(time),
            p_a->value(time));

    if (u_res)
    {
        const auto r = to->resolution();
        u_res->setFloats(r.width(), r.height());
    }

    if (u_chan_res)
    {
        gl::GLfloat data[3*4];
        memset(&data, 0, 3*4*sizeof(gl::GLfloat));
        int k=0;
        for (const ParameterTexture * p : to->textureParams())
        if (const GL::Texture * t = p->value(time))
        {
            data[k*3] = t->width();
            data[k*3+1] = t->height();
            data[k*2+2] = t->height() > 0
                                ? gl::GLfloat(t->width()) / t->height()
                                : gl::GLfloat(1);
            // XXX what is the 3rd value in shadertoy??
            // UPDATE Should be aspect, is usually 1.
            ++k;
        }
        MO_CHECK_GL_THROW( gl::glUniform3fv(u_chan_res->location(),
                                            4, &data[0]));
    }

    if (u_mouse)
    {
        Float x = MouseState::globalInstance().dragPos().x(),
              y = MouseState::globalInstance().size().height() - 1
                  - MouseState::globalInstance().dragPos().y();
        QSize res = to->resolution();
        if (!res.isEmpty())
        {
            x = MouseState::globalInstance().dragPosNorm().x() * res.width();
            y = (1. - MouseState::globalInstance().dragPosNorm().y()) * res.height();
        }
        u_mouse->setFloats(x, y,
                           MouseState::globalInstance().isDown(Qt::LeftButton),
                           MouseState::globalInstance().isDown(Qt::RightButton)
                    );
    }

    /** @todo should be a local frame count with restart ability !?? */
    if (u_frame)
        u_frame->floats[0] = to->renderCount();

    if (u_date)
    {
        const auto d = QDate::currentDate();
        u_date->setFloats(d.year(), d.month(), d.day(),
                          QTime::currentTime().second());
    }

    if (u_samplerate)
        u_samplerate->floats[0] = to->sampleRate();

    uint texSlot = 0;
    uniformSetting->updateUniforms(time, &texSlot);
    to->renderShaderQuad(0, time, &texSlot);
}


} // namespace MO
