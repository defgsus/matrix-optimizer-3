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
    io.writeHeader("param", 1);

    io << parameterId();

    // modulations
    io << modulatorIds_;
}

void Parameter::deserialize(IO::DataStream &io)
{
    io.readHeader("param", 1);

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
    if (s) s->treeChanged();
}

void Parameter::removeModulator(const QString &idName)
{
    modulatorIds_.remove(idName);
    // hacky
    Scene * s = sceneObject();
    if (s) s->treeChanged();
}

} // namespace MO
