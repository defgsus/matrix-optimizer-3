/** @file parameter.cpp

    @brief Abstract general purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include "parameter.h"
#include "scene.h"
#include "io/datastream.h"

namespace MO {

Parameter::Parameter(QObject *parent) :
    Object(parent)
{
}

void Parameter::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("par", 1);

    io << parameterId();

    // modulations
    io << modulatorIds_;
}

void Parameter::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("par", 1);

    QString id;
    io >> id;
    setParameterId(id);

    io >> modulatorIds_;
}


void Parameter::addModulator(const QString &idName)
{
    modulatorIds_.insert(idName);
    // hacky
    Scene * s = sceneObject();
    if (s) s->tellTreeChanged();
}

void Parameter::removeModulator(const QString &idName)
{
    modulatorIds_.remove(idName);
    // hacky
    Scene * s = sceneObject();
    if (s) s->tellTreeChanged();
}

} // namespace MO
