/** @file fftao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.12.2014</p>
*/

//#ifndef MO_DISABLE_EXP

#include <QMutex>
#include <QMutexLocker>

#include "FftAO.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/tool/ResampleBuffer.h"
#include "audio/tool/FixedBlockDelay.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterFloatMatrix.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "object/param/FloatMatrix.h"
#include "object/util/ObjectEditor.h"
#include "math/Fft.h"
#include "io/DataStream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(FftAO)

class FftAO::Private
{
    public:

    Private()
        : delayInSamples    (0)
        , hasMatrixChanged  (true)
    { }

    ParameterSelect
        * paramType,
        * paramSize,
        * paramCompensate,
        * paramMatrixOut;
    ParameterFloatMatrix
        * paramMatrix;

    std::vector<MATH::Fft<F32>> ffts;
    std::vector<AUDIO::ResampleBuffer<F32>> inbufs, outbufs;
    std::vector<AUDIO::FixedBlockDelay<F32>> delays;
    FloatMatrix matrix;

    size_t fftSize, delayInSamples;
    QMutex matrixMutex;
    bool hasMatrixChanged;

    std::vector<std::vector<F32>> ringBuffers;
    size_t ringWrite;
};

FftAO::FftAO()
    : AudioObject   (),
      p_            (new Private())
{
    setName("FFT");
    setNumberAudioInputsOutputs(1, 0);
    //setNumberOutputs(ST_FLOAT, 1);
    setNumberOutputs(ST_FLOAT_MATRIX, 1);
}

FftAO::~FftAO()
{
    delete p_;
}

size_t FftAO::fftSize() const { return p_->fftSize; }
size_t FftAO::delayInSamples() const { return p_->delayInSamples; }

QString FftAO::getOutputName(SignalType st, uint channel) const
{
    if (st == ST_FLOAT && channel == 0)
        return tr("delay");
    if (st == ST_FLOAT_MATRIX && channel == 0)
        return tr("matrix");
    return AudioObject::getOutputName(st, channel);
}

Double FftAO::valueFloat(uint , const RenderTime& ) const
{
    return delayInSamples();
}

FloatMatrix FftAO::valueFloatMatrix(uint , const RenderTime& ) const
{
    QMutexLocker lock(&p_->matrixMutex);
    p_->hasMatrixChanged = false;
    return p_->matrix;
}

bool FftAO::hasFloatMatrixChanged(
        uint , const RenderTime&) const
{
    QMutexLocker lock(&p_->matrixMutex);
    return p_->hasMatrixChanged;
}


QString FftAO::infoString() const
{
    return QString("fft-size=%1, delay=%2")
            .arg(fftSize())
            .arg(delayInSamples())
            ;
}

void FftAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aofft", 1);
}

void FftAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aofft", 1);
}

void FftAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("fft", tr("fourier transform"));
    initParameterGroupExpanded("fft");

        p_->paramType = params()->createSelectParameter("_fft_type", tr("mode"),
          tr("Selectes the type of fourier transform"),
          { "fft", "ifft", "ampphase" },
          { tr("audio -> fourier"), tr("fourier -> audio"),
            tr("audio -> amp/phase") },
          { tr("fast fourier transform"),
            tr("inverse fast fourier transform"),
            tr("fast fourier transform followed by conversion to amp and phase") },
          { FT_AUDIO_2_FFT,
            FT_FFT_2_AUDIO,
            FT_AUDIO_2_AMPPHASE },
          FT_AUDIO_2_FFT,
          true, false);

        p_->paramMatrixOut = params()->createSelectParameter(
                    "_fft_matrix_out", tr("matrix output"),
          tr("Selectes the type of output on the matrix connector"),
          { "off", "linear", "split" },
          { tr("off"),
            tr("one line"),
            tr("two lines") },
          { tr("No output"),
            tr("Whole data in one line"),
            tr("Data split into two lines, e.g. amplitude and phase") },
          { MM_OFF,
            MM_LINEAR,
            MM_SPLIT },
          MM_LINEAR,
          true, true);

        p_->paramSize = params()->createSelectParameter(
                    "_fft_size", tr("size"),
            tr("The size of the fourier window in samples"),
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "", "", "", "", "", "", "", "", "", "", "", "", "" },
            { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
            1024, true, false);

        p_->paramCompensate = params()->createBooleanParameter(
                    "_fft_compensate", tr("compensate latency"),
                    tr("Should the output be delayed to match the fft size to the "
                       "dsp buffer size"),
                    tr("yes"), tr("no"),
                    true, true, false);
        p_->paramCompensate->setZombie(true);

        p_->paramMatrix = params()->createFloatMatrixParameter(
                    "_fft_input", tr("input"),
                    tr("Input matrix"),
                    FloatMatrix(), false, true);

    params()->endParameterGroup();
}

void FftAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->paramSize)
        p_->fftSize = nextPowerOfTwo((uint)p_->paramSize->baseValue());
}

void FftAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    p_->fftSize = nextPowerOfTwo((uint)p_->paramSize->baseValue());
}

void FftAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->ffts.resize(count);
    p_->inbufs.resize(count);
    p_->outbufs.resize(count);
    p_->delays.resize(count);
    p_->ringBuffers.resize(count);
}

void FftAO::setAudioBuffers(uint thread, uint bufferSize,
                            const QList<AUDIO::AudioBuffer*>&,
                            const QList<AUDIO::AudioBuffer*>&)
{
    p_->ffts[thread].setSize(p_->fftSize);
    p_->inbufs[thread].setSize(p_->fftSize);
    p_->outbufs[thread].setSize(bufferSize);
    p_->ringBuffers[thread].resize(p_->fftSize);
    QMutexLocker lock(&p_->matrixMutex);
    p_->matrix.setDimensions({p_->fftSize});
}

void FftAO::processAudio(const RenderTime& time)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs = audioInputs(time.thread()),
            outputs = audioOutputs(time.thread());

    if (inputs.size() < 1 || inputs[0] == nullptr)
        return;

    const auto mode = p_->paramType->baseValue();
    const bool
            forward = (mode == FT_AUDIO_2_FFT || mode == FT_AUDIO_2_AMPPHASE),
            doConvertAmpPhase = mode == FT_AUDIO_2_AMPPHASE;
            //doCompensate = p_->paramCompensate->baseValue() != 0;

    MATH::Fft<F32> * fft = &p_->ffts[time.thread()];
    if (fft->size() != p_->fftSize)
        fft->setSize(p_->fftSize);

    // insert into ringbuffer
    std::vector<F32>& ringBuf = p_->ringBuffers[time.thread()];
    if (ringBuf.size() != fft->size())
        ringBuf.resize(fft->size());
    size_t num = std::min(inputs[0]->blockSize(), ringBuf.size());
    for (size_t i=0; i<num; ++i)
    {
        if (p_->ringWrite >= ringBuf.size())
            p_->ringWrite = 0;
        ringBuf[p_->ringWrite++] = inputs[0]->read(i);
    }

    // do transform
    /** @todo add a worker thread group to Scene or somewhere
        and do fft transform async for matrix outputs */
    memcpy(fft->buffer(), &ringBuf[0], fft->size() * sizeof(F32));
    if (forward)
    {
        fft->fft();
        if (doConvertAmpPhase)
            fft->toAmplitudeAndPhase();
    }
    else
        fft->ifft();

    // copy to matrix output
    auto mao = (MatrixMode)p_->paramMatrixOut->value(time);
    {
        QMutexLocker lock(&p_->matrixMutex);
        switch (mao)
        {
            case MM_OFF:
                if (!p_->matrix.isEmpty())
                    p_->matrix.clear();
            break;

            case MM_LINEAR:
            {
                std::vector<size_t> dim = { fft->size() };
                if (p_->matrix.hasDimensions(dim))
                    p_->matrix.setDimensions(dim);
                for (size_t i=0; i<fft->size(); ++i)
                    *p_->matrix.data(i) = fft->buffer(i);
            }
            break;

            case MM_SPLIT:
            {
                std::vector<size_t> dim = { fft->halfSize(), 2 };
                if (!p_->matrix.hasDimensions(dim))
                    p_->matrix.setDimensions(dim);
                for (size_t i=0; i<fft->halfSize(); ++i)
                {
                    *p_->matrix.data(i, 0) = fft->buffer(i);
                    *p_->matrix.data(i, 1) = fft->buffer(i+fft->halfSize());
                }
            }
            break;
        }

        p_->hasMatrixChanged = true;
    }

    // Not quite right
