/** @file modulatorfloat.cpp

    @brief Float modulator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "ModulatorFloat.h"
#include "io/error.h"
#include "io/DataStream.h"
#include "object/control/TrackFloat.h"
#include "object/control/SequenceFloat.h"
#include "object/control/ModulatorObjectFloat.h"
#include "object/AudioObject.h"
#include "object/interface/ValueFloatInterface.h"

namespace MO {


ModulatorFloat::ModulatorFloat(const QString &name, const QString &modulatorId, const QString& outputId,
                               Parameter * p, Object *parent)
    : Modulator     (name, modulatorId, outputId, p, ST_FLOAT, parent),
      sourceType_   (ST_NONE),
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

    return
            (dynamic_cast<const ValueFloatInterface*>(o) != 0)
         || (o->type() == Object::T_AUDIO_OBJECT)
         || (o->hasAudioOutput())
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
    return true;
        //sourceType_ != ST_AUDIO_OBJECT;
}


void ModulatorFloat::modulatorChanged_()
{
    if (modulator() == 0)
    {
        sourceType_ = ST_NONE;
        return;
    }

    if (auto iface = dynamic_cast<ValueFloatInterface*>(modulator()))
    {
        sourceType_ = ST_INTERFACE_FLOAT;
        interface_ = iface;
        return;
    }

    //if (outputId().startsWith("_audio_"))
    if (dynamic_cast<AudioObject*>(modulator()))
    {
        sourceType_ = ST_AUDIO_OBJECT;
        return;
    }

    sourceType_ = ST_NONE;
    MO_ASSERT(false, "illegal assignment of modulator '" << modulator()->idName()
                   << "' to ModulatorFloat, outputId='" << outputId() << "'");
}

Double ModulatorFloat::value(const RenderTime& rtime) const
{
    RenderTime time(rtime);
    time += timeOffset_;

    if (!modulator() || !modulator()->active(time))
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
            return amplitude_ * interface_->valueFloat(outputChannel(), time);
        }

        case ST_AUDIO_OBJECT:
            time.setThread(MO_AUDIO_THREAD);
            return amplitude_ *
                    static_cast<AudioObject*>(modulator())
                        ->getAudioOutputAsFloat(outputChannel(), time);
    }

    return 0.0;
}


} // namespace MO
