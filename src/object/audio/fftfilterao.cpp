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
#include "object/param/parametertext.h"
#include "object/param/parametertimeline1d.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/resamplebuffer.h"
#include "math/timeline1d.h"
#include "math/fft2.h"
#include "math/fftwindow.h"
#include "math/funcparser/parser.h"
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

    struct PerWindow;
    struct PerThread;
    void createWindowParams(PerWindow*, const QString& suffix);
    void updateBuffers(PerThread& p, size_t blockSize, size_t numChan);
    void updateFft(PerThread* pt);
    void updateFactors(std::vector<F32>& factors, size_t num, PerWindow* pw);
    void mixFactors(const RenderTime& time, PerThread* pt);

    FftFilterAO * ao;

    ParameterFloat
            * pAmp,
            * pMix;
    ParameterInt
            * pNumChannels;
    ParameterSelect
            * pWindowSize,
            * pWindowType,
            * pDoMix,
            * pDoMixWin;

    struct Equation
    {
        PPP_NAMESPACE::Parser parser;
        PPP_NAMESPACE::Float bin, x, xr, N;
        Equation()
        {
            parser.variables().add(
                "i", &bin, tr("The current bin number").toStdString());
            parser.variables().add(
                "x", &x, tr("The position along the curve [0,1]").toStdString());
            parser.variables().add(
                "xr", &xr, tr("The position along the curve in radians").toStdString());
            parser.variables().add(
                "N", &N, tr("The number of bins").toStdString());
        }
    };

    /** Per window-function */
    struct PerWindow
    {
        ParameterSelect
                * pMode;
        ParameterTimeline1D
                * pFilterCurve;
        ParameterText
                * pEqu;
        ParameterFloat
                * pFloatInput,
                * pFloatInputRange;
    };

    struct PerChannel
    {
        AUDIO::ResampleBuffer<F32> inRebuf, outRebuf;
        AUDIO::AudioBuffer inBuf;
        std::vector<std::vector<F32>> scratch;
        size_t curBlock;
    };

    struct PerThread
    {
        std::vector<F32>
                factors[2], factorsMix,
                window, mixWindow;
        MATH::OouraFFT<F32> fft;
        F32 prevMix;
        std::vector<PerChannel> perChan;
    };
    std::vector<PerWindow> perWindow;
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
    params()->beginEvolveGroup(false);

        p_->pDoMix = params()->createBooleanParameter(
                    "do_mix", tr("mix two curves"),
                    tr("This enables a second response curve and a mixing value"),
                    tr("Off"), tr("On"), false, true, false);

        p_->perWindow.resize(2);
        p_->createWindowParams(&p_->perWindow[0], "");
        p_->createWindowParams(&p_->perWindow[1], "2");

        p_->pMix = params()->createFloatParameter(
                    "mix", tr("mix"),
                    tr("The mix value [0,1] between the two response curves"),
                   0.,
                   0.0, 1.0,
                   0.1);

        p_->pAmp = params()->createFloatParameter(
                    "amp", tr("amplitude"),
                    tr("The volume of the output signal"),
                   1.,
                   0.1);


        /*
        p_->pMode = params()->createSelectParameter(
                    "proc_mode", tr("processig mode"),
        tr("Mainly selects the output latency vs. computational expense"),
        { "per_buffer", "per_window" },
        { tr("per buffer"), tr("per fft-window") },
        { tr("The signals is filtered for each dsp-block"),
          tr("The signal is filters for each fft-window, which introduces a delay") },
        { 0, 1 },
          0, true, false);
        */

        p_->pWindowSize = params()->createSelectParameter(
                    "fft_size", tr("fft size"),
            tr("The size of the fourier window in samples"),
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "", "", "", "", "", "", "", "", "", "", "", "", "" },
            { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
            2048, true, false);

        p_->pWindowType = params()->createSelectParameter(
                    "fft_window", tr("window type"),
                    tr("The windowing function"),
        { "tri", "hanning", "blackman", "gauss", "flattop" },
        { tr("Triangular"), tr("Hanning"), tr("Blackman"),
          tr("Gauss"), tr("flat-top") },
        { "", "", "", "", "" },
        { MATH::FftWindow::T_TRIANGULAR,
          MATH::FftWindow::T_HANNING,
          MATH::FftWindow::T_BLACKMAN,
          MATH::FftWindow::T_GAUSS,
          MATH::FftWindow::T_FLATTOP,
                    },
          MATH::FftWindow::T_HANNING, true, false);

        p_->pDoMixWin = params()->createBooleanParameter(
                    "do_mix_win", tr("apply mixing window"),
            tr("Applies a windowing function before mixing the filtered windows"),
                    tr("Off"), tr("On"), true, true, true);

    params()->endEvolveGroup();
    params()->endParameterGroup();

}

