/** @file mverbao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 04.12.2014</p>
*/

#ifndef MO_DISABLE_EXP

#include "mverbao.h"
#include "audio/3rd/MVerb.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "math/constants.h"
#include "audio/tool/multifilter.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(MVerbAO)

class MVerbAO::Private
{
    public:

    ParameterFloat
            * pDampingFreq,
            * pDensity,
            * pBandwidthFreq,
            * pDecay,
            * pPreDelay,
            * pSize,
            * pGain,
            * pMix,
            * pEarlyMix;

    std::vector<MVERB::MVerb<F32>> reverbs;
};

MVerbAO::MVerbAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private())
{
    setName("Reverb");
    setNumberAudioInputs(2);
    setNumberAudioOutputs(2);
}

MVerbAO::~MVerbAO()
{
    delete p_;
}

void MVerbAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aomverb", 1);
}

void MVerbAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aomverb", 1);
}

void MVerbAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("_mverb", tr("Reverb"));
    initParameterGroupExpanded("_mverb");

        MVERB::MVerb<F32> temp;
        temp.reset();

        p_->pDampingFreq = params()->createFloatParameter("_mverb_dampingfreq", tr("damping frequency"),
                                                   tr(""),
                                                   temp.getParameter(temp.DAMPINGFREQ),
                                                   0.1);
        p_->pDensity = params()->createFloatParameter("_mverb_density", tr("density"),
                                                   tr(""),
                                                   temp.getParameter(temp.DENSITY),
                                                   0.1);
        p_->pBandwidthFreq = params()->createFloatParameter("_mverb_bandwidthfreq", tr("bandwidth frequency"),
                                                   tr(""),
                                                   temp.getParameter(temp.BANDWIDTHFREQ),
                                                   0.1);
        p_->pDecay = params()->createFloatParameter("_mverb_decay", tr("decay frequency"),
                                                   tr(""),
                                                   temp.getParameter(temp.DECAY),
                                                   0.1);
        p_->pPreDelay = params()->createFloatParameter("_mverb_predelay", tr("pre-delay"),
                                                   tr(""),
                                                   temp.getParameter(temp.PREDELAY),
                                                   0.1);
        p_->pSize = params()->createFloatParameter("_mverb_size", tr("size"),
                                                   tr(""),
                                                   temp.getParameter(temp.SIZE),
                                                   0.1);
        p_->pGain = params()->createFloatParameter("_mverb_gain", tr("gain"),
                                                   tr(""),
                                                   temp.getParameter(temp.GAIN),
                                                   0.1);
        p_->pMix = params()->createFloatParameter("_mverb_mix", tr("mix"),
                                                   tr(""),
                                                   temp.getParameter(temp.MIX),
                                                   0.1);
        p_->pEarlyMix = params()->createFloatParameter("_mverb_earlymix", tr("early mix"),
                                                   tr(""),
                                                   temp.getParameter(temp.EARLYMIX),
                                                   0.1);



    params()->endParameterGroup();
}

void MVerbAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->reverbs.resize(count);
    for (MVERB::MVerb<F32> & reverb : p_->reverbs)
        reverb.reset();
}

void MVerbAO::processAudio(uint bsize, SamplePos pos, uint thread)
{
    const Double time = sampleRateInv() * pos;

    MVERB::MVerb<F32> * reverb = &p_->reverbs[thread];

    if (reverb->sampleRate() != sampleRate())
        reverb->setSampleRate(sampleRate());

    reverb->setParameter(reverb->DAMPINGFREQ, p_->pDampingFreq->value(time, thread));
    reverb->setParameter(reverb->DENSITY, p_->pDensity->value(time, thread));
    reverb->setParameter(reverb->BANDWIDTHFREQ, p_->pBandwidthFreq->value(time, thread));
    reverb->setParameter(reverb->DECAY, p_->pDecay->value(time, thread));
    reverb->setParameter(reverb->PREDELAY, p_->pPreDelay->value(time, thread));
    reverb->setParameter(reverb->SIZE, p_->pSize->value(time, thread));
    reverb->setParameter(reverb->GAIN, p_->pGain->value(time, thread));
    reverb->setParameter(reverb->MIX, p_->pMix->value(time, thread));
    reverb->setParameter(reverb->EARLYMIX, p_->pEarlyMix->value(time, thread));

    const QList<AUDIO::AudioBuffer*>&
            inputs = audioInputs(thread),
            outputs = audioOutputs(thread);

    reverb->process(inputs[0] ? inputs[0]->readPointer() : 0,
                    inputs[1] ? inputs[1]->readPointer() : 0,
                    outputs[0] ? outputs[0]->writePointer() : 0,
                    outputs[1] ? outputs[1]->writePointer() : 0,
                    bsize);

/*
    AUDIO::AudioBuffer::process(inputs, outputs,
    [reverb](const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        reverb->processMono(in->readPointer(), out->writePointer(), out->blockSize());
    });*/
}

} // namespace MO

#endif // #ifndef MO_DISABLE_EXP
