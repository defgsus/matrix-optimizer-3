/** @file fftao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.12.2014</p>
*/

#include "fftao.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/resamplebuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "math/fft.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(FftAO)

class FftAO::Private
{
    public:

    ParameterSelect
        * paramType;

    std::vector<MATH::Fft<F32>> ffts;
    std::vector<AUDIO::ResampleBuffer<F32>> inbufs, outbufs;
    std::vector<size_t> writtenOut;
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
    p_->inbufs.resize(count);
    p_->outbufs.resize(count);
    p_->writtenOut.resize(count);
}

void FftAO::setBufferSize(uint bufferSize, uint thread)
{
    AudioObject::setBufferSize(bufferSize, thread);

    // XXX Not called yet,
    // need new concept for buffersize signal from AudioEngine
    p_->ffts[thread].setSize(1024);
    p_->inbufs[thread].setSize(1024);
    p_->outbufs[thread].setSize(bufferSize);
    p_->writtenOut[thread] = 0;
}

void FftAO::processAudio(const QList<AUDIO::AudioBuffer *> &inputs,
                                const QList<AUDIO::AudioBuffer *> &outputs,
                                uint bSize, SamplePos pos, uint thread)
{
//    const Double time = sampleRateInv() * pos;

    const bool forward = p_->paramType->baseValue() == FT_FFT;

    // update filter
    MATH::Fft<F32> * fft = &p_->ffts[thread];
    AUDIO::ResampleBuffer<F32>
            * inbuf = &p_->inbufs[thread],
            * outbuf = &p_->outbufs[thread];

    // !!! THIS IS ILLEGAL (only debugging here)
    if (outbuf->blockSize() != bSize)
        outbuf->setSize(bSize);

    AUDIO::AudioBuffer::process(inputs, outputs,
    [=](const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        MO_DEBUG(idName() << " pos=" << pos);
        size_t read = 0, written = 0;
        bool dofft = true;
        do
        {
            // copy partial or whole input buffer
            read += inbuf->writeBlock(
                        in->readPointer() + read, in->blockSize() - read);

            MO_DEBUG("fft read=" << read << ", \twrit=" << written << ", \tin-left=" << inbuf->left() << ", \tout-left=" << outbuf->left());

            // perform fft
            if (inbuf->left() == 0 && dofft)
            {
                MO_DEBUG("FFT");
                inbuf->readBlock(fft->buffer());
                if (forward)
                    fft->fft();
                else
                    fft->ifft();

                dofft = false;
            }

            // send one fft (part) to output

            // chop to output size
            written += outbuf->writeBlock(
                            fft->buffer() + written, fft->size() - written);

            // and eventually send away
            if (outbuf->left() == 0)
            {
                MO_DEBUG("OUT");
                outbuf->readBlock(out->writePointer());
            }

        }
        // make sure we insert all of input buffer
        while (read < in->blockSize()
               // and write one output buffer
               && written < out->blockSize());

    });
}

} // namespace MO
