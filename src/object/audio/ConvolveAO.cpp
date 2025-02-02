/** @file convolveao.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.05.2015</p>
*/

#define MO_USE_KLANGFALTER_FOR_NOW

#include <memory>

#include "ConvolveAO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterFilename.h"
#ifdef MO_USE_KLANGFALTER_FOR_NOW
#   include "audio/3rd/KlangFalter/FFTConvolver.h"
#else
#   include "audio/tool/convolvebuffer.h"
#endif
#include "audio/tool/MultiFilter.h"
#include "audio/tool/SoundFileManager.h"
#include "audio/tool/SoundFile.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/tool/Delay.h"
#include "math/constants.h"
#include "math/Convolution.h"
#include "io/DataStream.h"


namespace MO {

MO_REGISTER_OBJECT(ConvolveAO)

struct ConvolveAO::Private
{
    Private(ConvolveAO * ao) : ao(ao)
    {
        filterHp.setType(AUDIO::MultiFilter::T_FIRST_ORDER_HIGH);
    }

    void updateFilter();
    void initConvolver(size_t blockSize);

    ConvolveAO * ao;

    ParameterFilename
            * pFile;
    ParameterFloat
            * pWet,
            * pAmp,
            * pDelayTime,
            * pFeedback,
            * pfFreq,
            * pfFreqHp,
            * pfReso;
    ParameterInt
            * pChannel,
            * pfOrder;
    ParameterSelect
            * pPostProc,
            * pfType;

#ifdef MO_USE_KLANGFALTER_FOR_NOW
    ::fftconvolver::FFTConvolver convolver;
#else
    AUDIO::ConvolveBuffer convolver;
#endif
    AUDIO::AudioBuffer inbuf, outbuf, outbuf2;
    AUDIO::Delay<F32, int> delay;
    AUDIO::MultiFilter filter, filterHp;

    bool needUpdate, delayCleared;
};

ConvolveAO::ConvolveAO()
    : AudioObject   (),
      p_            (new Private(this))
{
    setName("Convolver");
    setNumberAudioInputsOutputs(1,1);
    //setNumberChannelsAdjustable(true);
}

ConvolveAO::~ConvolveAO()
{
    delete p_;
}

void ConvolveAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aoconvolve", 1);
}

void ConvolveAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aoconvolve", 1);
}

void ConvolveAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("_convolve", tr("convolver"));
    initParameterGroupExpanded("_convolve");

        p_->pWet = params()->createFloatParameter("wet", tr("dry/wet mix"),
                                                   tr("Mix between pure input signal and pure convolved signal"),
                                                   1.,  0., 1.,
                                                   0.05);

        p_->pAmp = params()->createFloatParameter("amp", tr("amplitude"),
                                                   tr("The volume of the convoluted signal"),
                                                   1.,
                                                   0.1);

        p_->pFile = params()->createFilenameParameter("irfile", tr("impulse response"),
                                                   tr("An impulse response audio file"),
                                                   IO::FT_IMPULSE_RESPONSE);

        p_->pChannel = params()->createIntParameter("ir_channel", tr("channel"),
                                                   tr("The channel in the impulse response file to use"),
                                                   0, true, false);
        p_->pChannel->setMinValue(0);

    params()->endParameterGroup();

    params()->beginParameterGroup("_postproc", tr("post processing"));

        p_->pPostProc = params()->createBooleanParameter("postproc", tr("enable"),
                                                   tr("Enables additional dsp effects"),
                                                   tr("Off"), tr("On"),
                                                   false);

        p_->pfFreqHp = params()->createFloatParameter("_filter_freq_hp", tr("high-pass frequency"),
                                                   tr("The frequency of a high-pass filter (about 6db/oct) in Hertz"),
                                                   400.0, 10.0);
        p_->pfFreqHp->setMinValue(1);

        p_->pfType = params()->createSelectParameter("_filter_type", tr("filter type"),
                                                  tr("Selectes the type of a second filter"),
                                                  AUDIO::MultiFilter::filterTypeIds,
                                                  AUDIO::MultiFilter::filterTypeNames,
                                                  AUDIO::MultiFilter::filterTypeStatusTips,
                                                  AUDIO::MultiFilter::filterTypeEnums,
                                                  AUDIO::MultiFilter::T_FIRST_ORDER_LOW, true, false);

        p_->pfOrder = params()->createIntParameter("_filter_order", tr("order"),
                                                   tr("The order (sharpness) of the filter for the 'nth order' types"),
                                                   2,
                                                   1, 10,
                                                   1, true, false);

        p_->pfFreq = params()->createFloatParameter("_filter_freq", tr("frequency"),
                                                   tr("The cutoff frequency of the filter in Hertz"),
                                                   1000.0, 10.0);
        p_->pfFreq->setMinValue(1);
        p_->pfReso = params()->createFloatParameter("_filter_reso", tr("resonance"),
                                                   tr("The filter steepness [0,1]"),
                                                   0.0, 0.0, 1.0, 0.02);


        p_->pDelayTime = params()->createFloatParameter("delaytime", tr("delay time"),
                                                   tr("Delay time in seconds - "
                                                      "note that it can not be smaller than the dsp buffer size"),
                                                   .25,  0., 60.,
                                                   0.1);

        p_->pFeedback = params()->createFloatParameter("delayfeed", tr("feedback"),
                                                   tr("Feedback delay amount"),
                                                   0.3,
                                                   0.1);

    params()->endParameterGroup();

}


void ConvolveAO::setSampleRate(uint samplerate)
{
    AudioObject::setSampleRate(samplerate);
    p_->needUpdate = true;
}

void ConvolveAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->pFile || p == p_->pChannel)
        p_->needUpdate = true;

    if (p == p_->pfType || p == p_->pfOrder)
        p_->updateFilter();
}

void ConvolveAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    p_->needUpdate = true;
    p_->updateFilter();
    //p_->initConvolver();
}

void ConvolveAO::Private::updateFilter()
{
    // set type and order
    // other parameters can/will be set in audio thread
    filter.setType((AUDIO::MultiFilter::FilterType)pfType->baseValue());
    filter.setOrder(pfOrder->baseValue());
    filter.updateCoefficients();
}

void ConvolveAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    auto type = (AUDIO::MultiFilter::FilterType)p_->pfType->baseValue();

    p_->pfOrder->setVisible(
                AUDIO::MultiFilter::supportsOrder(type));
    p_->pfReso->setVisible(
                AUDIO::MultiFilter::supportsResonance(type));
}

void ConvolveAO::getNeededFiles(IO::FileList &files)
{
    AudioObject::getNeededFiles(files);

    files << IO::FileListEntry(p_->pFile->baseValue(), IO::FT_IMPULSE_RESPONSE);
}

void ConvolveAO::Private::initConvolver(size_t blockSize)
{
    ao->clearError();

    // load IR file
    QString fn = pFile->baseValue();
    if (fn.isEmpty())
        return;

    auto sf = AUDIO::SoundFileManager::getSoundFile(fn);
    if (!sf->isOk())
    {
        sf->release();
        ao->setErrorMessage(QString("%1 not loaded").arg(fn));
        return;
    }

    auto sam = sf->getResampled(ao->sampleRate(),
                std::min(sf->numberChannels()-1, uint(pChannel->baseValue())));
    sf->release();

    // set kernel
#ifdef MO_USE_KLANGFALTER_FOR_NOW
    convolver.init(blockSize, sam.data(), sam.size());
#else
    convolver.setKernel(sam.data(), sam.size());
#endif
    delay.setNull();
    delayCleared = true;
}

