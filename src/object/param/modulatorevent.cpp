/** @file modulatorevent.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.12.2014</p>
*/

#include "modulatorevent.h"
#include "io/error.h"
#include "io/datastream.h"
#include "object/trackfloat.h"
#include "object/sequencefloat.h"
#include "object/modulatorobjectfloat.h"
#include "object/audioobject.h"

namespace MO {


ModulatorEvent::ModulatorEvent(
        const QString &name, const QString &modulatorId, Parameter * p, Object *parent)
    : Modulator     (name, modulatorId, p, parent),
      type_         (T_NONE),
      amplitude_    (1.0),
      timeOffset_   (0.0),
      buffer_       (16) // 2 pow x
{
}

void ModulatorEvent::serialize(IO::DataStream & io) const
{
    Modulator::serialize(io);

    io.writeHeader("mode", 1);

    io << amplitude_ << timeOffset_;
}

void ModulatorEvent::deserialize(IO::DataStream & io)
{
    Modulator::deserialize(io);

    io.readHeader("mode", 1);

    io >> amplitude_ >> timeOffset_;
}

void ModulatorEvent::copySettingsFrom(const Modulator *other)
{
    const ModulatorEvent * f = dynamic_cast<const ModulatorEvent*>(other);
    if (!f)
    {
        MO_WARNING("ModulatorEvent::copySettingsFrom(" << other << ") "
                   "with non-event type " << other->nameAutomatic());
        return;
    }

    amplitude_ = f->amplitude();
    timeOffset_ = f->timeOffset();
}

bool ModulatorEvent::canBeModulator(const Object * o) const
{
    MO_ASSERT(o, "ModulatorEvent::canBeModulator(NULL) called");

    return o->type() == Object::T_AUDIO_OBJECT;
}

bool ModulatorEvent::hasAmplitude() const
{
    return type_ == T_FLOAT;
}

bool ModulatorEvent::hasTimeOffset() const
{
    return type_ == T_FLOAT;
}


void ModulatorEvent::modulatorChanged_()
{
    if (modulator() == 0)
        type_ = T_NONE;
    else
    if (qobject_cast<AudioObject*>(modulator()))
        type_ = T_FLOAT;
    else
    {
        type_ = T_NONE;
        MO_ASSERT(false, "illegal assignment of modulator '" << modulator()->namePath()
                       << "' to ModulatorEvent");
    }
}


void ModulatorEvent::setValue(Double time, Double value)
{
    bufPos_ = (bufPos_ + 1) & mask_;
    buffer_[bufPos_].vfloat = value;
    buffer_[bufPos_].time = time;
}

const ModulatorEvent::Event * ModulatorEvent::getEvent(Double time) const
{
    // find newest event for requested time
    // by walking backwards the input buffer
    int i = bufPos_;
    const Event * closestEvent = &buffer_[i];
    Double best = 1e+100;
    for (size_t j = 0; j < buffer_.size(); ++j)
    {
        const Event * event = &buffer_[i];
        if (event->time <= time)
        {
            Double diff = time - event->time;
            if (diff < best)
            {
                best = diff;
                closestEvent = event;
            }
            else
                break;
        }

        i = (i - 1) & mask_;
    }

    return closestEvent;
}

Double ModulatorEvent::valueFloat(Double time, uint thread) const
{
    time += timeOffset_;

    if (!modulator() || !modulator()->active(time, thread))
        return 0.0;

    return amplitude() * getEvent(time)->vfloat;
}


} // namespace MO
