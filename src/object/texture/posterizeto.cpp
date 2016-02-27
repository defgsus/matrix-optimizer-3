/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/9/2015</p>
*/

#include "posterizeto.h"
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

MO_REGISTER_OBJECT(PosterizeTO)

struct PosterizeTO::Private
{
    Private(PosterizeTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    PosterizeTO * to;

    ParameterFloat
            * p_posSteps, * p_colorSteps, * p_tolerance;
    ParameterSelect
            * p_shape, * p_smooth, * p_mono, * p_quantTex, * p_quantColor,
            * p_alpha;
    GL::Uniform
            * u_quant,
            * u_tolerance;
};


PosterizeTO::PosterizeTO()
    : TextureObjectBase ()
    , p_                (new Private(this))
{
    setName("Posterize");
    initMaximumTextureInputs(1);
    //initEnableColorRange(true);
}

PosterizeTO::~PosterizeTO()
{
    delete p_;
}

void PosterizeTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texposter", 1);
}

void PosterizeTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("texposter", 1);
}

void PosterizeTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void PosterizeTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void PosterizeTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void PosterizeTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void PosterizeTO::Private::createParameters()
{
    to->params()->beginParameterGroup("posterize", tr("posterize"));
    to->initParameterGroupExpanded("posterize");

        p_posSteps = to->params()->createFloatParameter(
                    "pos_steps", tr("number of shapes"),
                    tr("Number of shapes from top to bottom"), 50.);
        p_posSteps->setMinValue(1.);

        p_shape = to->params()->createSelectParameter(
                    "shape", tr("shape type"), tr("The type of shape for posterization"),
        { "hline", "vline", "cross", "circle", "diamond", "rect" },
        { tr("horizontal line"), tr("vertical line"), tr("cross"),
          tr("circle"), tr("diamond"), tr("rectangle") },
        { 0, 1, 2, 3, 4, 5 },
                    0, true, false);

        p_smooth = to->params()->createBooleanParameter("smooth", tr("smooth shape"),
                                tr("Smooth shape transitions"),
                                tr("Off"),
                                tr("On"),
                                false, true, false);

        p_tolerance = to->params()->createFloatParameter(
                    "tolerance", tr("tolerance"), tr("Tolerance / smoothness"), 0.5, 0.05);


        p_alpha = to->params()->createBooleanParameter("use_alpha", tr("use alpha"),
                                tr("Include alpha channel in posterization"),
                                tr("Off"),
                                tr("On"),
                                false, true, false);

        p_mono = to->params()->createBooleanParameter("mono", tr("monochrome"),
                                tr("Strictly monochrome colors"),
                                tr("Off"),
                                tr("On"),
                                false, true, false);

        p_quantColor = to->params()->createBooleanParameter("quant_color", tr("quantize colors"),
                                tr("Quantize the color values"),
                                tr("Off"),
                                tr("On"),
                                false, true, false);

        p_colorSteps = to->params()->createFloatParameter(
                    "color_steps", tr("color steps"), tr("Number of color steps"), 5.);
        p_colorSteps->setMinValue(1.);

        p_quantTex = to->params()->createBooleanParameter("quant_tex", tr("quantize texture"),
                                tr("Quantize the texture lookup position"),
                                tr("Off"),
                                tr("On"),
                                true, true, false);

    to->params()->endParameterGroup();
}

void PosterizeTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_shape
        || p == p_->p_smooth
        || p == p_->p_mono
        || p == p_->p_alpha
        || p == p_->p_quantColor
        || p == p_->p_quantTex)
        requestReinitGl();

}

void PosterizeTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void PosterizeTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    p_->p_colorSteps->setVisible( p_->p_quantColor->baseValue() );
    p_->p_tolerance->setVisible( p_->p_smooth->baseValue() );

}



void PosterizeTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/posterize.frag");

        src.addDefine(QString("#define QUANT_COLOR %1").arg(p_quantColor->baseValue()));
        src.addDefine(QString("#define QUANT_TEX %1").arg(p_quantTex->baseValue()));
        src.addDefine(QString("#define MONOCHROME %1").arg(p_mono->baseValue()));
        src.addDefine(QString("#define USE_ALPHA %1").arg(p_alpha->baseValue()));
        src.addDefine(QString("#define SMOOTH_STEP %1").arg(p_smooth->baseValue()));
        src.addDefine(QString("#define SHAPE %1").arg(p_shape->baseValue()));
    }
    catch (Exception& e)
    {
        to->setErrorMessage(e.what());
        throw;
    }

    auto shader = to->createShaderQuad(
                src, { "u_tex" })->shader();

    // uniforms

    u_quant = shader->getUniform("u_quant", false);
    u_tolerance = shader->getUniform("u_tolerance", false);
}

void PosterizeTO::Private::releaseGl()
{

}

void PosterizeTO::Private::renderGl(const GL::RenderSettings& , const RenderTime& time)
{
    // update uniforms

    if (u_quant)
    {
        u_quant->setFloats( p_posSteps->value(time),
                            p_colorSteps->value(time));
    }

    if (u_tolerance)
        u_tolerance->floats[0] = p_tolerance->value(time);

    to->renderShaderQuad(time);
}


} // namespace MO
