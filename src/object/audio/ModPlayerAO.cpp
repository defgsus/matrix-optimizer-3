/** @file modplayerao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.12.2014</p>
*/

#ifndef MO_DISABLE_DUMB


#include "ModPlayerAO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterFilename.h"
#include "audio/tool/DumbFile.h"
#include "audio/tool/FloatGate.h"
#include "io/FileManager.h"
#include "io/DataStream.h"
#include "io/error.h"


namespace MO {

MO_REGISTER_OBJECT(ModPlayerAO)


class ModPlayerAO::Private
{
    public:

    enum Mode
    {
        M_ModPlayer,
        M_EQUATION
    };

    Private(ModPlayerAO * ao)
        : ao        (ao)
    { }

    void updateTrackerFile();
    void updateTracker(uint bufferSize);

    ModPlayerAO * ao;
    ParameterFloat
        * paramAmp,
        * paramPlayGate,
        * paramPlayPos;
    ParameterFilename
        * paramFilename;

    AUDIO::DumbFile dumb;

    AUDIO::FloatGate<Double> gate;
};







ModPlayerAO::ModPlayerAO()
    : AudioObject   (),
      p_            (new Private(this))
{
    setName("ModPlayer");

    setNumberAudioInputs(0);
    setNumberAudioOutputs(2);
}

ModPlayerAO::~ModPlayerAO()
{
    delete p_;
}

void ModPlayerAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aodumb", 1);
}

void ModPlayerAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aodumb", 1);
}

void ModPlayerAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("play", tr("playback"));
    initParameterGroupExpanded("play");

        p_->paramAmp = params()->createFloatParameter("dumb_amp", tr("amplitude"),
                                                   tr("The amplitude of the output"),
                                                   1.0, 0.05);
        p_->paramFilename = params()->createFilenameParameter("dumb_fn", tr("filename"),
                                                              tr("The location of the tracker file"),
                                                              IO::FT_TRACKER);

        p_->paramPlayGate = params()->createFloatParameter("dumb_gate", tr("start gate"),
                                                    tr("A gate signal to start playing at a new position"),
                                                    0.0);

        p_->paramPlayPos = params()->createFloatParameter("dumb_pos", tr("start position"),
                                                    tr("Where to start the playback on a gate signal"),
                                                    0.0);
        p_->paramPlayPos->setMinValue(0);

    params()->endParameterGroup();
}

void ModPlayerAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();
    p_->updateTrackerFile();
}

void ModPlayerAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);
    if (p == p_->paramFilename)
        p_->updateTrackerFile();
}

void ModPlayerAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();
}

void ModPlayerAO::getNeededFiles(IO::FileList &files)
{
    AudioObject::getNeededFiles(files);

    files.append(IO::FileListEntry( p_->paramFilename->baseValue(), IO::FT_TRACKER) );
}

void ModPlayerAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);

    // Must the tracker really be reentrant??
}

void ModPlayerAO::setAudioBuffers(uint , uint bufferSize,
                             const QList<AUDIO::AudioBuffer*>& ,
                             const QList<AUDIO::AudioBuffer*>& )
{
    p_->updateTracker(bufferSize);
}

void ModPlayerAO::processAudio(const RenderTime& time)
{
    const QList<AUDIO::AudioBuffer*>&
            outputs = audioOutputs(time.thread());

    const Double
            gate = p_->gate.input(p_->paramPlayGate->value(time));

    // retrigger
    if (gate)
    {
        p_->dumb.setPosition(
                    p_->paramPlayPos->value(time));
    }

    p_->dumb.process(outputs, p_->paramAmp->value(time));
}



void ModPlayerAO::Private::updateTrackerFile()
{
    ao->clearError();
    if (paramFilename->value().isEmpty())
        dumb.close();
    else
    if (!dumb.isOpen() || dumb.filename() != paramFilename->value())
    {
        try
        {
            dumb.open(IO::fileManager().localFilename( paramFilename->value() ));
        }
        catch (const Exception& e)
        {
            ao->setErrorMessage(e.what());
            MO_WARNING("ModPlayerAO: on loading dumb file:\n" << e.what());
        }
    }
}

void ModPlayerAO::Private::updateTracker(uint bufferSize)
{
    AUDIO::Configuration conf(ao->sampleRate(), bufferSize, 0, 2);

    if (dumb.config() != conf)
        dumb.setConfig(conf);
}






} // namespace MO

#endif // #ifndef MO_DISABLE_DUMB
