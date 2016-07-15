/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/15/2016</p>
*/

#include "fftfilterao.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertimeline1d.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/resamplebuffer.h"
#include "math/timeline1d.h"
#include "math/fft2.h"
#include "io/datastream.h"
#include "io/error.h"

namespace MO {

MO_REGISTER_OBJECT(FftFilterAO)

struct FftFilterAO::Private
{
    Private(FftFilterAO * ao)
        : ao(ao)
    {

    }

    struct PerThread;
    void updateBuffers(PerThread& p, size_t blockSize, size_t numChan);
    void updateFft(PerThread* pt);

    FftFilterAO * ao;

    ParameterFloat
            * pAmp;
    ParameterInt
            * pNumChannels;
    ParameterSelect
            * pMode,
            * pWindowSize;
    ParameterTimeline1D
            * pFilterCurve;

    struct PerThread
    {
        std::vector<F32> factors, window;
        MATH::OouraFFT<F32> fft;
        std::vector<AUDIO::ResampleBuffer<F32>> inRebuf, outRebuf;
        std::vector<AUDIO::AudioBuffer> inBuf;
        std::vector<std::vector<F32>> scratch1, scratch2;
    };
    std::vector<PerThread> perThread;
    bool curveChanged;
};

FftFilterAO::FftFilterAO()
    : AudioObject   (),
      p_            (new Private(this))
{
    setName("FftFilter");
    setNumberChannelsAdjustable(true);
}

FftFilterAO::~FftFilterAO()
{
    delete p_;
}

void FftFilterAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aofftfilter", 1);
}

void FftFilterAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aofftfilter", 1);
}

void FftFilterAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("filter", tr("filter"));
    initParameterGroupExpanded("filter");

        p_->pNumChannels = params()->createIntParameter(
                "num_channels", tr("number of channels"),
                tr("The number of channels to process"),
                1, 1, 16, 1, true, false);


        auto tl = new MATH::Timeline1d;
        tl->add(0., 1., MATH::TimelinePoint::LINEAR);
        tl->add(1., 1., MATH::TimelinePoint::LINEAR);
        p_->pFilterCurve = params()->createTimeline1DParameter(
                    "response", tr("response curve"),
                    tr("The response of the filter"),
                    tl, 0., 1., 0., 1., true);

        p_->pAmp = params()->createFloatParameter(
                    "amp", tr("amplitude"),
                    tr("The volume of the output signal"),
                   1.,
                   0.1);

        p_->pMode = params()->createSelectParameter(
                    "proc_mode", tr("processig mode"),
        tr("Mainly selects the output latency vs. computational expense"),
        { "per_buffer", "per_window" },
        { tr("per buffer"), tr("per fft-window") },
        { tr("The signals is filtered for each dsp-block"),
          tr("The signal is filters for each fft-window, which introduces a delay") },
        { 0, 1 },
          0, true, false);

        p_->pWindowSize = params()->createSelectParameter(
                    "fft_size", tr("fft size"),
            tr("The size of the fourier window in samples"),
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "", "", "", "", "", "", "", "", "", "", "", "", "" },
            { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
            1024, true, false);


    params()->endParameterGroup();

}

void FftFilterAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->pFilterCurve)
        p_->curveChanged = true;
}

void FftFilterAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    p_->curveChanged = true;
}

void FftFilterAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

}

void FftFilterAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
    p_->perThread.resize(num);
}

void FftFilterAO::setAudioBuffers(
        uint thread, uint bufferSize,
        const QList<AUDIO::AudioBuffer*>& inputs,
        const QList<AUDIO::AudioBuffer*>& outputs)
{
    p_->updateBuffers(p_->perThread[thread], bufferSize,
                      std::min(inputs.size(), outputs.size()));
}

void FftFilterAO::Private::updateBuffers(
        PerThread& p, size_t blockSize, size_t numChan)
{
    const size_t num = pWindowSize->baseValue();

    p.inRebuf.resize(numChan);
    p.outRebuf.resize(numChan);
    p.inBuf.resize(numChan);
    p.scratch1.resize(numChan);
    p.scratch2.resize(numChan);
    for (size_t i=0; i<numChan; ++i)
    {
        p.inRebuf[i].setSize(num/2);
        p.outRebuf[i].setSize(blockSize);
        p.inBuf[i].setSize(num/2, 3);
        p.scratch1[i].resize(num, 0.f);
        p.scratch2[i].resize(num, 0.f);
    }
}

