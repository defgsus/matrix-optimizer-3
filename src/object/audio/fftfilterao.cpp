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
#include "math/fftwindow.h"
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

    struct PerChannel
    {
        AUDIO::ResampleBuffer<F32> inRebuf, outRebuf;
        AUDIO::AudioBuffer inBuf;
        std::vector<std::vector<F32>> scratch;
        size_t curBlock;
    };

    struct PerThread
    {
        std::vector<F32> factors, window;
        MATH::OouraFFT<F32> fft;
        std::vector<PerChannel> perChan;
    };
    std::vector<PerThread> perThread;
    bool needsUpdate;
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

    if (p == p_->pFilterCurve
        || p == p_->pWindowSize)
        p_->needsUpdate = true;
}

void FftFilterAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    p_->needsUpdate = true;
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

    p.perChan.resize(numChan);
    for (Private::PerChannel& c : p.perChan)
    {
        c.curBlock = 0;
        c.outRebuf.setSize(blockSize);
        c.inRebuf.setSize(num/2);
        c.inBuf.setSize(num/2, 2);
        c.scratch.resize(3);
        for (auto& s : c.scratch)
            s.resize(num, 0.f);
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

    MATH::FftWindow::makeWindow(
        pt->window, num, MATH::FftWindow::T_COSINE);
}



void FftFilterAO::processAudio(const RenderTime& time)
{
    auto inputs = audioInputs(time.thread()),
         outputs = audioOutputs(time.thread());

    if (inputs.isEmpty() || outputs.isEmpty())
        return;

    Private::PerThread* pt = &p_->perThread[time.thread()];

    if (pt->fft.size() != (size_t)p_->pWindowSize->baseValue()
        || p_->needsUpdate)
    {
        p_->updateBuffers(*pt, time.bufferSize(),
                          std::min(inputs.size(), outputs.size()));
        p_->updateFft(pt);
        p_->needsUpdate = false;
    }

    MO_ASSERT(pt->factors.size() == pt->fft.ComplexSize(), "");

    // overlap
    {
        MO_ASSERT(pt->window.size() == pt->fft.size(), "");
        MO_ASSERT(pt->inRebuf.size()
                  == (size_t)std::min(inputs.size(), outputs.size()), "");
        MO_ASSERT(pt->outRebuf.size()
                  == (size_t)std::min(inputs.size(), outputs.size()), "");

/*
            1   2   3
          |-------|
      :.......|-------|
          :.......|-------|

*/
        AUDIO::AudioBuffer::process(inputs, outputs,
        [=](uint chan, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            Private::PerChannel* pc = &pt->perChan[chan];
            pc->inRebuf.push(in);
            // read a half-window
            while (pc->inRebuf.pop(pc->inBuf.writePointer()))
            {
                const size_t
                        size = pt->fft.size(),
                        halfSize = size / 2;

                // current window
                auto scratch1 = pc->scratch[pc->curBlock].data(),
                // previous window
                     scratch2 = pc->scratch[1-pc->curBlock].data();

                pc->inBuf.nextBlock();
                pc->inBuf.readBlock(scratch1 + halfSize);
                pc->inBuf.nextBlock();
                pc->inBuf.readBlock(scratch1);
                pc->inBuf.nextBlock();

                // apply window
                for (size_t i=0; i<size; ++i)
                    scratch1[i] *= pt->window[i];
#if 1
                // filter the window
                pt->fft.fft(scratch1);
                pt->fft.multiply(scratch1, pt->factors.data());
                pt->fft.ifft(scratch1);
#endif
                // combine
                for (size_t i=0; i<halfSize; ++i)
                    scratch1[i] += scratch2[i + halfSize];

                pc->outRebuf.push(scratch1, halfSize);
                pc->curBlock = (pc->curBlock + 1) & 1;
            }

            if (!pc->outRebuf.pop(out))
                out->writeNullBlock();
        });
    }
}


} // namespace MO
