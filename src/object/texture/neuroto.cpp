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

    ParameterFloat* p_learnrate;
    ParameterInt * p_out_width, *p_out_height;
    ParameterSelect* p_mode, *p_signed_input, *p_signed_output;
    ParameterSelect* p_tex_format_out, *p_tex_type_out;
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
        { "bypass", "fprop", "brop"
                    },
        { tr("bypass"), tr("forward propagation network"), tr("back propagation network")
                    },
        { tr("copies states from input to output"),
          tr("A complex input->output processor with fixed weights"),
          tr("A complex input->output processor with expected input and learning ability")
                    },
        { M_BYPASS, M_FPROP, M_FULL_BP
                    },
          getNeuroMode(), true, false);

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

        p_->p_tex_format_out = params()->createTextureFormatParameter(
                        "tex_format_out", tr("output format"),
                        tr("The channel format of the output texture"));
        p_->p_tex_type_out = params()->createTextureTypeParameter(
                        "tex_type_out", tr("output type"),
                        tr("The precision of the output texture"), 32);

        p_->p_learnrate = params()->createFloatParameter(
                    "learnrate", tr("learnrate"),
                    tr("The amount of change to the weights per iteration"),
                    0.01, 0., 1.,
                    0.001);

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
            fullbp = mode == M_FULL_BP;

    p_->p_out_width->setVisible(bypass);
    p_->p_out_height->setVisible(bypass);

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

    neurogl->setInputTexture(texIn);
    neurogl->setOutputFormat((int)glformat);
    neurogl->setInputSigned(p_signed_input->baseValue());
    neurogl->setOutputSigned(p_signed_output->baseValue());

    if (!texIn)
        return;

    switch (to->getNeuroMode())
    {
        case M_BYPASS:
        {
            neurogl->setMode(NeuroGl::MODE_BYPASS);
            neurogl->setOutputRes(QSize(p_out_width->baseValue(),
                                        p_out_height->baseValue()));
        }
        break;

        case M_FPROP:
        {
            auto texWeight = to->inputTexture(NeuroGl::SLOT_WEIGHT, rt);

            neurogl->setMode(NeuroGl::MODE_FPROP);
            neurogl->setWeightTexture(texWeight);
        }
        break;

        case M_FULL_BP:
        {
            auto texWeight = to->inputTexture(NeuroGl::SLOT_WEIGHT, rt);
            auto texError = to->inputTexture(NeuroGl::SLOT_ERROR, rt);

            neurogl->setMode(NeuroGl::MODE_BPROP);
            neurogl->setWeightTexture(texWeight);
            neurogl->setErrorTexture(texError);
            neurogl->setLearnrate(p_learnrate->value(rt));
        }
        break;
    }
}

void NeuroTO::renderGl(const GL::RenderSettings&, const RenderTime& time)
{
    p_->updateNeuro(time);
    p_->neurogl->step(1);
}

} // namespace MO
