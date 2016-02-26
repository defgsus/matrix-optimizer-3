/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/5/2016</p>
*/

#include "neuroto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterfont.h"
#include "object/param/parameterint.h"
#include "object/param/parametertexture.h"
#include "object/param/parameterselect.h"
#include "object/param/parametercallback.h"
#include "gl/neurogl.h"
#include "gl/texture.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(NeuroTO)

struct NeuroTO::Private
{
    Private(NeuroTO * to)
        : to            (to)
        , neurogl       (new NeuroGl())
        , doResetWeights(false)
        , p_mode        (0)
    {
    }

    ~Private()
    {
        delete neurogl;
    }

    /** Sets mode and updates object ins/outs and parameters */
    void setMode(Mode m);

    // lazily create or exchange resources
    // for changed parameters or input resolutions
    void updateNeuro(const RenderTime& rt);

    NeuroTO * to;

    NeuroGl * neurogl;

    bool doResetWeights;

    ParameterFloat
            *p_learnrate,
            *p_weight_amp, *p_weight_offset,
            *p_weight_local_amp, *p_weight_local_pow;
    ParameterInt
            *p_in_width, *p_in_height,
            *p_out_width, *p_out_height,
            *p_weight_width, *p_weight_height,
            *p_random_seed;
    ParameterSelect
            *p_mode,
            *p_signed_input, *p_signed_input_weight, *p_signed_input_error,
            *p_signed_output, *p_signed_output_weight, *p_signed_output_error,
            *p_clamp_alpha, *p_error_is_label,
            *p_tex_format_out, *p_tex_type_out,
            *p_activation;
    ParameterCallback
            *p_reset_weights;
};

NeuroTO::NeuroTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Neuro");
    initInternalFbo(false);
    initMaximumTextureInputs(3);
    setNumberOutputs(ST_TEXTURE, 3);
}

NeuroTO::~NeuroTO()
{
    delete p_;
}

void NeuroTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("toneuro", 1);
}

void NeuroTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("toneuro", 1);
}