#if 0
    AUDIO::ResampleBuffer<F32>
            * inbuf = &p_->inbufs[time.thread()],
            * outbuf = &p_->outbufs[time.thread()];

    if (inbuf->size() != p_->fftSize)
        inbuf->setSize(p_->fftSize);
    if (outbuf->size() != time.bufferSize())
        outbuf->setSize(time.bufferSize());

    // calculate matching delays
    size_t buf1 = inbuf->size(), buf2 = outbuf->size();
    if (buf2 > buf1)
        std::swap(buf1, buf2);

    if (buf2 == buf1)
        p_->delayInSamples = 0;
    else
    {
        p_->delayInSamples = ((buf1/buf2 + 1) * buf2) % buf1;
        bool cong = buf1 % buf2 == 0;
        if (!cong || p_->delayInSamples >= buf2/2)
            p_->delayInSamples = buf1 - p_->delayInSamples;
    }

    AUDIO::FixedBlockDelay<F32>* delay = &p_->delays[time.thread()];
    if (doCompensate && (delay->size() != time.bufferSize()
                      || delay->delay() != p_->delayInSamples))
        delay->setSize(time.bufferSize(), p_->delayInSamples);

    auto inputMatrix = p_->paramMatrix->value(time);

    AUDIO::AudioBuffer::process(inputs, outputs,
    [=, &time, &inputMatrix](uint, const AUDIO::AudioBuffer * in,
                                         AUDIO::AudioBuffer * out)
    {
        if (inputMatrix.dimensions() == 1 &&
                inputMatrix.size(0) == p_->fftSize)
        {
            for (size_t i=0; i<p_->fftSize; ++i)
                fft->buffer()[i] = inputMatrix.data()[i];
            inbuf->push(fft->buffer(), p_->fftSize);
        }
        else
        {
            inbuf->push(in->readPointer(), in->blockSize());
        }

        while (inbuf->pop(fft->buffer()))
        {
            if (forward)
            {
                fft->fft();
                if (doConvertAmpPhase)
                    fft->toAmplitudeAndPhase();
            }
            else
                fft->ifft();

            outbuf->push(fft->buffer(), fft->size());
        }
        if (doCompensate)
        {
            if (!outbuf->pop(delay->scratchBuffer()))
                out->writeNullBlock();
            else
            {
                delay->writeFromScratchBuffer();
                delay->read(out->writePointer());
            }
        }
        else
            if (!outbuf->pop(out->writePointer()))
                out->writeNullBlock();

        // copy to matrix output
        {
            QMutexLocker lock(&p_->matrixMutex);
            //for (size_t i=0; i<in->blockSize(); ++i)
            //    p_->matrix.data()[i] = in->read(i);
            for (size_t i=0; i<p_->fftSize; ++i)
                p_->matrix.data()[i] = *(out->writePointer() + i);
        }
    });
#endif
}

/*  |512|512|
    |1024   |

               |128
    |384|384|384|=1152
    |1024      |

                         |256
    |384|384|384|384|384|384|=2304
    |2048                |

    | 384 | 384 | 384 |
    |128|

*/

} // namespace MO


//#endif // #ifndef MO_DISABLE_EXP
