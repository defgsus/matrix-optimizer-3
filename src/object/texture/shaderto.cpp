/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/7/2015</p>
*/

#include <QDate>

#include "shaderto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parametertexture.h"
#include "object/util/useruniformsetting.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/texture.h"
#include "math/functions.h"
#include "io/datastream.h"
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
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

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
            * u_time,
            * u_date,
            * u_samplerate;
};


ShaderTO::ShaderTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Shader");
    initMaximumTextureInputs(4);
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

void ShaderTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
}


void ShaderTO::Private::createParameters()
{
    to->params()->beginParameterGroup("shader", tr("shader"));
    to->initParameterGroupExpanded("shader");

        p_glsl = to->params()->createTextParameter("glsl", tr("glsl source"),
               tr("A piece of glsl code to calculate the pixel output"),
               TT_GLSL,
               "/* ShaderToy compatibile\n"
               "\n"
               " uniforms:\n"
               "   vec3  iResolution;              // resolution of output texture in pixels\n"
               "   float iGlobalTime;              // scene time in seconds\n"
               "   float iChannelTime[4];          // playback of channel in seconds (not defined yet)\n"
               "   vec3  iChannelResolution[4];    // resolution per channel in pixels\n"
               "   vec4  iMouse;                   // (not defined yet)\n"
               "   vec4  iDate;                    // year, month, day, time in seconds\n"
               "   float iSampleRate;              // sound sampling rate in Hertz\n"
               "   sampler2D iChannel0-3;          // input texture\n"
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

        p_r = to->params()->createFloatParameter("red", tr("red"), tr("Red amount of output"), 1.0,  0.,1.,  0.025);
        p_g = to->params()->createFloatParameter("green", tr("green"), tr("Green amount of output"), 1.0,  0.,1.,  0.025);
        p_b = to->params()->createFloatParameter("blue", tr("blue"), tr("Blue amount of output"), 1.0,  0.,1.,  0.025);
        p_a = to->params()->createFloatParameter("alpha", tr("alpha"), tr("Alpha amount of output"), 1.0,  0.,1.,  0.025);

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

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/shader.frag");
        src.pasteDefaultIncludes();
        src.replace("//%mo_user_code%",
                uniformSetting->getDeclarations()
                + "\n#line 0\n" + p_glsl->baseValue());
    }
    catch (Exception& e)
    {
        to->setErrorMessage(e.what());
        throw;
    }

    GL::Shader * shader;
    try
    {
        shader = to->createShaderQuad(
                src, { "iChannel0", "iChannel1", "iChannel2", "iChannel3" })->shader();
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
    u_time = shader->getUniform("iGlobalTime");
    u_res = shader->getUniform("iResolution");
    u_chan_res = shader->getUniform("iChannelResolution[0]");
    u_date = shader->getUniform("iDate");
    u_samplerate = shader->getUniform("iSampleRate");
    uniformSetting->tieToShader(shader);
}

void ShaderTO::Private::releaseGl()
{

}

void ShaderTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    // update uniforms

    if (u_color)
        u_color->setFloats(
            p_r->value(time, thread),
            p_g->value(time, thread),
            p_b->value(time, thread),
            p_a->value(time, thread));

    if (u_time)
        u_time->floats[0] = time;

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
        if (const GL::Texture * t = p->value(time, thread))
        {
            data[k*3] = t->width();
            data[k*3+1] = t->height();
            // XXX what is the 3rd value in shadertoy??
            ++k;
        }
        MO_CHECK_GL_THROW( gl::glUniform3fv(u_chan_res->location(), 4, &data[0]));
    }

    if (u_date)
    {
        const auto d = QDate::currentDate();
        u_date->setFloats(d.year(), d.month(), d.day(),
                          QTime::currentTime().second());
    }

    if (u_samplerate)
        u_samplerate->floats[0] = to->sampleRate();

    uint texSlot = 0;
    uniformSetting->updateUniforms(time, thread, texSlot);

    to->renderShaderQuad(0, time, thread, texSlot);
}


} // namespace MO
