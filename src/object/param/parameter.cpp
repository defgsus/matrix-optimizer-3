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
#include "modulator.h"

namespace MO {

Parameter::Parameter(Object * object, const QString& id, const QString& name) :
    object_     (object),
    idName_     (id),
    name_       (name),
    isEditable_ (false),
    isModulateable_(false)
{
}

Parameter::~Parameter()
{
    clearModulators_();
}

void Parameter::serialize(IO::DataStream &io) const
{
    io.writeHeader("par", 2);

    io << idName_;

    // v2
    io << (qint32)modulators_.size();
    for (auto m : modulators_)
    {
        io << m->modulatorId();
        m->serialize(io);
    }
}

void Parameter::deserialize(IO::DataStream &io)
{
    int ver = io.readHeader("par", 2);

    io >> idName_;

    if (ver <= 1)
    {
        QStringList ids;
        io >> ids;
        for (auto &id : ids)
            addModulator(id);
    }

    if (ver >= 2)
    {
        qint32 num;
        io >> num;
        for (qint32 i=0; i<num; ++i)
        {
            QString id;
            io >> id;
            Modulator * m = getModulator(id);
            m->deserialize(io);
        }
    }
}

QStringList Parameter::modulatorIds() const
{
    QStringList list;
    for (auto m : modulators_)
        list << m->modulatorId();
    return list;
}

Modulator * Parameter::addModulator(const QString &idName)
{
    MO_DEBUG_MOD("Parameter("<<this->idName()<<")::addModulator(" << idName << ")");

    if (Modulator * m = findModulator(idName))
    {
        MO_WARNING("trying to add duplicate parameter modulator '" << idName << "'");
        return m;
    }

    return getModulator(idName);
}

void Parameter::removeModulator(const QString &idName)
{
    MO_DEBUG_MOD("Parameter("<<this->idName()<<")::removeModulator(" << idName << ")");

    if (!findModulator(idName))
    {
        MO_WARNING("trying to remove unknown parameter modulator '" << idName << "'");
        return;
    }

    for (auto m : modulators_)
    {
        if (m->modulatorId() == idName)
        {
            modulators_.removeOne(m);
            delete m;
            break;
        }
    }
}

void Parameter::removeAllModulators()
{
    clearModulators_();
}

void Parameter::addModulator_(Modulator * m)
{
    MO_ASSERT(!findModulator(m->modulatorId()), "duplicate modulator added");
    modulators_.append(m);
}

void Parameter::clearModulators_()
{
    for (auto m : modulators_)
        delete m;

    modulators_.clear();
}

Modulator * Parameter::findModulator(const QString& id) const
{
    for (auto m : modulators_)
        if (m->modulatorId() == id)
            return m;

    return 0;
}

QList<Object*> Parameter::getModulatingObjects() const
{
    QList<Object*> list;

    for (auto m : modulators_)
        if (m->modulator())
            list.append(m->modulator());

    for (auto m : modulators_)
        if (m->modulator())
            list.append(m->modulator()->getModulatingObjects());

    return list;
}

} // namespace MO
