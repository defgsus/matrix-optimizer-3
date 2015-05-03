/** @file playbufferao.cpp

    @brief A audio buffer to play store audio data and files and play them

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include "playbufferao.h"

#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parameterfilename.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/soundfile.h"
#include "audio/tool/soundfilemanager.h"
#include "io/datastream.h"

#include <unistd.h>

namespace MO {

MO_REGISTER_OBJECT(PlayBufferAO)

class PlayBufferAO::Private
{
    public:

    enum Mode {
        M_BUFFER,
        M_FILE
    };

    Private(PlayBufferAO *ao)
        : ao(ao)
        , size(1.0)
        , readPtr(1)
        , writePtr(0)
    {
        buffer.resize(ao->sampleRate());
    }

    Mode mode() const { return (Mode)paramMode->baseValue(); }


    PlayBufferAO * ao;
    Double size;
    std::vector<F32> buffer;
    AUDIO::SoundFile * sndfile;
    uint readPtr, writePtr;
    ParameterFloat
            * paramAmp,
            * paramSize;
    ParameterFilename
            * paramFile;
    ParameterSelect
            * paramRec,
            * paramMode;
};

PlayBufferAO::PlayBufferAO(QObject * parent)
    : AudioObject(parent)
    , p_         (new Private(this))
{
    setName("PlayBuffer");
    setNumberAudioInputs(4);
    setNumberAudioOutputs(2);
}

PlayBufferAO::~PlayBufferAO()
{
    delete p_;
}

void PlayBufferAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aoplaybuffer", 1);
}

void PlayBufferAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aoplaybuffer", 1);
}

void PlayBufferAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
}

void PlayBufferAO::createParameters()
{
    AudioObject::createParameters();
    params()->beginParameterGroup("playbuffer", tr("playable buffer"));
    p_->paramAmp  = params()->createFloatParameter("playbuf_amp", tr("amplitude"),
                                                   tr("The amplitude of the signal"),
                                                   1.0, 0.05);
    p_->paramRec  = params()->createBooleanParameter("playbuf_rec", tr("record"),
                                                     tr("Record a signal into the buffer"),
                                                     tr("Start recording"),
                                                     tr("Stop recording"),
                                                     false);
    p_->paramSize = params()->createFloatParameter("playbuf_size", tr("length"),
                                                   tr("The length in seconds"),
                                                   1.0, 0.05);
    p_->paramFile = params()->createFilenameParameter("playbuf_file", tr("filename"),
                                                      tr("The file to play from the buffer"),
                                                      IO::FT_SOUND_FILE,
                                                      "data/audio/speek/84macs.wav");
    p_->paramMode = params()->createSelectParameter("playbuf_mode", tr("buffer mode"),
                                                    tr("Selects the mode tthe buffer is used"),
                                                    {"buffer", "file"},
                                                    {tr("Buffer"), tr("File")},
                                                    {tr("Play and record from and to buffer"), tr("Play from file")},
                                                    {Private::M_BUFFER, Private::M_FILE},
                                                    Private::M_BUFFER);
    params()->endParameterGroup();
    p_->sndfile = AUDIO::SoundFileManager::getSoundFile(p_->paramFile->value());
}

void PlayBufferAO::onParameterChanged(Parameter *p)
{
    AudioObject::onParameterChanged(p);
    if(p==p_->paramSize) {
        p_->size = p_->paramSize->baseValue();
        p_->buffer.resize(p_->size * p_->ao->sampleRate());
    } else if(p==p_->paramFile) {
        if(p_->sndfile != NULL) {
            AUDIO::SoundFileManager::releaseSoundFile(p_->sndfile);
            p_->sndfile = NULL;
        }
        p_->sndfile = AUDIO::SoundFileManager::getSoundFile(p_->paramFile->value());
    }
}

void PlayBufferAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();
    p_->size = p_->paramSize->baseValue();
    p_->buffer.resize(p_->size * p_->ao->sampleRate());

    if(p_->sndfile != NULL) {
        AUDIO::SoundFileManager::releaseSoundFile(p_->sndfile);
        p_->sndfile = NULL;
    }
    p_->sndfile = AUDIO::SoundFileManager::getSoundFile(p_->paramFile->value());
    //while(!p_->sndfile->ok()) usleep(1);
}

void PlayBufferAO::updateParameterVisibility()
{
    switch(p_->mode()) {
    case Private::M_BUFFER:
        p_->paramFile->setVisible(false);
        p_->paramRec->setVisible(true);
        p_->paramSize->setVisible(true);
        break;
    case Private::M_FILE:
        p_->paramFile->setVisible(true);
        p_->paramRec->setVisible(false);
        p_->paramSize->setVisible(false);
        break;
    }
}

QString PlayBufferAO::getAudioInputName(uint channel) const
{
    switch(channel)
    {
    case 0: return "read pos";
    case 1: return "write pos";
    case 2: return "rec gate";
    case 3: return "in";
    }
    return AudioObject::getAudioInputName(channel);
}

QString PlayBufferAO::getAudioOutputName(uint channel) const
{
    switch(channel)
    {
    case 0: return "out";
    case 1: return "length";
    }
    return AudioObject::getAudioOutputName(channel);
}

void PlayBufferAO::processBuffer(uint, SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs  = audioInputs(thread),
            outputs = audioOutputs(thread);

    AUDIO::AudioBuffer
            * inRead  = inputs.size() < 1  ? 0 : inputs[0],
            * inWrite = inputs.size() < 2  ? 0 : inputs[1],
            * inRec   = inputs.size() < 3  ? 0 : inputs[2],
            * in      = inputs.size() < 4  ? 0 : inputs[3],
            * out     = outputs.size() < 1 ? 0 : outputs[0];

    Double length = p_->size * sampleRate();
    //Double reqSize = p_->paramSize->value(sampleRateInv()*pos,thread);

    for(uint i=0;i<out->blockSize();++i) {
        Double
                time   = sampleRateInv() * (pos + i),
                amp    = p_->paramAmp->value(time, thread),
                rec    = (Double)p_->paramRec->value(time,thread),
                outval = 0.0;
        uint
                readPtr  = p_->readPtr,
                writePtr = p_->writePtr;
        if(inRead)
            readPtr += (uint)(inRead->read(i) * length) % p_->buffer.size();
        if(inWrite)
            writePtr += (uint)(inWrite->read(i) * length) % p_->buffer.size();
        if(inRec)
            rec *= inRec->read(i); /* only record on signal if selected */

        outval = p_->buffer[readPtr];
        if(rec > 0.0)
            p_->buffer[writePtr] = in->read(i);
        out->write(i, outval * amp);
    }
}

void PlayBufferAO::processFile(uint, SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs  = audioInputs(thread),
            outputs = audioOutputs(thread);

    AUDIO::AudioBuffer
            * inRead  = inputs.size() < 2  ? 0 : inputs[1],
            * out     = outputs.size() < 1 ? 0 : outputs[0];

    for(uint i=0;i<out->blockSize();++i) {
        Double
                time   = sampleRateInv() * (pos + i),
                amp    = p_->paramAmp->value(time, thread),
                outval = 0.0;
        uint
                readPtr  = p_->readPtr;

        if(inRead)
            readPtr += (uint)(inRead->read(i) * p_->sndfile->lengthSamples()) % p_->sndfile->lengthSamples();
        outval = p_->sndfile->value(readPtr);
        out->write(i, outval * amp);
    }
}

void PlayBufferAO::processAudio(uint bufferSize, SamplePos pos, uint thread)
{
    switch(p_->mode()) {
    case Private::M_FILE:
        processFile(bufferSize, pos, thread);
        break;
    case Private::M_BUFFER:
    default:
        processBuffer(bufferSize, pos, thread);
    }
}

}
