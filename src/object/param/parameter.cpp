/** @file parameter.cpp

    @brief Abstract general purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include "parameter.h"
#include "object/scene.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

Parameter::Parameter(Object * object, const QString& id, const QString& name) :
    object_     (object),
    idName_     (id),
    name_       (name),
    isEditable_ (true)
{
}

void Parameter::serialize(IO::DataStream &io) const
{
    io.writeHeader("par", 1);

    io << idName_ << name_;

    // modulations
    io << modulatorIds_;
}

void Parameter::deserialize(IO::DataStream &io)
{
    io.readHeader("par", 1);

    io >> idName_ >> name_;

    io >> modulatorIds_;
}


void Parameter::addModulator(const QString &idName)
{
    MO_DEBUG_MOD("Parameter("<<this->idName()<<")::addModulator(" << idName << ")");

    if (modulatorIds_.contains(idName))
    {
        MO_WARNING("trying to add duplicate parameter modulator '" << idName << "'");
        return;
    }
    modulatorIds_.append(idName);
}

void Parameter::removeModulator(const QString &idName)
{
    MO_DEBUG_MOD("Parameter("<<this->idName()<<")::removeModulator(" << idName << ")");

    if (!modulatorIds_.contains(idName))
    {
        MO_WARNING("trying to remove unknown parameter modulator '" << idName << "'");
        return;
    }
    modulatorIds_.removeOne(idName);
}

} // namespace MO
