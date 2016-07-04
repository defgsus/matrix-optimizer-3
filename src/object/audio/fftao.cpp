/** @file fftao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.12.2014</p>
*/

//#ifndef MO_DISABLE_EXP

#include "fftao.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/resamplebuffer.h"
#include "audio/tool/fixedblockdelay.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/util/objecteditor.h"
#include "math/fft.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(FftAO)

class FftAO::Private
{
    public:

    Private()
        : delayInSamples    (0)
    { }

    ParameterSelect
        * paramType,
        * paramSize,
        * paramCompensate;

    std::vector<MATH::Fft<F32>> ffts;
    std::vector<AUDIO::ResampleBuffer<F32>> inbufs, outbufs;
    std::vector<AUDIO::FixedBlockDelay<F32>> delays;

    size_t fftSize, delayInSamples;
};

FftAO::FftAO()
    : AudioObject   (),
      p_            (new Private())
{
    setName("FFT");
    setNumberAudioOutputs(1);
    setNumberOutputs(ST_FLOAT, 1);
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
    return AudioObject::getOutputName(st, channel);
}

Double FftAO::valueFloat(uint , const RenderTime& ) const
{
    return delayInSamples();
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
          { "fft", "ifft" },
          { tr("fft"), tr("ifft") },
          { tr("fast fourier transform"), tr("inverse fast fourier transform") },
          { FT_FFT, FT_IFFT },
          FT_FFT,
          true, false);

        p_->paramSize = params()->createSelectParameter(
                    "_fft_size", tr("size"),
            tr("The size of the fourier window in samples"),
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "", "", "", "", "", "", "", "", "", "", "", "", "" },
            { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
            1024, true, false);
        p_->fftSize = nextPowerOfTwo((uint)p_->paramSize->baseValue());

        p_->paramCompensate = params()->createBooleanParameter(
                    "_fft_compensate", tr("compensate latency"),
                    tr("Should the output be delayed to match the fft size to the "
                       "dsp buffer size"),
                    tr("yes"), tr("no"),
                    true, true, false);

    params()->endParameterGroup();
}

void FftAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->paramSize)
        p_->fftSize = nextPowerOfTwo((uint)p_->paramSize->baseValue());
}

void FftAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->ffts.resize(count);
    p_->inbufs.resize(count);
    p_->outbufs.resize(count);
    p_->delays.resize(count);
}

void FftAO::setAudioBuffers(uint thread, uint bufferSize,
                            const QList<AUDIO::AudioBuffer*>&,
                            const QList<AUDIO::AudioBuffer*>&)
{
    // fftsize must at least be the current buffer length
    uint fftsize = nextPowerOfTwo(std::max((uint)p_->paramSize->baseValue(),
                                           bufferSize));
    p_->ffts[thread].setSize(fftsize);
    p_->inbufs[thread].setSize(fftsize);
    p_->outbufs[thread].setSize(bufferSize);
}

void FftAO::processAudio(const RenderTime& time)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs = audioInputs(time.thread()),
            outputs = audioOutputs(time.thread());

    const bool
            forward = p_->paramType->baseValue() == FT_FFT,
            doCompensate = p_->paramCompensate->baseValue() != 0;

    MATH::Fft<F32> * fft = &p_->ffts[time.thread()];
    if (fft->size() != p_->fftSize)
        fft->setSize(p_->fftSize);

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

    AUDIO::AudioBuffer::process(inputs, outputs,
    [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        inbuf->push(in->readPointer(), in->blockSize());

        while (inbuf->pop(fft->buffer()))
        {
            if (forward)
                fft->fft();
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
    });
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