void ConvolveAO::processAudio(const RenderTime& time)
{
    // init convolution buffer
    if (p_->needUpdate)
    {
        p_->needUpdate = false; // XXX not reentrant
        p_->initConvolver(time.bufferSize());
    }
#ifdef MO_USE_KLANGFALTER_FOR_NOW
    if (p_->convolver.blockSize() != time.bufferSize())
        p_->initConvolver(time.bufferSize());
#endif

    // init output buffer
    if (p_->outbuf.blockSize() != time.bufferSize())
    {
        // XXX not reentrant either
        p_->outbuf.setSize(time.bufferSize());
        p_->outbuf2.setSize(time.bufferSize());
    }

    bool doPp = p_->pPostProc->value(time);

    const F32 amp = p_->pAmp->value(time),
              wet = p_->pWet->value(time);
    F32 dtime=0.f, damt=0.f;

    // prepare postproc
    if (doPp)
    {
        // delay can at least be equal to buffer-size
        dtime = std::max(0., p_->pDelayTime->value(time) * sampleRate()
                                - time.bufferSize());
        damt = p_->pFeedback->value(time);

        // update delay
        if (p_->delay.size() <= dtime)
            p_->delay.resize(dtime+1);

        if (p_->inbuf.blockSize() != time.bufferSize())
            p_->inbuf.setSize(time.bufferSize());

        p_->delayCleared = false;

        // update filter settings
        // XXX Note that these are only updated at the beginning of one dsp block!
        Float   freq = p_->pfFreq->value(time),
                freqHp = p_->pfFreqHp->value(time),
                res = p_->pfReso->value(time);

        if (   p_->filter.resonance() != res
            || p_->filter.frequency() != freq
            || p_->filter.sampleRate() != sampleRate())
        {
            p_->filter.setFrequency(freq);
            p_->filter.setResonance(res);
            p_->filter.setSampleRate(sampleRate());
            p_->filter.updateCoefficients();
        }

        if (  p_->filterHp.frequency() != freqHp)
        {
            p_->filterHp.setFrequency(freqHp);
            p_->filterHp.updateCoefficients();
        }
    }
    else
    // reset delay history when switched off
    if (!p_->delayCleared)
    {
        p_->delay.setNull();
        p_->delayCleared = true;
    }

    AUDIO::AudioBuffer::process(audioInputs(time.thread()), audioOutputs(time.thread()),
    [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        if (!doPp)
        {
            // convolve
#ifdef MO_USE_KLANGFALTER_FOR_NOW
            p_->convolver.process(in->readPointer(), p_->outbuf.writePointer(),
                                  p_->outbuf.blockSize());
#else
            p_->convolver.process(in, &p_->outbuf);
#endif
        }
        else
        {
            // get input
            in->readBlock(p_->inbuf.writePointer());
            // mix delay feedback
            for (uint i = 0; i < time.bufferSize(); ++i)
                p_->inbuf.writePointer()[i] +=
                    damt * p_->delay.read(dtime - i);

            // convolve
#ifdef MO_USE_KLANGFALTER_FOR_NOW
            p_->convolver.process(p_->inbuf.readPointer(), p_->outbuf.writePointer(),
                                  p_->outbuf.blockSize());
#else
            p_->convolver.process(&p_->inbuf, &p_->outbuf);
#endif

            // filter
            p_->filter.process(p_->outbuf.readPointer(),
                               p_->inbuf.writePointer(), time.bufferSize());
            p_->filterHp.process(p_->inbuf.readPointer(),
                                 p_->outbuf.writePointer(), time.bufferSize());

            // write convoluted signal to delay line
            p_->delay.writeBlock(p_->outbuf.readPointer(), time.bufferSize());

        }
        // mix into output channel
        for (size_t i=0; i<time.bufferSize(); ++i)
            out->write(i, in->readPointer()[i] + wet * (
                        amp * p_->outbuf.writePointer()[i] - in->readPointer()[i])
                       );
    });

}

} // namespace MO
