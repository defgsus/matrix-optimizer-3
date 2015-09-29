/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/21/2015</p>
*/

#include "thresholdto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/datastream.h"
#include "io/log.h"


namespace MO {

MO_REGISTER_OBJECT(ThresholdTO)

struct ThresholdTO::Private
{
    Private(ThresholdTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

    ThresholdTO * to;

    ParameterFloat
            * p_r, * p_g, * p_b, * p_a,
            * p_thresh;
    ParameterSelect
            * p_invert;
    GL::Uniform
            * u_color,
            * u_settings;
};


ThresholdTO::ThresholdTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Threshold");
    initMaximumTextureInputs(1);
}

ThresholdTO::~ThresholdTO()
{
    delete p_;
}

void ThresholdTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texthresh", 1);
}

void ThresholdTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("texthresh", 1);
}

void ThresholdTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void ThresholdTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void ThresholdTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void ThresholdTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
}


void ThresholdTO::Private::createParameters()
{
    to->params()->beginParameterGroup("thresh", tr("threshold"));
    to->initParameterGroupExpanded("thresh");

        p_thresh = to->params()->createFloatParameter(
                    "thresh", tr("threshold"),
                    tr("Threshold value above which the color is interpreted as ON [0,1]"),
                    0.5, 0.05);

        p_r = to->params()->createFloatParameter("red", tr("red"), tr("Red contribution to threshold value"), 1.0, 0.1);
        p_g = to->params()->createFloatParameter("green", tr("green"), tr("Green contribution to threshold value"), 1.0, 0.1);
        p_b = to->params()->createFloatParameter("blue", tr("blue"), tr("Blue contribution to threshold value"), 1.0, 0.1);
        p_a = to->params()->createFloatParameter("alpha", tr("alpha"), tr("Alpha contribution to threshold value"), 0.0, 0.1);

        p_invert = to->params()->createBooleanParameter("invert", tr("invert"),
                                tr("Invert the selection"),
                                tr("No inversion"),
                                tr("Invert selection"),
                                false, true, false);

    to->params()->endParameterGroup();
}

void ThresholdTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_invert)
        requestReinitGl();

}

void ThresholdTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void ThresholdTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();


}




void ThresholdTO::Private::initGl()
{
    // --- threshold shader-quad ---

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/threshold.frag");
        src.pasteDefaultIncludes();
        src.addDefine(QString("#define INVERT %1")
                      .arg(p_invert->baseValue() ? "1" : "0"));
    }
    catch (const Exception& e)
    {
        to->setErrorMessage(e.what());
        throw;
    }

    auto shader = to->createShaderQuad(
                src, { "u_tex" })->shader();

    // uniforms

    u_color = shader->getUniform("u_color", false);
    u_settings = shader->getUniform("u_settings", false);
}

void ThresholdTO::Private::releaseGl()
{

}

void ThresholdTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    // update uniforms

    const Float
            r = p_r->value(time, thread),
            g = p_g->value(time, thread),
            b = p_b->value(time, thread),
            a = p_a->value(time, thread);

    if (u_color)
        u_color->setFloats( r, g, b, a );

    if (u_settings)
        u_settings->setFloats(
                            p_thresh->value(time, thread) * (r + g + b + a)
                            );

    to->renderShaderQuad(time, thread);
}


} // namespace MO