void FftFilterAO::Private::createWindowParams(PerWindow* pw, const QString& suffix)
{
    pw->pMode = ao->params()->createSelectParameter(
                "response_mode" + suffix, tr("type") + " " + suffix,
                tr("Select the mode of defining the filter response"),
        { "tl", "equ", "input" },
        { tr("timeline"), tr("equation"), tr("float input") },
        { tr("Define the response by editing a timeline"),
          tr("Define the response with an equation"),
          tr("Define the response by a time-slice of a float input") },
        { 0, 1, 2 },
        0, true, false);

    auto tl = new MATH::Timeline1d;
    if (suffix.isEmpty())
    {
        tl->add(0., 1., MATH::TimelinePoint::LINEAR);
        tl->add(1., 0., MATH::TimelinePoint::LINEAR);
    }
    else
    {
        tl->add(0., 0., MATH::TimelinePoint::LINEAR);
        tl->add(1., 1., MATH::TimelinePoint::LINEAR);
    }
    pw->pFilterCurve = ao->params()->createTimeline1DParameter(
                "response" + suffix, tr("response curve") + " " + suffix,
                tr("The response of the filter"),
                tl, 0., 1., -10., 10., true);
    tl->releaseRef("finish create parameter");
    pw->pFilterCurve->setDefaultEvolvable(true);

    pw->pEqu = ao->params()->createTextParameter(
                "response_func" + suffix, tr("response equation") + " " + suffix,
                tr("An equation calculating the filter response"),
                TT_EQUATION,
                "i % 2 == 0", true, false);
    Equation tmpequ;
    pw->pEqu->setVariableNames(tmpequ.parser.variables().variableNames());
    pw->pEqu->setVariableDescriptions(
                tmpequ.parser.variables().variableDescriptions());

    pw->pFloatInput = ao->params()->createFloatParameter(
                "response_input" + suffix, tr("response input") + " " + suffix,
                tr("The input signal that is sliced in time to produce a"
                   "response curve"),
                0.0, 0.1);
    pw->pFloatInput->setVisibleGraph(true);

    pw->pFloatInputRange = ao->params()->createFloatParameter(
                "response_input_range" + suffix, tr("input time range") + " " + suffix,
                tr("The range of time to slice the response input signal"),
                1.0, 0.05);
}

void FftFilterAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (   p == p_->pDoMix
           || p == p_->perWindow[0].pMode
           || p == p_->perWindow[0].pFilterCurve
           || p == p_->perWindow[0].pEqu
           || p == p_->perWindow[1].pMode
           || p == p_->perWindow[1].pFilterCurve
           || p == p_->perWindow[1].pEqu
           || p == p_->pWindowSize
           || p == p_->pWindowType)
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

    const bool doMix = p_->pDoMix->baseValue();
    p_->pMix->setVisible(doMix);
    bool e = true;
    for (int i=0; i<2; ++i)
    {
        Private::PerWindow& pw = p_->perWindow[i];
        const int mode = pw.pMode->baseValue();
        pw.pMode->setVisible(e);
        pw.pFilterCurve->setVisible(e && mode == 0);
        pw.pEqu->setVisible(e && mode == 1);
        pw.pFloatInput->setVisible(e && mode == 2);
        pw.pFloatInputRange->setVisible(e && mode == 2);
        e = doMix;
    }
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
        c.outRebuf.setSize(blockSize); c.outRebuf.clear();
        c.inRebuf.setSize(num/2); c.inRebuf.clear();
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

    MATH::FftWindow w;
    w.makeWindow(
        pt->window, num, MATH::FftWindow::Type(pWindowType->baseValue()));
    w.alpha = .8;
    w.makeWindow(pt->mixWindow, num, MATH::FftWindow::T_HANNING);

    ao->clearError();
    pt->prevMix = -1.f;
    updateFactors(pt->factors[0], pt->fft.ComplexSize(), &perWindow[0]);
    if (pDoMix->baseValue())
        updateFactors(pt->factors[1], pt->fft.ComplexSize(), &perWindow[1]);
}

