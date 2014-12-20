/** @file modplayerao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.12.2014</p>
*/

#include "modplayerao.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterfilename.h"
#include "audio/tool/dumbfile.h"
#include "io/datastream.h"
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
        * paramAmp;
    ParameterFilename
        * paramFilename;

    AUDIO::DumbFile dumb;
};







ModPlayerAO::ModPlayerAO(QObject *parent)
    : AudioObject   (parent),
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

    params()->beginParameterGroup("mod", tr("playback"));


        p_->paramAmp = params()->createFloatParameter("dumb_amp", tr("amplitude"),
                                                   tr("The amplitude of the output"),
                                                   1.0, 0.05);
        p_->paramFilename = params()->createFilenameParameter("dumb_fn", tr("filename"),
                                                              tr("The location of the tracker file"),
                                                              IO::FT_TRACKER);
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

void ModPlayerAO::processAudio(uint , SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*>&
            outputs = audioOutputs(thread);

    p_->dumb.process(outputs, p_->paramAmp->value(sampleRateInv() * pos, thread));
}



void ModPlayerAO::Private::updateTrackerFile()
{
    if (paramFilename->value().isEmpty())
        dumb.close();
    else
    if (!dumb.isOpen() || dumb.filename() != paramFilename->value())
    {
        try
        {
            dumb.open(paramFilename->value());
        }
        catch (const Exception& e)
        {
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