void FftFilterAO::Private::updateFft(PerThread* pt)
{
    const size_t num = pWindowSize->baseValue();
    pt->fft.init(num);

    pt->factors.resize(pt->fft.ComplexSize());
    for (size_t i=0; i<pt->factors.size(); ++i)
    {
        pt->factors[i] = pFilterCurve->baseValue().get(
                    F32(i)/F32(pt->factors.size()-1));
    }

    pt->window.resize(num);
    for (size_t i=0; i<num/2; ++i)
    {
        F32 t = F32(i) / F32(num/2-1);
        if (t < .25)
            pt->window[i] = 0.f;
        else if (t >= .75)
            pt->window[i] = 1.f;
        else
            pt->window[i] = (t-.25) / .5;

    }
    for (size_t i=0; i<num/2; ++i)
        pt->window[num-1-i] = pt->window[i];
}



void FftFilterAO::processAudio(const RenderTime& time)
{
    auto inputs = audioInputs(time.thread()),
         outputs = audioOutputs(time.thread());

    if (inputs.isEmpty() || outputs.isEmpty())
        return;

    Private::PerThread* pt = &p_->perThread[time.thread()];

    if (pt->fft.size() != (size_t)p_->pWindowSize->baseValue()
        || p_->curveChanged)
    {
        p_->updateFft(pt);
        p_->curveChanged = false;
    }

    MO_ASSERT(pt->factors.size() == pt->fft.ComplexSize(), "");

    // per-window mode
    if (0)
    {
        MO_ASSERT(pt->inRebuf.size()
                  == (size_t)std::min(inputs.size(), outputs.size()), "");
        MO_ASSERT(pt->outRebuf.size()
                  == (size_t)std::min(inputs.size(), outputs.size()), "");

        AUDIO::AudioBuffer::process(inputs, outputs,
        [=](uint chan, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            pt->inRebuf[chan].push(in);
            auto scratch = pt->scratch1[chan].data();
            while (pt->inRebuf[chan].pop(scratch))
            {
                pt->fft.fft(scratch);

                pt->fft.multiply(scratch, pt->factors.data());

                pt->fft.ifft(scratch);
                for (size_t i=0; i<pt->fft.size(); ++i)
                    scratch[i] = pt->window[i];

                pt->outRebuf[chan].push(scratch, pt->fft.size());
            }

            pt->outRebuf[chan].pop(out);
        });
    }
    // overlap
    else
    {
        MO_ASSERT(pt->window.size() == pt->fft.size(), "");
        MO_ASSERT(pt->inRebuf.size()
                  == (size_t)std::min(inputs.size(), outputs.size()), "");
        MO_ASSERT(pt->outRebuf.size()
                  == (size_t)std::min(inputs.size(), outputs.size()), "");

        AUDIO::AudioBuffer::process(inputs, outputs,
        [=](uint chan, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            pt->inRebuf[chan].push(in);
            while (pt->inRebuf[chan].pop(pt->inBuf[chan].writePointer()))
            {
                const size_t halfSize = pt->fft.size() / 2;

                auto scratch1 = pt->scratch1[chan].data();
                // second overlapping window
                auto scratch2 = pt->scratch2[chan].data();

                std::cout << pt->inBuf[chan].curReadBlock() << std::endl;

                pt->inBuf[chan].readBlock(scratch1);
                pt->inBuf[chan].nextBlock();
                pt->inBuf[chan].readBlock(scratch1 + halfSize);
                pt->inBuf[chan].readBlock(scratch2);
                pt->inBuf[chan].nextBlock();
                pt->inBuf[chan].readBlock(scratch2 + halfSize);

                pt->inBuf[chan].nextBlock();
                pt->inBuf[chan].nextBlock();

#if 1
                // filter one window
                pt->fft.fft(scratch1);
                pt->fft.multiply(scratch1, pt->factors.data());
                pt->fft.ifft(scratch1);

                // filter one window
                pt->fft.fft(scratch2);
                pt->fft.multiply(scratch2, pt->factors.data());
                pt->fft.ifft(scratch2);
#endif
                // combine & windowing
                size_t winOfs1 = (pt->inBuf[chan].curWriteBlock() % 2) * halfSize,
                       winOfs2 = halfSize - winOfs1;
#if 1
                for (size_t i=0; i<pt->fft.size()/2; ++i)
                    scratch1[i] =
                            pt->window[i+winOfs1] * scratch1[i + halfSize]
                          + pt->window[i+winOfs2] * scratch2[i];
#endif
                pt->outRebuf[chan].push(scratch1, pt->fft.size()/2);
            }

            if (!pt->outRebuf[chan].pop(out))
                out->writeNullBlock();
        });
    }
}


} // namespace MO
