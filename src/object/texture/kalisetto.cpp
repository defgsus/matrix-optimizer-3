/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/9/2015</p>
*/

#include "kalisetto.h"
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

MO_REGISTER_OBJECT(KaliSetTO)

struct KaliSetTO::Private
{
    Private(KaliSetTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

    KaliSetTO * to;

    ParameterFloat
            *p_paramX, *p_paramY, *p_paramZ, *p_paramW,
            *p_offsetX, *p_offsetY, *p_offsetZ, *p_offsetW,
            *p_scale, *p_scaleX, *p_scaleY,
            *p_bright, *p_exponent, *p_freq;
    ParameterInt
            *p_numIter, *p_AntiAlias_;
    ParameterSelect
            *p_numDim, *p_colMode, *p_outMode, *p_mono;
    GL::Uniform
            * u_kali_param,
            * u_offset,
            * u_scale,
            * u_bright,
            * u_freq;
};


KaliSetTO::KaliSetTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("KaliSet");
    initMaximumTextureInputs(0);
}

KaliSetTO::~KaliSetTO()
{
    delete p_;
}

void KaliSetTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texkali", 1);
}

void KaliSetTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("texkali", 1);
}

void KaliSetTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void KaliSetTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void KaliSetTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void KaliSetTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
}


void KaliSetTO::Private::createParameters()
{
    to->params()->beginParameterGroup("kali", tr("kali set"));
    to->initParameterGroupExpanded("kali");

        p_numDim = to->params()->createSelectParameter(
                    "num_dim", tr("dimensions"),
                    tr("The number of dimensions to use in the formula"),
        { "2", "3", "4" },
        { tr("two"), tr("three"), tr("four") },
        { tr("Two-dimensional"), tr("Three-dimensional"), tr("Four-dimensional") },
        { 2, 3, 4 },
                    2, true, false);

        p_numIter = to->params()->createIntParameter(
                    "num_iter", tr("iterations"),
                    tr("The number of iterations on the kali set formula"),
                    15, true, false);
        p_numIter->setMinValue(1);

        p_paramX = to->params()->createFloatParameter(
                    "param_x", tr("parameter x"),
                    tr("The magic kali parameter, typical range is [-0.1, 1.5]"), 0.5, 0.01);

        p_paramY = to->params()->createFloatParameter(
                    "param_y", tr("parameter y"),
                    tr("The magic kali parameter, typical range is [-0.1, 1.5]"), 0.5, 0.01);

        p_paramZ = to->params()->createFloatParameter(
                    "param_z", tr("parameter z"),
                    tr("The magic kali parameter, typical range is [-0.1, 1.5]"), 0.5, 0.01);

        p_paramW = to->params()->createFloatParameter(
                    "param_w", tr("parameter w"),
                    tr("The magic kali parameter, typical range is [-0.1, 1.5]"), 0.5, 0.01);


        p_offsetX = to->params()->createFloatParameter(
                    "offset_x", tr("offset x"),
                    tr("Offset/position on x-axis"), 0., 0.01);

        p_offsetY = to->params()->createFloatParameter(
                    "offset_y", tr("offset y"),
                    tr("Offset/position on y-axis"), 0., 0.01);

        p_offsetZ = to->params()->createFloatParameter(
                    "offset_z", tr("offset z"),
                    tr("Offset/position on z-axis"), 0., 0.01);

        p_offsetW = to->params()->createFloatParameter(
                    "offset_w", tr("offset w"),
                    tr("Offset/position on w-axis"), 0., 0.01);


        p_scale = to->params()->createFloatParameter(
                    "scale", tr("scale"),
                    tr("Overall scale"), 1., 0.01);

        p_scaleX = to->params()->createFloatParameter(
                    "scale_x", tr("scale x"),
                    tr("Scale on x-axis"), 1., 0.01);

        p_scaleY = to->params()->createFloatParameter(
                    "scale_y", tr("scale y"),
                    tr("Scale on y-axis"), 1., 0.01);


    to->params()->endParameterGroup();


    to->params()->beginParameterGroup("color", tr("color"));

        p_AntiAlias_ = to->params()->createIntParameter(
                    "aa", tr("antialiasing"),
                    tr("Square root of the number of multi-samples"),
                    0, true, false);
        p_AntiAlias_->setMinValue(0);

        p_colMode = to->params()->createSelectParameter(
            "color_mode", tr("color mode"),
            tr("Defines the kind of value to use for the output color"),
            { "final", "average", "max", "min" },
            { tr("final"), tr("average"), tr("max"), tr("min") },
            { tr("The value at the last iteration step"),
              tr("The average of all values"),
              tr("The maximum of all values"),
              tr("The minimum of all values") },
            { 0, 1, 2, 3 },
            0, true, false);

        p_mono = to->params()->createBooleanParameter("mono", tr("monochrome"),
                                tr("Strictly monochrome colors"),
                                tr("Off"),
                                tr("On"),
                                false, true, false);

        p_bright = to->params()->createFloatParameter(
                    "brightness", tr("brightness"),
                    tr("Output brightness"), 1., 0.05);

        p_exponent = to->params()->createFloatParameter(
                    "exponent", tr("exponent"),
                    tr("Output exponent"), 1., 0.05);
        p_exponent->setMinValue(0.0);

        p_outMode = to->params()->createSelectParameter(
            "output_mode", tr("output"),
            tr("The function applied to the value"),
            { "linear", "sine", },
            { tr("linear"), tr("sine") },
            { tr("Unchanged linear"),
              tr("Sine transform of value") },
            { 0, 1 },
            0, true, false);

        p_freq = to->params()->createFloatParameter(
                    "freq", tr("frequency"),
                    tr("Frequency of the sine transform"), 10., 0.1);

    to->params()->endParameterGroup();
}

void KaliSetTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_colMode
        || p == p_->p_numIter
        || p == p_->p_numDim
        || p == p_->p_outMode
        || p == p_->p_mono
        || p == p_->p_AntiAlias_)
        requestReinitGl();

}

void KaliSetTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void KaliSetTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    int dim = p_->p_numDim->baseValue();

    p_->p_paramZ->setVisible(dim >= 3);
    p_->p_paramW->setVisible(dim >= 4);
    p_->p_offsetZ->setVisible(dim >= 3);
    p_->p_offsetW->setVisible(dim >= 4);

    p_->p_freq->setVisible( p_->p_outMode->baseValue() != 0);

}



void KaliSetTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/kaliset.frag");
        src.pasteDefaultIncludes();

        src.addDefine(QString("#define NUM_ITER %1").arg(p_numIter->baseValue()), false);
        src.addDefine(QString("#define NUM_DIM %1").arg(p_numDim->baseValue()), false);
        src.addDefine(QString("#define COL_MODE %1").arg(p_colMode->baseValue()), false);
        src.addDefine(QString("#define SINE_OUT %1").arg(p_outMode->baseValue()), false);
        src.addDefine(QString("#define MONOCHROME %1").arg(p_mono->baseValue()), false);
        src.addDefine(QString("#define AA %1").arg(p_AntiAlias_->baseValue()), false);
    }
    catch (Exception& e)
    {
        to->setErrorMessage(e.what());
        throw;
    }

    auto shader = to->createShaderQuad(
                src, { "u_tex" })->shader();

    // uniforms

    u_kali_param = shader->getUniform("u_kali_param", false);
    u_offset = shader->getUniform("u_offset", false);
    u_scale = shader->getUniform("u_scale", false);
    u_bright = shader->getUniform("u_bright", false);
    u_freq = shader->getUniform("u_freq", false);
}

void KaliSetTO::Private::releaseGl()
{

}

void KaliSetTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    // update uniforms

    if (u_kali_param)
    {
        u_kali_param->setFloats(
                    p_paramX->value(time, thread),
                    p_paramY->value(time, thread),
                    p_paramZ->value(time, thread),
                    p_paramW->value(time, thread));
    }

    if (u_offset)
    {
        u_offset->setFloats(
                    p_offsetX->value(time, thread),
                    p_offsetY->value(time, thread),
                    p_offsetZ->value(time, thread),
                    p_offsetW->value(time, thread));
    }

    if (u_scale)
    {
        Float s = p_scale->value(time, thread);
        u_scale->setFloats(
                    p_scaleX->value(time, thread) * s,
                    p_scaleY->value(time, thread) * s);
    }

    if (u_bright)
    {
        u_bright->setFloats(
                    p_bright->value(time, thread),
                    p_exponent->value(time, thread));
    }

    if (u_freq)
    {
        u_freq->floats[0] = p_freq->value(time, thread);
    }

    to->renderShaderQuad(time, thread);
}


} // namespace MO