void NeuroTO::createParameters()
{
    TextureObjectBase::createParameters();

    params()->beginParameterGroup("neuro", tr("neuro"));
    initParameterGroupExpanded("neuro");

        p_->p_mode = params()->createSelectParameter(
                    "nn_mode", tr("mode"),
                    tr("Component mode / functionality"),
        { "bypass", "fpbp", "fprop", "bprop", "weightinit", "error"
                    },
        { tr("bypass"), tr("perceptron network"),
          tr("forward propagation"), tr("weight back propagation"),
          tr("weight init"), tr("get error") },
        { tr("copies states from input to output"),
          tr("A full input->output processor with trainable weights"),
          tr("An input->output processor with fixed weights"),
          tr("Back propagation step for the weights of an input->output processor"),
          tr("Initializes a weight texture for a specific input->output architecture"),
          tr("Calculates the difference between two inputs")
                    },
        { M_BYPASS, M_FULL_BP, M_FPROP, M_BPROP, M_WEIGHT_INIT, M_ERROR
                    },
          getNeuroMode(), true, false);

        p_->p_tex_format_out = params()->createTextureFormatParameter(
                        "tex_format_out", tr("output format"),
                        tr("The channel format of the output texture"));
        p_->p_tex_type_out = params()->createTextureTypeParameter(
                        "tex_type_out", tr("output type"),
                        tr("The precision of the output texture"), 32);

        p_->p_signed_input = params()->createBooleanParameter(
                    "signed_input", tr("signed input state"),
                    tr("The value range of the input texture"),
                    tr("Unsigned values in the range [0,1], where 0.5 is actually zero"),
                    tr("Signed values, range dependend on texture format, typically [-1,1]"),
                    true, true, false);

        p_->p_signed_input_weight = params()->createBooleanParameter(
                    "signed_input_weigth", tr("signed input weights"),
                    tr("The value range of the weight input texture"),
                    tr("Unsigned values in the range [0,1], where 0.5 is actually zero"),
                    tr("Signed values, range dependend on texture format, typically [-1,1]"),
                    true, true, false);

        p_->p_signed_input_error = params()->createBooleanParameter(
                    "signed_input_error", tr("signed input error/label"),
                    tr("The value range of the error/label input texture"),
                    tr("Unsigned values in the range [0,1], where 0.5 is actually zero"),
                    tr("Signed values, range dependend on texture format, typically [-1,1]"),
                    true, true, false);

        p_->p_signed_output = params()->createBooleanParameter(
                    "signed_output", tr("signed output state"),
                    tr("The value range of the output texture"),
                    tr("Unsigned values in the range [0,1], where 0.5 is actually zero"),
                    tr("Signed values, range dependend on texture format, typically [-1,1]"),
                    true, true, false);

        p_->p_signed_output_weight = params()->createBooleanParameter(
                    "signed_output_weight", tr("signed output weights"),
                    tr("The value range of the weight output texture"),
                    tr("Unsigned values in the range [0,1], where 0.5 is actually zero"),
                    tr("Signed values, range dependend on texture format, typically [-1,1]"),
                    true, true, false);

        p_->p_signed_output_error = params()->createBooleanParameter(
                    "signed_output_error", tr("signed output error"),
                    tr("The value range of the error output texture"),
                    tr("Unsigned values in the range [0,1], where 0.5 is actually zero"),
                    tr("Signed values, range dependend on texture format, typically [-1,1]"),
                    true, true, false);

        p_->p_clamp_alpha = params()->createBooleanParameter(
                    "clamp_alpha", tr("clamp alpha channel"),
                    tr("Should alpha channel be clamped"),
                    tr("Alpha channel is zero, or a normal value in case of RGBA format"),
                    tr("Alpha channel is clamped to 1"),
                    false, true, false);

        p_->p_error_is_label = params()->createBooleanParameter(
                    "error_is_label", tr("error / label"),
                    tr("Switch between error (off) or label (on) input"),
                    tr("Input is expected to be the error between desired and output"),
                    tr("Input is the desired output, e.g. the 'label' of the input data"),
                    true, true, false);

        p_->p_in_width = params()->createIntParameter(
                    "tex_width_in", tr("input width"),
                    tr("Width of the input texture"),
                    16, true, false);
        p_->p_in_width->setMinValue(1);

        p_->p_in_height = params()->createIntParameter(
                    "tex_height_in", tr("input height"),
                    tr("Height of the input texture"),
                    16, true, false);
        p_->p_in_height->setMinValue(1);

        p_->p_out_width = params()->createIntParameter(
                    "tex_width_out", tr("output width"),
                    tr("Width of the output texture"),
                    16, true, false);
        p_->p_out_width->setMinValue(1);

        p_->p_out_height = params()->createIntParameter(
                    "tex_height_out", tr("output height"),
                    tr("Height of the output texture"),
                    16, true, false);
        p_->p_out_height->setMinValue(1);

        p_->p_weight_width = params()->createIntParameter(
                    "tex_width_weight", tr("weights width"),
                    tr("Width of the weight texture"),
                    16, true, false);
        p_->p_weight_width->setMinValue(1);

        p_->p_weight_height = params()->createIntParameter(
                    "tex_height_weight", tr("weights height"),
                    tr("Height of the weight texture"),
                    16, true, false);
        p_->p_weight_height->setMinValue(1);


        p_->p_activation = params()->createSelectParameter(
                    "nn_activation", tr("activation function"),
                    tr("Type of activation function in state transportation"),
        { "linear", "tanh", "logistic"
                    },
        { tr("linear"), tr("tanh"),
          tr("logistic")
                    },
        { tr("Pure linear activation"),
          tr("Tangens hyperbolicus [-1, 1]"),
          tr("Classic logistic sigmoid 1 / (1 + exp(x)), range [0, 1]")
                    },
        { NeuroGl::A_LINEAR, NeuroGl::A_TANH, NeuroGl::A_LOGISTIC
                    },
          NeuroGl::A_LINEAR, true, false);


        p_->p_learnrate = params()->createFloatParameter(
                    "learnrate", tr("learnrate"),
                    tr("The amount of change to the weights per iteration"),
                    0.01, 0., 1.,
                    0.001);

        p_->p_reset_weights = params()->createCallbackParameter(
                    "reset_weights", tr("reset weights"),
                    tr("Resets the currently learned weights to the given input weights"),
                    [=](){ p_->doResetWeights = true; }, true);

        p_->p_random_seed = params()->createIntParameter(
                    "random_seed", tr("random seed"),
                    tr("Random variation seed"),
                    0, true, true);

        p_->p_weight_amp = params()->createFloatParameter(
                    "weight_amp", tr("weight deviation"),
                    tr("The amount of random deviation to the weights"),
                    0.5, 0., 100.,
                    0.01);

        p_->p_weight_offset = params()->createFloatParameter(
                    "weight_offset", tr("weight mean"),
                    tr("The offset of the weight values"),
                    0.0, -100., 100.,
                    0.01);

        p_->p_weight_local_amp = params()->createFloatParameter(
                    "weight_local_amp", tr("weight localize amplitude"),
                    tr("The amount of local reception fields on the weights - "
                       "mapping an 2D input area to the respective output area"),
                    0.0, -100., 100.,
                    0.01);

        p_->p_weight_local_pow = params()->createFloatParameter(
                    "weight_local_pow", tr("weight localize exponent"),
                    tr("The exponent of the local reception field curve, "
                       "the higher, the more localized"),
                    1.0, 0.00001, 100000.,
                    0.1);

    params()->endParameterGroup();
}

void NeuroTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);
/*
    if (p == p_->p_mode
            || p == p_->p_tex_format_out
            || p == p_->p_tex_type_out)
        p_->doRecompile = true;
*/
    if (p == p_->p_mode)
        emitConnectionsChanged();
}

void NeuroTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void NeuroTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    const Mode mode = getNeuroMode();
    const bool
            bypass = mode == M_BYPASS,
            fprop = mode == M_FPROP,
            fullbp = mode == M_FULL_BP,
            bprop = fullbp || mode == M_BPROP,
            weighti = mode == M_WEIGHT_INIT,
            err = mode == M_ERROR,
            hasInp = bypass || fprop || bprop || err;

    p_->p_signed_input->setVisible(             hasInp);
    p_->p_signed_input_weight->setVisible(      fprop || bprop || fullbp);
    p_->p_signed_input_error->setVisible(       bprop || fullbp || err);
    p_->p_signed_output->setVisible(            bypass || fprop || fullbp);
    p_->p_signed_output_weight->setVisible(     bprop || fullbp || weighti);
    p_->p_signed_output_error->setVisible(      fullbp || err);

    p_->p_in_width->setVisible(                 weighti);
    p_->p_in_height->setVisible(                weighti);
    p_->p_out_width->setVisible(                bypass || fprop || weighti);
    p_->p_out_height->setVisible(               bypass || fprop || weighti);
    p_->p_activation->setVisible(               fprop || bprop || err);
    p_->p_learnrate->setVisible(                bprop);
    p_->p_weight_width->setVisible(             weighti);
    p_->p_weight_height->setVisible(            weighti);
    p_->p_random_seed->setVisible(              weighti);
    p_->p_weight_amp->setVisible(               weighti);
    p_->p_weight_offset->setVisible(            weighti);
    p_->p_weight_local_amp->setVisible(         weighti);
    p_->p_weight_local_pow->setVisible(         weighti);
    p_->p_error_is_label->setVisible(           fullbp);
    p_->p_reset_weights->setVisible(            fullbp);

    textureParams()[NeuroGl::SLOT_INPUT]->setVisible(  hasInp );
    textureParams()[NeuroGl::SLOT_WEIGHT]->setVisible( fprop || bprop );
    textureParams()[NeuroGl::SLOT_ERROR]->setVisible(  bprop || err);

    textureParams()[NeuroGl::SLOT_INPUT]->setName(tr("input"));
    textureParams()[NeuroGl::SLOT_WEIGHT]->setName(tr("weights"));
    textureParams()[NeuroGl::SLOT_ERROR]->setName(
                (fullbp && p_->p_error_is_label->baseValue())
                || err
                ? tr("label") : tr("error"));
}

QString NeuroTO::getOutputName(SignalType st, uint channel) const
{
    if (st == ST_TEXTURE)
    {
        switch (channel)
        {
            case NeuroGl::SLOT_INPUT: return tr("output");
            case NeuroGl::SLOT_WEIGHT: return tr("weights");
            case NeuroGl::SLOT_ERROR: return tr("errors");
        }
    }

    return TextureObjectBase::getOutputName(st, channel);
}

NeuroTO::Mode NeuroTO::getNeuroMode() const
{
    if (!p_->p_mode)
        return M_FPROP;

    return (Mode)p_->p_mode->baseValue();
}


void NeuroTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);

    p_->neurogl->updateGl();
}

void NeuroTO::releaseGl(uint thread)
{
    p_->neurogl->releaseGl();

    TextureObjectBase::releaseGl(thread);
}

GL::ShaderSource NeuroTO::valueShaderSource(uint index) const
{
    // XXX index is actually not the stage index....
    return p_->neurogl->shaderSource(index);
}

const GL::Texture * NeuroTO::valueTexture(uint chan, const RenderTime& ) const
{
    switch (chan)
    {
        case NeuroGl::SLOT_INPUT:
            { auto t = p_->neurogl->outputTexture(); if (t && t->isAllocated()) return t; }
        break;
        case NeuroGl::SLOT_WEIGHT:
            { auto t = p_->neurogl->weightOutputTexture(); if (t && t->isAllocated()) return t; }
        break;
        case NeuroGl::SLOT_ERROR:
            { auto t = p_->neurogl->errorOutputTexture(); if (t && t->isAllocated()) return t; }
        break;
    }
    return 0;
}


