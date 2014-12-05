/** @file fftao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.12.2014</p>
*/

#include "fftao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "math/fft.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(FftAO)

class FftAO::Private
{
    public:

    ParameterSelect
        * paramType;

    std::vector<MATH::Fft<F32>> ffts;
};

FftAO::FftAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private())
{
    setName("FFT");
    setNumberAudioOutputs(1);
}

FftAO::~FftAO()
{
    delete p_;
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
    params()->beginParameterGroup("fft", tr("fourier transform"));

        p_->paramType = params()->createSelectParameter("_fft_type", tr("mode"),
                                                  tr("Selectes the type of fourier transform"),
                                                  { "fft", "ifft" },
                                                  { tr("fft"), tr("ifft") },
                                                  { tr("fast fourier transform"), tr("inverse fast fourier transform") },
                                                  { FT_FFT, FT_IFFT },
                                                  FT_FFT,
                                                  true, false);
    params()->endParameterGroup();
}

void FftAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->ffts.resize(count);
}

void FftAO::setBufferSize(uint bufferSize, uint thread)
{
    AudioObject::setBufferSize(bufferSize, thread);

    p_->ffts[thread].setSize(bufferSize);
}

void FftAO::processAudio(const QList<AUDIO::AudioBuffer *> &inputs,
                                const QList<AUDIO::AudioBuffer *> &outputs,
                                uint , SamplePos , uint thread)
{
//    const Double time = sampleRateInv() * pos;

    const bool forward = p_->paramType->baseValue() == FT_FFT;

    // update filter
    MATH::Fft<F32> * fft = &p_->ffts[thread];

    AUDIO::AudioBuffer::process(inputs, outputs,
    [fft, forward](const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        in->readBlock(fft->buffer());
        if (forward)
            fft->fft();
        else
            fft->ifft();
        out->writeBlock(fft->buffer());
        //filter->process(in->readPointer(), out->writePointer(), out->blockSize());
    });
}

} // namespace MO
