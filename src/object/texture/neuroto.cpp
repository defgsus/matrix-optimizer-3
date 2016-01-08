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

    ParameterFloat
            *p_learnrate,
            *p_weight_amp, *p_weight_offset,
            *p_weight_local_amp, *p_weight_local_pow;
    ParameterInt
            *p_in_width, *p_in_height,
            *p_out_width, *p_out_height,
            *p_weight_width, *p_weight_height,
            *p_randomSeed;
    ParameterSelect
            *p_mode, *p_signed_input, *p_signed_output,
            *p_clamp_alpha,
            *p_tex_format_out, *p_tex_type_out;
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
        { "bypass", "fprop", "brop", "weightinit"
                    },
        { tr("bypass"), tr("forward propagation network"), tr("back propagation network"),
          tr("weight init") },
        { tr("copies states from input to output"),
          tr("A complex input->output processor with fixed weights"),
          tr("A complex input->output processor with expected input and learning ability"),
          tr("Initializes weights for a specific input/output architecture")
                    },
        { M_BYPASS, M_FPROP, M_FULL_BP, M_WEIGHT_INIT
                    },
          getNeuroMode(), true, false);

        p_->p_tex_format_out = params()->createTextureFormatParameter(
                        "tex_format_out", tr("output format"),
                        tr("The channel format of the output texture"));
        p_->p_tex_type_out = params()->createTextureTypeParameter(
                        "tex_type_out", tr("output type"),
                        tr("The precision of the output texture"), 32);

        p_->p_signed_input = params()->createBooleanParameter(
                    "signed_input", tr("signed input texture"),
                    tr("The value range of the input texture"),
                    tr("Unsigned values in the range [0,1], where 0.5 is actually zero"),
                    tr("Signed values, range dependend on texture format, typically [-1,1]"),
                    true, true, false);

        p_->p_signed_output = params()->createBooleanParameter(
                    "signed_output", tr("signed output texture"),
                    tr("The value range of the output texture"),
                    tr("Unsigned values in the range [0,1], where 0.5 is actually zero"),
                    tr("Signed values, range dependend on texture format, typically [-1,1]"),
                    true, true, false);

        p_->p_clamp_alpha = params()->createBooleanParameter(
                    "clamp_alpha", tr("clamp alpha channel"),
                    tr("Should alpha channel be clamped"),
                    tr("Alpha channel is zero, or a normal value in case of RGBA format"),
                    tr("Alpha channel is clamped to 1"),
                    false, true, false);

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

        p_->p_randomSeed = params()->createIntParameter(
                    "random_seed", tr("random seed"),
                    tr("Random variation seed"),
                    0, true, true);

        p_->p_learnrate = params()->createFloatParameter(
                    "learnrate", tr("learnrate"),
                    tr("The amount of change to the weights per iteration"),
                    0.01, 0., 1.,
                    0.001);

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

    textureParams()[NeuroGl::SLOT_INPUT]->setName(tr("input"));
    textureParams()[NeuroGl::SLOT_WEIGHT]->setName(tr("weights"));
    textureParams()[NeuroGl::SLOT_ERROR]->setName(tr("label"));
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
            weighti = mode == M_WEIGHT_INIT,
            hasInp = bypass || fprop || fullbp;

    p_->p_signed_input->setVisible(hasInp);
    p_->p_in_width->setVisible(weighti);
    p_->p_in_height->setVisible(weighti);
    p_->p_out_width->setVisible(bypass || fprop || weighti);
    p_->p_out_height->setVisible(bypass || fprop || weighti);
    p_->p_learnrate->setVisible(fullbp);
    p_->p_weight_width->setVisible(weighti);
    p_->p_weight_height->setVisible(weighti);
    p_->p_weight_local_amp->setVisible(weighti);
    p_->p_weight_local_pow->setVisible(weighti);

    textureParams()[NeuroGl::SLOT_INPUT]->setVisible(  bypass || fprop || fullbp );
    textureParams()[NeuroGl::SLOT_WEIGHT]->setVisible( fprop || fullbp );
    textureParams()[NeuroGl::SLOT_ERROR]->setVisible(  fullbp );
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
        return M_FULL_BP;

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

const GL::Texture * NeuroTO::valueTexture(uint chan, const RenderTime& ) const
{
    switch (chan)
    {
        case NeuroGl::SLOT_INPUT:
            { auto t = p_->neurogl->outputTexture(); if (t && t->isAllocated()) return t; }
        break;
        case NeuroGl::SLOT_WEIGHT:
            { auto t = p_->neurogl->weightTexture(); if (t && t->isAllocated()) return t; }
        break;
        //case NeuroGl::SLOT_ERROR:
        //    { auto t = p_->neurogl->; if (t->isAllocated()) return t; }
        //break;
    }
    return 0;
}


void NeuroTO::Private::updateNeuro(const RenderTime& rt)
{
    gl::GLenum glformat = (gl::GLenum)Parameters::getTexFormat(
                p_tex_format_out->baseValue(),
                p_tex_type_out->baseValue());

    auto texIn = to->inputTexture(NeuroGl::SLOT_INPUT, rt);

    neurogl->setOutputFormat((int)glformat);
    neurogl->setTypeDimension(GL::channelSize(glformat));
    std::cout << GL::channelSize(glformat) << std::endl;
    neurogl->setInputSigned(p_signed_input->baseValue());
    neurogl->setOutputSigned(p_signed_output->baseValue());
    neurogl->setClampAlpha(p_clamp_alpha->baseValue());


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

        case M_FULL_BP:
        {
            auto texWeight = to->inputTexture(NeuroGl::SLOT_WEIGHT, rt);
            auto texError = to->inputTexture(NeuroGl::SLOT_ERROR, rt);

            neurogl->setMode(NeuroGl::MODE_BPROP);
            neurogl->setInputTexture(texIn);
            neurogl->setWeightTexture(texWeight);
            neurogl->setErrorTexture(texError);
            neurogl->setLearnrate(p_learnrate->value(rt));
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
            neurogl->setRandomSeed(p_randomSeed->value(rt));
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
    p_->updateNeuro(time);
    MO_EXTEND_EXCEPTION(
                p_->neurogl->step(1);
    , "in NeuroGl of object '" << name() << "'");
}

} // namespace MO