void FftFilterAO::Private::updateFactors(
        std::vector<F32>& factors, size_t num, PerWindow *pw)
{
    factors.clear(); factors.resize(num, F32(0));
    switch (pw->pMode->baseValue())
    {
        case 0:
            for (size_t i=0; i<num; ++i)
                factors[i] = pw->pFilterCurve->baseValue().get(F32(i)/F32(num-1));
        break;

        case 1:
        {
            Equation equ;
            if (!equ.parser.parse(pw->pEqu->baseValue().toStdString()))
            {
                pw->pEqu->addErrorMessage(0, tr("Parsing failed"));
                ao->setErrorMessage(tr("Parsing failed"));
                return;
            }

            for (size_t i=0; i<num; ++i)
            {
                equ.bin = i;
                equ.x = PPP_NAMESPACE::Float(i) / (num-1);
                equ.xr = equ.x * 3.14159265 * 2.;
                equ.N = num;
                factors[i] = equ.parser.eval();
                //std::cout << factors[i] << " ";
            }
            //std::cout << std::endl;
        }
    }
}

void FftFilterAO::Private::mixFactors(const RenderTime &time, PerThread *pt)
{
    bool isInput = false;
    // get response from float input
    size_t pwi=0;
    for (Private::PerWindow& pw : perWindow)
    {
        if (pw.pMode->baseValue() == 2)
        {
            isInput = true;
            const size_t num = pt->factors[pwi].size();
            const Double tInc = pw.pFloatInputRange->value(time) / num;
            RenderTime btime(time);
            for (size_t i=0; i<num; ++i)
                pt->factors[pwi][num-1-i] = pw.pFloatInput->value(
                            btime - tInc * i);
        }
        ++pwi;
    }

    // mix both factor arrays
    if (pDoMix->value(time))
    {
        const F32 mix = std::max(0.,std::min(1., pMix->value(time) ));
        if (isInput || mix != pt->prevMix)
        {
            pt->factorsMix.resize(pt->factors[0].size());
            for (size_t i=0; i<pt->factors[0].size(); ++i)
                pt->factorsMix[i] = pt->factors[0][i] * (1.f-mix)
                                  + pt->factors[1][i] * mix;
            pt->prevMix = mix;
        }
    }
}

void FftFilterAO::processAudio(const RenderTime& time)
{
    auto inputs = audioInputs(time.thread()),
         outputs = audioOutputs(time.thread());

    if (inputs.isEmpty() || outputs.isEmpty())
        return;

    Private::PerThread* pt = &p_->perThread[time.thread()];

    if (p_->needsUpdate
        || pt->fft.size() != (size_t)p_->pWindowSize->baseValue() )
    {
        p_->updateBuffers(*pt, time.bufferSize(),
                          std::min(inputs.size(), outputs.size()));
        p_->updateFft(pt);
        p_->needsUpdate = false;
    }

    MO_ASSERT(pt->factors[0].size() == pt->fft.ComplexSize(), "");
    MO_ASSERT(pt->factors[1].size() == pt->fft.ComplexSize(), "");

    MO_ASSERT(pt->window.size() == pt->fft.size(), "");
    MO_ASSERT(pt->perChan.size()
              == (size_t)std::min(inputs.size(), outputs.size()), "");


    const F32 amp = p_->pAmp->value(time);
    const bool doMixWin = p_->pDoMixWin->value(time);

/* overlap:
      |-------|
  :.......|-------|
      :.......|-------|
*/
    bool isFirst = true;
    AUDIO::AudioBuffer::process(inputs, outputs,
    [=, &isFirst](uint chan, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        Private::PerChannel* pc = &pt->perChan[chan];
        pc->inRebuf.push(in);
        // read a half-window
        while (pc->inRebuf.pop(pc->inBuf.writePointer()))
        {
            if (isFirst)
            {
                p_->mixFactors(time, pt);
                isFirst = false;
            }

            const F32* factors = pt->factors[0].data();
            if (p_->pDoMix->value(time))
                factors = pt->factorsMix.data();


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

            // apply pre-fft window
            for (size_t i=0; i<size; ++i)
                scratch1[i] *= pt->window[i];
#if 1
            // filter the window
            pt->fft.fft(scratch1);
            pt->fft.multiply(scratch1, factors);
            pt->fft.ifft(scratch1);
#endif
#if 1
            // apply mixing window
            if (doMixWin)
            for (size_t i=0; i<size; ++i)
                scratch1[i] *= pt->mixWindow[i];
#endif
            // combine
            for (size_t i=0; i<halfSize; ++i)
                scratch1[i] += scratch2[i + halfSize];

            pc->outRebuf.push(scratch1, halfSize);
            pc->curBlock = (pc->curBlock + 1) & 1;
        }

        if (!pc->outRebuf.pop(out))
            out->writeNullBlock();
        else
            out->multiply(amp);
    });
}


} // namespace MO
