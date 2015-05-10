/** @file colorto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#include "colorto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(ColorTO)

struct ColorTO::Private
{
    Private(ColorTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

    ColorTO * to;

    ParameterFloat
            * p_bright, * p_bright2, * p_r, * p_g, * p_b, * p_a,
            * p_sat, *p_hue, *p_gamma, *p_gamma_r, *p_gamma_g, *p_gamma_b;

    GL::Uniform
            * u_color,
            * u_hsv,
            * u_gamma_exp;
};


ColorTO::ColorTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Color");
    initMaximumTextureInputs(1);
}

ColorTO::~ColorTO()
{
    delete p_;
}

void ColorTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texcol", 1);
}

void ColorTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("texcol", 1);
}

void ColorTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void ColorTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void ColorTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void ColorTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
}


void ColorTO::Private::createParameters()
{
    to->params()->beginParameterGroup("color", tr("color"));
    to->initParameterGroupExpanded("color");

        p_hue = to->params()->createFloatParameter(
                    "hue", tr("hue"), tr("Hue value [0,1]"), 0.0,  0.05);

        p_r = to->params()->createFloatParameter("red", tr("red"), tr("Red amount of output color"), 1.0, 0.1);
        p_g = to->params()->createFloatParameter("green", tr("green"), tr("Green amount of output color"), 1.0, 0.1);
        p_b = to->params()->createFloatParameter("blue", tr("blue"), tr("Blue amount of output color"), 1.0, 0.1);
        p_a = to->params()->createFloatParameter("alpha", tr("alpha"),
                    tr("Defines the opaqueness/transparency of the output [0,1]"),
                    1.0,
                    0.0, 1.0, 0.05);

        p_sat = to->params()->createFloatParameter(
                    "saturation", tr("saturation"), tr("Saturation"), 1.0,  0.0, 1.,  0.1);

        p_bright = to->params()->createFloatParameter("bright", tr("brightness"), tr("Amplifier for output color"), 1.0, 0.1);


        p_gamma = to->params()->createFloatParameter(
                    "gamma", tr("gamma"), tr("Gamma"), 1.0,  0.001, 1000.,  0.1);

        p_gamma_r = to->params()->createFloatParameter(
                    "gammar", tr("gamma red"), tr("Gamma of red channel"), 1.0,  0.001, 1000.,  0.1);

        p_gamma_g = to->params()->createFloatParameter(
                    "gammag", tr("gamma green"), tr("Gamma of green channel"), 1.0,  0.001, 1000.,  0.1);

        p_gamma_b = to->params()->createFloatParameter(
                    "gammab", tr("gamma blur"), tr("Gamma of blue channel"), 1.0,  0.001, 1000.,  0.1);

        p_bright2 = to->params()->createFloatParameter("bright2", tr("brightness"), tr("Amplifier for output color after gamma"), 1.0, 0.1);

    to->params()->endParameterGroup();
}

void ColorTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    /*if (p == p_->p_)
        requestReinitGl();*/

}

void ColorTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void ColorTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();


}




void ColorTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/color.frag");
    }
    catch (Exception& )
    {
        // XXX Signal to gui
        throw;
    }

    auto shader = to->createShaderQuad(
                src, { "u_tex" })->shader();

    // uniforms

    u_color = shader->getUniform("u_color", false);
    u_hsv = shader->getUniform("u_hsv", false);
    u_gamma_exp = shader->getUniform("u_gamma_exp", false);
}

void ColorTO::Private::releaseGl()
{

}

void ColorTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    // update uniforms

    if (u_color)
    {
        float b = p_bright->value(time, thread);
        u_color->setFloats( p_r->value(time, thread) * b,
                            p_g->value(time, thread) * b,
                            p_b->value(time, thread) * b,
                            p_a->value(time, thread));
    }

    if (u_hsv)
        u_hsv->setFloats(   p_hue->value(time, thread),
                            p_sat->value(time, thread),
                            p_bright2->value(time, thread));

    if (u_gamma_exp)
    {
        Double g = p_gamma->value(time, thread);
        u_gamma_exp->setFloats( 1./(g*p_gamma_r->value(time, thread)),
                                1./(g*p_gamma_g->value(time, thread)),
                                1./(g*p_gamma_b->value(time, thread)));
    }

    to->renderShaderQuad(time, thread);
}


} // namespace MO
