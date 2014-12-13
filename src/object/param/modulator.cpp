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
#include "object/param/parameter.h"

namespace MO {


Modulator::Modulator(const QString &name,
                     const QString &id, const QString& outputId,
                     Parameter * p, Object *parent)
    : parent_       (parent),
      name_         (name),
      modulatorId_  (id),
      outputId_     (outputId),
      param_        (p)
{
    MO_DEBUG_MOD("Modulator::Modulator(" << id << ", " << outputId << ", " << parent << ")");
}


void Modulator::serialize(IO::DataStream & io) const
{
    io.writeHeader("mod", 2);

    io << modulatorId_;

    // v2
    io << outputId_;
}

void Modulator::deserialize(IO::DataStream & io)
{
    const auto ver = io.readHeader("mod", 2);

    io >> modulatorId_;

    if (ver >= 2)
        io >> outputId_;
}

QString Modulator::nameAutomatic() const
{
    QString n = modulator() ? modulator()->name() : "NULL";
    if (!outputId_.isEmpty())
        n += ":" + outputId_;
    n += " -> ";
    if (!parent())
        n += "NULL";
    else
    {
        n += parent()->name();
        if (param_)
            n += "." + param_->name();
    }
    return n;
}

bool Modulator::isAudioToFloatConverter() const
{
    return modulator_ && modulator_->isAudioObject()
            // and connecting to an audio output (should be in outputId)
            ;
}

void Modulator::setModulator(Object * object)
{
    MO_DEBUG_MOD("Modulator('" << (parent_? parent_->idName() : "NULL") << "')::setModulator('"
             << (object? object->idName() : "NULL") << "')");

    if (object)
        MO_ASSERT(canBeModulator(object), "invalid modulating object '"
                  << object->idName() << "' given to Modulator('" << name_ << "')::setModulator()");

    modulator_ = object;

    modulatorChanged_();
}

} // namespace MO
