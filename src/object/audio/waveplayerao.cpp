/** @file waveplayerao.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#include "waveplayerao.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterselect.h"
#include "audio/tool/floatgate.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/soundfile.h"
#include "audio/tool/soundfilemanager.h"
#include "math/functions.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(WavePlayerAO)


class WavePlayerAO::Private
{
    public:

    enum TimeMode
    {
        M_SYNCED,
        M_FREE
    };

    Private(WavePlayerAO * ao)
        : ao        (ao)
        , wave      (0)
        , newWave   (0)
        , curTime   (0.)
        , curTimeAll(0.)
    { }

    void updateFile();
    void close();
    void processAudio(uint , SamplePos pos, uint thread);

    WavePlayerAO * ao;
    ParameterFloat
        * paramAmp,
        * paramPlay,
        * paramGate,
        * paramPlayPos,
        * paramOffset,
        * paramLength,
        * paramLoopStart,
        * paramLoopLength;
    ParameterSelect
        * paramLoadMem,
        * paramMode,
        * paramLoop;

    ParameterFilename
        * paramFilename;

    AUDIO::SoundFile * wave;
    volatile AUDIO::SoundFile * newWave;

    Double curTime, curTimeAll;
    AUDIO::FloatGate<Double> gate;
};







WavePlayerAO::WavePlayerAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private(this))
{
    setName("WavePlayer");

    setNumberAudioInputs(0);
    setNumberAudioOutputs(2);
}

WavePlayerAO::~WavePlayerAO()
{
    p_->close();
    if (p_->wave)
        p_->wave->release();
    delete p_;
}

void WavePlayerAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aowave", 1);
}

void WavePlayerAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aowave", 1);
}

void WavePlayerAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("play", tr("playback"));
    initParameterGroupExpanded("play");

        p_->paramFilename = params()->createFilenameParameter("fn", tr("filename"),
                                                          tr("Select a file to play"),
                                                          IO::FT_SOUND);

        p_->paramLoadMem = params()->createBooleanParameter("load_memory",
                                                           tr("load to memory"),
                tr("When selected, the whole file will be loaded into memory, "
                   "otherwise it gets streamed from disk"),
                tr("The file is streamed from disk"),
                tr("The file is completely loaded into memory"),
                true,
                true, false);

        p_->paramMode = params()->createSelectParameter("time_mode", tr("time mode"),
                                            tr("Selects the kind of timing to use"),
                { "scene", "free" },
                { tr("scene time"), tr("free time") },
                { tr("The audio file is played in-sync with the scene time"),
                  tr("The playback time only increases when playback is activated")},
                { Private::M_SYNCED, Private::M_FREE },
                  Private::M_SYNCED, true, false);

        p_->paramAmp = params()->createFloatParameter("amp", tr("amplitude"),
                                                   tr("The amplitude of the output"),
                                                   1.0, 0.05);

        p_->paramPlay = params()->createFloatParameter("play", tr("playback enable"),
                                                    tr("A value > 0 enables playback"),
                                                    1.0);

        p_->paramGate = params()->createFloatParameter("gate", tr("restart gate"),
                                                    tr("A gate signal to start playing at a new position"),
                                                    0.0);

        p_->paramPlayPos = params()->createFloatParameter("pos", tr("start position"),
                                                    tr("Where to start the playback on a gate signal (seconds)"),
                                                    0.0);
        p_->paramPlayPos->setMinValue(0);

        p_->paramOffset = params()->createFloatParameter("offset", tr("position offset"),
                                                    tr("The number of seconds that should be added to the playback time"),
                                                    0.0);

        p_->paramLength = params()->createFloatParameter("length", tr("maximum length"),
                                                    tr("When > 0, limits the length of playback (in seconds)"),
                                                    0.0);
        p_->paramLength->setMinValue(0);

        p_->paramLoop = params()->createBooleanParameter("loop", tr("looping"),
                                                    tr("Enables looping between a specified range"),
                                                    tr("Off"), tr("On"),
                                                    false, true, true);

        p_->paramLoopStart = params()->createFloatParameter("loop_start", tr("loop start position"),
                                                    tr("Where to start the looping (seconds)"),
                                                    0.0);
        p_->paramLoopStart->setMinValue(0);

        p_->paramLoopLength = params()->createFloatParameter("loop_length", tr("loop length"),
                                                    tr("The length of the loop in seconds"),
                                                    10.0);
        p_->paramLoopLength->setMinValue(0);

    params()->endParameterGroup();
}

void WavePlayerAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    p_->updateFile();
}

void WavePlayerAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->paramFilename || p == p_->paramLoadMem)
        p_->updateFile();
}

void WavePlayerAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    bool isSync = p_->paramMode->baseValue() == Private::M_SYNCED;
    bool isFree = p_->paramMode->baseValue() == Private::M_FREE;

    p_->paramOffset->setVisible(isSync);
    p_->paramGate->setVisible(isFree);
    p_->paramPlayPos->setVisible(isFree);
}

void WavePlayerAO::getNeededFiles(IO::FileList &files)
{
    AudioObject::getNeededFiles(files);

    files.append(IO::FileListEntry( p_->paramFilename->baseValue(), IO::FT_TRACKER) );
}

void WavePlayerAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
}

void WavePlayerAO::setAudioBuffers(uint , uint ,
                             const QList<AUDIO::AudioBuffer*>& ,
                             const QList<AUDIO::AudioBuffer*>& )
{

}

void WavePlayerAO::processAudio(uint bsize, SamplePos pos, uint thread)
{
    p_->processAudio(bsize, pos, thread);
}

void WavePlayerAO::Private::processAudio(uint bsize, SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*>&
            outputs = ao->audioOutputs(thread);

    // lazy exchange file
    if (newWave)
    {
        wave = (AUDIO::SoundFile*)newWave;
        newWave = 0;
    }

    Double  time = ao->sampleRateInv() * pos,
            maxLength = paramLength->value(time, thread);
    bool    doPlay = paramPlay->value(time, thread);
    TimeMode mode = (TimeMode)paramMode->value(time, thread);

    // empty output
    if (!wave || !doPlay)
    {
        ao->writeNullBlock(pos, thread);
        return;
    }

    bool    doLoop = paramLoop->value(time, thread);
    Double  dtime = time, // playback time
            blength = ao->sampleRateInv() * bsize; // bufferlength in seconds

    // synced time
    if (mode == M_SYNCED)
    {
        dtime += paramOffset->value(time, thread);

        // loop playback time
        if (doLoop)
        {
            Double  start = paramLoopStart->value(time, thread),
                    length = paramLoopLength->value(time, thread);
            if (dtime >= start && length != 0.)
                dtime = start + MATH::moduloSigned(dtime-start, length);
        }
    }

    // free time
    else if (mode == M_FREE)
    {
        // retrigger
        if (gate.input( paramGate->value(time, thread) ) > 0.)
        {
            curTime = paramPlayPos->value(time, thread);
            curTimeAll = 0.;
        }

        dtime = curTime;

        // playing length exceeded?
        if (maxLength > 0. && curTimeAll > maxLength + blength)
        {
            ao->writeNullBlock(pos, thread);
            return;
        }
    }

    // get wave data range
    Double wlen = wave->lengthSeconds();
    if (mode == M_SYNCED && maxLength > 0.)
        wlen = std::min(wlen, maxLength);

    // sample from wave data
    if (dtime > blength && dtime < wlen + blength)
    {
        wave->getResampled(outputs,
                       dtime * wave->sampleRate(),
                       ao->sampleRate(),
                       paramAmp->value(time, thread));
    }
    // zero output when outside file length
    else
    {
        for (auto o : outputs)
            if (o) o->writeNullBlock();
    }

    // forward playback time
    if (mode == M_FREE)
    {
        curTime += blength;
        curTimeAll += blength;

        // loop playback time
        if (doLoop)
        {
            Double  start = paramLoopStart->value(time, thread),
                    length = paramLoopLength->value(time, thread);
            if (length != 0. && curTime >= start)
            {
                curTime = start + MATH::moduloSigned(curTime-start, length);
            }
        }
    }
}



void WavePlayerAO::Private::updateFile()
{
    // get local filename
    QString fn = IO::fileManager().localFilename( paramFilename->value() );

    if (fn.isEmpty())
    {
        close();
        return;
    }

    if (!wave || wave->filename() != fn
        || wave->isStream() != !paramLoadMem->baseValue())
    {
        curTime = curTimeAll = 0.;

        auto wav = AUDIO::SoundFileManager::getSoundFile(fn, paramLoadMem->baseValue());
        if (!wav->isOk())
            close();
        else
        {
            ao->setNumberAudioOutputs(wav->numberChannels());
            newWave = wav;

            // update parameter ranges
            // XXX not fitting here conceptually but perfect place as well
            paramLoopLength->setDefaultValue(wav->lengthSeconds());
            paramLength->setDefaultValue(wav->lengthSeconds());
        }
    }
}

void WavePlayerAO::Private::close()
{
    // dispose queue
    if (newWave)
    {
        // cast away volatile
        auto wav = (AUDIO::SoundFile*)newWave;
        wav->release();
    }

    // tell audio thread
    newWave = 0;
}




} // namespace MO
