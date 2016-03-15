/** @file modulator.cpp

    @brief Abstract modulator class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include <QRegExp>

#include "modulator.h"
#include "io/datastream.h"
#include "io/log_mod.h"
#include "io/error.h"
#include "object/object.h"
#include "object/param/parameter.h"
#include "tool/stringmanip.h"

namespace MO {


Modulator::Modulator(const QString &name,
                     const QString &id, const QString& outputId,
                     Parameter * p, SignalType sigType, Object *parent)
    : parent_       (parent),
      signalType_   (sigType),
      name_         (name),
      modulatorId_  (id),
      outputId_     (outputId),
      channel_      (0),
      param_        (p)
{
    MO_DEBUG_MOD("Modulator::Modulator(" << id << ", " << outputId << ", " << parent << ")");

    // XXX

    // extract channel number
    QRegExp exp("[0-9]+$");
    if (exp.indexIn(outputId) >= 0)
    {
        channel_ = outputId.mid(exp.pos(), exp.matchedLength()).toInt();
    }
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
    QString n = modulator() ? modulator()->name() : (modulatorId() + "(NULL)");
    if (!outputId_.isEmpty())
        n += ":" + outputId_;
    n += QString("(%1) -> ").arg(channel_);
    if (!parent())
        n += name() + "(NULL)";
    else
    {
        n += parent()->name();
        if (param_)
            n += ":" + param_->name();
    }
    return n;
}
/*
bool Modulator::isAudioToFloatConverter() const
{
    return modulator_ && modulator_->isAudioObject()
            // and connecting to an audio output (should be in outputId)
            ;
}*/
/*
uint Modulator::getAudioOutputChannel() const
{
    // skip the '_audio_' part and read channel
    return outputId().mid(7).toUInt();
}*/

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
