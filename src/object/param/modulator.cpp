/** @file modulator.cpp

    @brief Abstract modulator class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "modulator.h"
#include "io/datastream.h"
#include "io/log.h"
#include "io/error.h"
#include "object/object.h"

namespace MO {


Modulator::Modulator(const QString &name, const QString &id, Object *parent)
    : parent_       (parent),
      name_         (name),
      modulatorId_  (id)
{
    MO_DEBUG_MOD("Modulator::Modulator(" << id << ", " << parent << ")");
}


void Modulator::serialize(IO::DataStream & io) const
{
    io.writeHeader("mod", 1);

    io << modulatorId_;
}

void Modulator::deserialize(IO::DataStream & io)
{
    io.readHeader("mod", 1);

    io >> modulatorId_;
}

void Modulator::setModulator(Object * object)
{
    MO_DEBUG_MOD("Modulator('" << (parent_? parent_->idName() : "") << "')::setModulator('"
             << (object? object->idName() : "") << "')");
    if (object)
        MO_ASSERT(canBeModulator(object), "invalid modulating object '"
                  << object->idName() << "' given to Modulator('" << name_ << "')::setModulator()");

    modulator_ = object;

    modulatorChanged_();
}

} // namespace MO
