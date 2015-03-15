/** @file fftao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.12.2014</p>
*/

#ifndef MO_DISABLE_EXP

#include "fftao.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/resamplebuffer.h"
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

    ParameterSelect
        * paramType,
        * paramSize;

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
    AudioObject::createParameters();

    params()->beginParameterGroup("fft", tr("fourier transform"));

        p_->paramType = params()->createSelectParameter("_fft_type", tr("mode"),
                                                  tr("Selectes the type of fourier transform"),
                                                  { "fft", "ifft" },
                                                  { tr("fft"), tr("ifft") },
                                                  { tr("fast fourier transform"), tr("inverse fast fourier transform") },
                                                  { FT_FFT, FT_IFFT },
                                                  FT_FFT,
                                                  true, false);

        p_->paramSize = params()->createSelectParameter("_fft_size", tr("size"),
                    tr("The size of the fourier window in samples"),
                    { "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" },
                    { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
                    { "", "", "", "", "", "", "", "", "", "", "", "", "" },
                    { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
                    1024, true, false);

    params()->endParameterGroup();
}

void FftAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->paramSize)
        // hack to get called setAudioBuffers again
        if (editor())
            editor()->emitAudioChannelsChanged(this);
}

void FftAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->ffts.resize(count);
    p_->inbufs.resize(count);
    p_->outbufs.resize(count);
    p_->writtenOut.resize(count);
}

void FftAO::setAudioBuffers(uint thread, uint bufferSize,
                            const QList<AUDIO::AudioBuffer*>&,
                            const QList<AUDIO::AudioBuffer*>&)
{
    // fftsize must at least be the current buffer length
    uint fftsize = nextPowerOfTwo(std::max((uint)p_->paramSize->baseValue(),
                                           bufferSize));
    p_->ffts[thread].setSize(fftsize);
    p_->inbufs[thread].setSize(p_->paramSize->baseValue());
    p_->outbufs[thread].setSize(bufferSize);
    p_->writtenOut[thread] = 0;
}

void FftAO::processAudio(uint , SamplePos , uint )
{
//    const QList<AUDIO::AudioBuffer*>&
//            inputs = audioInputs(thread),
//            outputs = audioOutputs(thread);

//    const Double time = sampleRateInv() * pos;

//    const bool forward = p_->paramType->baseValue() == FT_FFT;

//    MATH::Fft<F32> * fft = &p_->ffts[thread];

#ifdef MO__fft_and_buffersize_equal_is_easy_

    AUDIO::ResampleBuffer<F32>
            * inbuf = &p_->inbufs[thread],
            * outbuf = &p_->outbufs[thread];

    AUDIO::AudioBuffer::process(inputs, outputs,
    [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        inbuf->writeBlock( in->readPointer(), in->blockSize() );

        inbuf->readBlock(fft->buffer());
        if (forward)
            fft->fft();
        else
            fft->ifft();
        out->writeBlock(fft->buffer());
    });

#endif

#if 0
    AUDIO::AudioBuffer::process(inputs, outputs,
    [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
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
#endif
}

} // namespace MO

#endif // #ifndef MO_DISABLE_EXP
