/** @file parameter.cpp

    @brief Abstract general purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/


#include "parameter.h"
#include "modulator.h"
#include "object/scene.h"
#include "types/properties.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

Parameter::Parameter(Object * object, const QString& id, const QString& name) :
    object_     (object),
    idName_     (id),
    name_       (name),
    isEditable_ (false),
    isModulateable_(false),
    isVisible_  (true),
    isVisibleGraph_(false),
    isVisibleInterface_(false),
    iProps_     (new Properties)
{
}

Parameter::~Parameter()
{
    clearModulators_();
    delete iProps_;
}

void Parameter::serialize(IO::DataStream &io) const
{
    io.writeHeader("par", 5);

    io << idName_;

    // v2
    io << (qint32)modulators_.size();
    for (auto m : modulators_)
    {
        io << m->modulatorId();
        // v4
        io << m->outputId();
        m->serialize(io);
    }

    // v3
    io << isVisibleGraph_;
    // v5
    io << isVisibleInterface_;
    iProps_->serialize(io);
}

void Parameter::deserialize(IO::DataStream &io)
{
    const int ver = io.readHeader("par", 5);

    io >> idName_;

    if (ver <= 1)
    {
        QStringList ids;
        io >> ids;
        for (auto &id : ids)
            addModulator(id, "");
    }

    if (ver >= 2)
    {
        qint32 num;
        io >> num;
        for (qint32 i=0; i<num; ++i)
        {
            QString id, outputId;
            io >> id;
            if (ver >= 4)
                io >> outputId;
            Modulator * m = getModulator(id, outputId);
            m->deserialize(io);
        }
    }

    if (ver >= 3)
        io >> isVisibleGraph_;
    else
        isVisibleGraph_ = false;

    if (ver >= 5)
    {
        io >> isVisibleInterface_;
        iProps_->deserialize(io);
    }
    else
        isVisibleInterface_ = false;
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
            if (Scene * scene = object()->sceneObject())
                scene->notifyParameterVisibility(this);
    }
}

void Parameter::setInterfaceProperties(const Properties &p)
{
    *iProps_ = p;
}

void Parameter::idNamesChanged(const QMap<QString, QString> & map)
{
    // adjust modulator ids
    for (Modulator * m : modulators_)
    {
        auto i = map.find(m->modulatorId());
        if (i != map.end())
            m->setModulatorId(i.value());
    }
}


QList<QPair<QString, QString>> Parameter::modulatorIds() const
{
    QList<QPair<QString, QString>> list;
    for (auto m : modulators_)
        list << QPair<QString, QString>(m->modulatorId(), m->outputId());
    return list;
}

Modulator * Parameter::addModulator(const QString &idName, const QString &outputId)
{
    MO_DEBUG_MOD("Parameter("<<this->idName()<<")::addModulator(" << idName << ", " << outputId << ")");

    if (Modulator * m = findModulator(idName, outputId))
    {
        MO_WARNING("trying to add duplicate parameter modulator '" << idName << "'");
        return m;
    }

    return getModulator(idName, outputId);
}

void Parameter::removeModulator(const QString &idName, const QString& outputId)
{
    MO_DEBUG_MOD("Parameter("<<this->idName()<<")::removeModulator(" << idName << ", " << outputId << ")");

    if (!findModulator(idName, outputId))
    {
        MO_WARNING("trying to remove unknown parameter modulator '" << idName << "'");
        return;
    }

    for (auto m : modulators_)
    {
        if (m->modulatorId() == idName
         && m->outputId() == outputId)
        {
            modulators_.removeOne(m);
            delete m;
            break;
        }
    }
}

void Parameter::removeAllModulators(const QString &idName)
{
    MO_DEBUG_MOD("Parameter("<<this->idName()<<")::removeAllModulators(" << idName << ")");

    QList<Modulator*> mods;

    for (auto m : modulators_)
    {
        if (m->modulatorId() == idName)
            delete m;
        else
            mods << m;
    }

    if (mods.size() != modulators_.size())
        std::swap(modulators_, mods);
}

void Parameter::removeAllModulators()
{
    clearModulators_();
}

void Parameter::addModulator_(Modulator * m)
{
    MO_ASSERT(!findModulator(m->modulatorId(), m->outputId()), "duplicate modulator added");
    modulators_.append(m);
}

void Parameter::clearModulators_()
{
    for (auto m : modulators_)
        delete m;

    modulators_.clear();
}

Modulator * Parameter::findModulator(const QString& id, const QString& conId) const
{
    for (auto m : modulators_)
        if (m->modulatorId() == id
            && m->outputId() == conId)
            return m;

    return 0;
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
        if (Object * o = scene->findChildObject(id.first, true))
            mods.append(o);
    }

    list = mods;

    for (auto m : mods)
        list.append(m->getModulatingObjects());

    return list;
}


} // namespace MO
