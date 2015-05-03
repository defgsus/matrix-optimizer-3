/** @file convolveao.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.05.2015</p>
*/

#include "convolveao.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterfilename.h"
#include "audio/tool/convolvebuffer.h"
#include "audio/tool/multifilter.h"
#include "audio/tool/soundfilemanager.h"
#include "audio/tool/soundfile.h"
#include "audio/tool/audiobuffer.h"
#include "math/constants.h"
#include "math/convolution.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(ConvolveAO)

struct ConvolveAO::Private
{
    public:

    void initConvolver(uint thread);

    ParameterFilename
            * pFile;
    ParameterFloat
            * pWet;

    std::vector<AUDIO::ConvolveBuffer> cbuf;

    bool needUpdate;
};

ConvolveAO::ConvolveAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private())
{
    setName("Convolver");
    setNumberAudioInputs(1);
    setNumberAudioOutputs(1);
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
                                                   1.,
                                                   0.05);

        p_->pFile = params()->createFilenameParameter("irfile", tr("impulse response"),
                                                   tr("An impulse response audio file"),
                                                   IO::FT_IMPULSE_RESPONSE);


    params()->endParameterGroup();
}

void ConvolveAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->cbuf.resize(count);
    p_->needUpdate = true;
}

void ConvolveAO::onParameterChanged(Parameter * p)
{
    if (p == p_->pFile)
        p_->needUpdate = true;
}

void ConvolveAO::Private::initConvolver(uint thread)
{
    AUDIO::ConvolveBuffer & conv = cbuf[thread];

    // load IR file
    QString fn = pFile->baseValue();
    if (fn.isEmpty())
        return;

    auto sf = AUDIO::SoundFileManager::getSoundFile(fn);
    if (!sf)
        return;

    auto sam = sf->getSamples();

    // set kernel
    conv.setKernel(&sam[0], sam.size());
}

void ConvolveAO::processAudio(uint , SamplePos pos, uint thread)
{
    if (p_->needUpdate)
    {
        p_->needUpdate = false; // XXX not reentrant
        p_->initConvolver(thread);
    }

    //const Double time = sampleRateInv() * pos;

    AUDIO::AudioBuffer::process(audioInputs(thread), audioOutputs(thread),
    [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        p_->cbuf[thread].process(in, out);
        //reverb->processMono(in->readPointer(), out->writePointer(), out->blockSize());
    });

}

} // namespace MO
