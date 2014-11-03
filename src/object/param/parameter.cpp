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
    isModulateable_(false),
    isVisible_  (true)
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

QString Parameter::infoName() const
{
    if (!object_)
        return name_;

    QString s = object_->name() + "." + name_;
    Object * o = object_;
    while (o && !(o->type() & Object::TG_REAL_OBJECT
                  || o->type() & Object::T_CLIP))
    {
        o = o->parentObject();

        if (o)
            s.prepend(o->name() + "/");
    }

    return s;
}

QString Parameter::infoIdName() const
{
    if (!object_)
        return idName_;

    QString s = object_->idName() + "." + idName_;
    Object * o = object_;
    while (o && !(o->type() & Object::TG_REAL_OBJECT
                  || o->type() & Object::T_CLIP))
    {
        o = o->parentObject();

        if (o)
            s.prepend(o->idName() + "/");
    }

    return s;
}

void Parameter::setVisible(bool visible)
{
    if (visible != isVisible_)
    {
        isVisible_ = visible;

        // notify scene/gui
        if (object())
        {
            Scene * scene = object()->sceneObject();
            if (scene)
            {
                scene->notifyParameterVisibility(this);
            }
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


void Parameter::collectModulators()
{
    if (modulators().isEmpty())
        return;

    MO_DEBUG_MOD("Parameter("<<idName()<<")::collectModulators()");

    Object * root = object()->rootObject();

    uint k = 0;
    for (auto m : modulators())
    {
        Object * o = root->findChildObject(m->modulatorId(), true);

        if (o)
        {
            if (m->canBeModulator(o))
            {
                m->setModulator(o);
                ++k;
            }
            else
            {
                m->setModulator(0);
                MO_WARNING("parameter '" << idName()
                           << "' can not work with modulator '" << m->modulatorId() << "'");
            }
        }
        else
        {
            m->setModulator(0);
            MO_WARNING("parameter '" << idName()
                       << "' could not find modulator '" << m->modulatorId() << "'");
        }
    }

    MO_DEBUG_MOD("Parameter("<<idName()<<") found " << k << " of "
                 << modulators().size() << " modulator(s)");
}


QList<Object*> Parameter::getFutureModulatingObjects(const Scene *scene) const
{
    QList<Object*> mods, list;

    auto ids = modulatorIds();

    for (const auto &id : ids)
    {
        if (Object * o = scene->findChildObject(id, true))
            mods.append(o);
    }

    list = mods;

    for (auto m : mods)
        list.append(m->getModulatingObjects());

    return list;
}


} // namespace MO