void NeuroTO::Private::updateNeuro(const RenderTime& rt)
{
    gl::GLenum glformat = (gl::GLenum)Parameters::getTexFormat(
                p_tex_format_out->baseValue(),
                p_tex_type_out->baseValue());

    auto texIn = to->inputTexture(NeuroGl::SLOT_INPUT, rt);

    // -- set common parameters --

    neurogl->setOutputFormat((int)glformat);
    neurogl->setTypeDimension(GL::channelSize(glformat));
    neurogl->setActivation((NeuroGl::Activation)p_activation->baseValue());

    neurogl->setInputSigned(p_signed_input->baseValue());
    neurogl->setInputWeightSigned(p_signed_input_weight->baseValue());
    neurogl->setInputErrorSigned(p_signed_input_error->baseValue());
    neurogl->setOutputSigned(p_signed_output->baseValue());
    neurogl->setOutputWeightSigned(p_signed_output_weight->baseValue());
    neurogl->setOutputErrorSigned(p_signed_output_error->baseValue());
    neurogl->setClampAlpha(p_clamp_alpha->baseValue());

    // -- set mode specific things --

    switch (to->getNeuroMode())
    {
        case M_BYPASS:
        {
            neurogl->setMode(NeuroGl::MODE_BYPASS);
            neurogl->setInputTexture(texIn);
            neurogl->setOutputRes(QSize(p_out_width->baseValue(),
                                        p_out_height->baseValue()));
        }
        break;

        case M_ERROR:
        {
            auto texError = to->inputTexture(NeuroGl::SLOT_ERROR, rt);

            neurogl->setMode(NeuroGl::MODE_ERROR);
            neurogl->setInputTexture(texIn);
            neurogl->setErrorTexture(texError);
        }
        break;

        case M_FPROP:
        {
            auto texWeight = to->inputTexture(NeuroGl::SLOT_WEIGHT, rt);

            neurogl->setMode(NeuroGl::MODE_FPROP);
            neurogl->setInputTexture(texIn);
            neurogl->setWeightTexture(texWeight);
            neurogl->setOutputRes(QSize(p_out_width->baseValue(),
                                        p_out_height->baseValue()));
        }
        break;

        case M_BPROP:
        {
            auto texWeight = to->inputTexture(NeuroGl::SLOT_WEIGHT, rt);
            auto texError = to->inputTexture(NeuroGl::SLOT_ERROR, rt);

            neurogl->setMode(NeuroGl::MODE_BPROP);
            neurogl->setInputTexture(texIn);
            neurogl->setWeightTexture(texWeight);
            neurogl->setErrorTexture(texError);
            neurogl->setOutputRes(QSize(p_out_width->baseValue(),
                                        p_out_height->baseValue()));
            neurogl->setLearnrate(p_learnrate->value(rt));
            neurogl->setErrorIsLabel(false);
        }
        break;

        case M_FULL_BP:
        {
            auto texWeight = to->inputTexture(NeuroGl::SLOT_WEIGHT, rt);
            auto texError = to->inputTexture(NeuroGl::SLOT_ERROR, rt);

            neurogl->setMode(NeuroGl::MODE_FULL_BP);
            neurogl->setInputTexture(texIn);
            neurogl->setWeightTexture(texWeight);
            neurogl->setErrorTexture(texError);
            neurogl->setLearnrate(p_learnrate->value(rt));
            neurogl->setErrorIsLabel(p_error_is_label->baseValue());
        }
        break;

        case M_WEIGHT_INIT:
        {
            neurogl->setMode(NeuroGl::MODE_WEIGHT_INIT);
            neurogl->setInputRes(QSize(p_in_width->baseValue(),
                                        p_in_height->baseValue()));
            neurogl->setOutputRes(QSize(p_out_width->baseValue(),
                                        p_out_height->baseValue()));
            neurogl->setWeightRes(QSize(p_weight_width->baseValue(),
                                        p_weight_height->baseValue()));
            neurogl->setRandomSeed(p_random_seed->value(rt));
            neurogl->setWeightInitAmp(p_weight_amp->value(rt));
            neurogl->setWeightInitOffset(p_weight_offset->value(rt));
            neurogl->setWeightInitLocalAmp(p_weight_local_amp->value(rt));
            neurogl->setWeightInitLocalPow(p_weight_local_pow->value(rt));
        }
        break;
    }
}

void NeuroTO::renderGl(const GL::RenderSettings&, const RenderTime& time)
{
    clearError();
    p_->updateNeuro(time);

    if (p_->doResetWeights)
    {
        p_->neurogl->setResetWeights(true);
        p_->doResetWeights = false;
    }

    try
    {
        p_->neurogl->step(1);
    }
    catch (Exception& e)
    {
        setErrorMessage(e.what());
        e << "\nin NeuroGl of object '" << name() << "'";
        throw;
    }
}

} // namespace MO
