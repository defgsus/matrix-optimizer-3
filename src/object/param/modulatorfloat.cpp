/** @file modulatorfloat.cpp

    @brief Float modulator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "modulatorfloat.h"
#include "io/error.h"
#include "io/datastream.h"
#include "object/control/trackfloat.h"
#include "object/control/sequencefloat.h"
#include "object/control/modulatorobjectfloat.h"
#include "object/audioobject.h"
#include "object/interface/valuefloatinterface.h"

namespace MO {


ModulatorFloat::ModulatorFloat(const QString &name, const QString &modulatorId, const QString& outputId,
                               Parameter * p, Object *parent)
    : Modulator     (name, modulatorId, outputId, p, parent),
      sourceType_   (ST_NONE),
      channel_      (0),
      outStaticFloat_   (0),
      interface_    (0),
      amplitude_    (1.0),
      timeOffset_   (0.0)
{
}

void ModulatorFloat::serialize(IO::DataStream & io) const
{
    Modulator::serialize(io);

    io.writeHeader("modf", 1);

    io << amplitude_ << timeOffset_;
}

void ModulatorFloat::deserialize(IO::DataStream & io)
{
    Modulator::deserialize(io);

    io.readHeader("modf", 1);

    io >> amplitude_ >> timeOffset_;
}

void ModulatorFloat::copySettingsFrom(const Modulator *other)
{
    const ModulatorFloat * f = dynamic_cast<const ModulatorFloat*>(other);
    if (!f)
    {
        MO_WARNING("ModulatorFloat::copySettingsFrom(" << other << ") "
                   "with non-float type");
        return;
    }

    amplitude_ = f->amplitude();
    timeOffset_ = f->timeOffset();
}

bool ModulatorFloat::canBeModulator(const Object * o) const
{
    MO_ASSERT(o, "ModulatorFloat::canBeModulator(NULL) called");

    return (dynamic_cast<const ValueFloatInterface*>(o) != 0)

    //return o->type() == Object::T_TRACK_FLOAT
    //    || o->type() == Object::T_SEQUENCE_FLOAT
    //    || o->type() == Object::T_MODULATOR_OBJECT_FLOAT
        || (o->type() == Object::T_AUDIO_OBJECT)
            //&& !o->modulatorOutputs().isEmpty())

            ;
}

bool ModulatorFloat::hasAmplitude() const
{
    return true;
    /*
    return     sourceType_ == ST_SEQUENCE_FLOAT
            || sourceType_ == ST_TRACK_FLOAT
            || sourceType_ == ST_MODULATOR_OBJECT_FLOAT
            || sourceType_ == ST_AUDIO_OBJECT;
            */
}

bool ModulatorFloat::hasTimeOffset() const
{
    return sourceType_ != ST_AUDIO_OBJECT;
    /*
    return     sourceType_ == ST_SEQUENCE_FLOAT
            || sourceType_ == ST_TRACK_FLOAT
            || sourceType_ == ST_MODULATOR_OBJECT_FLOAT;*/
}


void ModulatorFloat::modulatorChanged_()
{
    outStaticFloat_ = 0;

    if (modulator() == 0)
        sourceType_ = ST_NONE;
    else
    if (auto iface = dynamic_cast<ValueFloatInterface*>(modulator()))
    {
        sourceType_ = ST_INTERFACE_FLOAT;
        interface_ = iface;
    }
    else

        /*
    if (qobject_cast<TrackFloat*>(modulator()))
        sourceType_ = ST_TRACK_FLOAT;
    else
    if (qobject_cast<SequenceFloat*>(modulator()))
        sourceType_ = ST_SEQUENCE_FLOAT;
    else
    if (qobject_cast<ModulatorObjectFloat*>(modulator()))
        sourceType_ = ST_MODULATOR_OBJECT_FLOAT;
    else
    */
    if (qobject_cast<AudioObject*>(modulator()))
    {
        sourceType_ = ST_AUDIO_OBJECT;
        channel_ = getAudioOutputChannel();
        //std::cout << "--- " << outputId() << " " << outputId().mid(7) << " " << channel_ << std::endl;
        //outStaticFloat_ = dynamic_cast<ModulatorOutputStaticFloat*>(modulator()->getModulatorOutput("0"));
        //MO_ASSERT(outStaticFloat_, "not even that?");
    }

    else
    {
        sourceType_ = ST_NONE;
        MO_ASSERT(false, "illegal assignment of modulator '" << modulator()->idName()
                       << "' to ModulatorFloat");
    }
}

Double ModulatorFloat::value(Double time, uint thread) const
{
    time += timeOffset_;

    if (!modulator() || !modulator()->active(time, thread))
        return 0.0;

    switch (sourceType_)
    {
        case ST_NONE:
            return 0.0;

        case ST_INTERFACE_FLOAT:
        {
            if (modulator()->isSequence())
            {
                auto seq = static_cast<SequenceFloat*>(modulator());
                // sequences in clips need a playing clip
                if (seq->parentClip() && !seq->parentClip()->isPlaying())
                    return 0.0;
            }
            return amplitude_ * interface_->value(time, thread);
        }

        case ST_AUDIO_OBJECT:
            //return amplitude_ * (outStaticFloat_ ? outStaticFloat_->value() : 0.0);
            return amplitude_ *
                    static_cast<AudioObject*>(modulator())->getAudioOutputAsFloat(channel_, time, MO_AUDIO_THREAD);
    }

    return 0.0;
}


} // namespace MO
