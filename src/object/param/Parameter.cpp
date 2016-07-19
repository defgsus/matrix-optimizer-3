/** @file parameter.cpp

    @brief Abstract general purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include <QTextStream>

#include "Parameter.h"
#include "Modulator.h"
#include "object/Scene.h"
#include "object/util/ObjectConnectionGraph.h"
#include "types/Properties.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log_mod.h"
#include "io/log_param.h"

namespace MO {

Parameter::Parameter(Object * object, const QString& id, const QString& name)
    : object_           (object)
    , idName_           (id)
    , name_             (name)
    , isEditable_       (false)
    , isModulateable_   (false)
    , isVisible_        (true)
    , isVisibleGraph_   (false)
    , isVisibleInterface_(false)
    , isZombie_         (false)
    , isEvolve_         (true)
    , p_specFlag_       (SF_NONE)
    //iProps_     (new Properties)
{
}

Parameter::~Parameter()
{
    clearModulators_();
    //delete iProps_;
}

const QString& Parameter::displayName() const
{
    return userName().isEmpty() ? name() : userName();
}

void Parameter::serialize(IO::DataStream &io) const
{
    io.writeHeader("par", 6);

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
    // v5 (removed)
//    io << isVisibleInterface_;
//    iProps_->serialize(io);
    // v6
    io << userName_;
}

void Parameter::deserialize(IO::DataStream &io)
{
    const int ver = io.readHeader("par", 6);

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

    if (ver == 5) // dummy
    {
        io >> isVisibleInterface_;
        Properties tmp;
        tmp.deserialize(io);
        //iProps_->deserialize(io);
    }
    else
        isVisibleInterface_ = false;

    if (ver >= 6)
        io >> userName_;
    else
        userName_.clear();
}

void Parameter::copyFrom(Parameter* other)
{
    userName_ = other->userName();
    isVisibleGraph_ = other->isVisibleGraph_;
}

QString Parameter::infoName() const
{
    QString s = userName_.isEmpty() ? name_ : userName_;
    if (!groupName().isEmpty())
        s.prepend(QString("(%1)").arg(groupName()));

    if (!object_)
        return s;

    s.prepend(object_->name() + ".");
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


QString Parameter::getDoc() const
{
    QString str;
    QTextStream html(&str);
    html << "<b>" << name() << "</b> (<i>" << getDocType();
    if (isModulateable())
        html << ", " << Object::tr("modulateable");
    html << "</i>)\n"
         << "<ul>"; // inset paragraph

        html << "<p>" << getDocDesc() << "</p>";

        QString vals = getDocValues();
        if (!vals.isEmpty())
            html << "<p>" << vals << "</p>";

    html << "</ul><br/>";

    return str;
}

QString Parameter::getDocType() const { return typeName(); }
QString Parameter::getDocValues() const { return QString(); }
QString Parameter::getDocDesc() const { return statusTip(); }

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



bool Parameter::setVisible(bool visible)
{
    if (visible != isVisible_)
    {
        isVisible_ = visible;

        // notify scene/gui
        if (object())
            if (Scene * scene = object()->sceneObject())
                scene->notifyParameterVisibility(this);
    }
    return isVisible_;
}

//void Parameter::setInterfaceProperties(const Properties &p)
//{
//    *iProps_ = p;
//}

void Parameter::addSynonymId(const QString &id)
{
    synonymIds_.insert(id);
}

bool Parameter::hasSynonymId(const QString &id) const
{
    return synonymIds_.contains(id);
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

QList<QString> Parameter::getModulatorIds() const
{
    QList<QString> list;
    for (auto m : modulators_)
        list << m->modulatorId();
    return list;
}

Modulator * Parameter::addModulator(const QString &idName, const QString &outputId)
{
    MO_DEBUG_MOD("Parameter(" << this->idName()
                 << ")::addModulator(" << idName << ", " << outputId << ")");

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

void Parameter::removeAllModulators(const QList<QString>& idNames)
{
    MO_DEBUG_MOD("Parameter("<<this->idName()<<")::removeAllModulators(" << idNames.size() << ")");

    QList<Modulator*> mods;

    for (auto m : modulators_)
    {
        if (idNames.contains(m->modulatorId()))
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

void Parameter::getModulatingObjects(ObjectConnectionGraph& graph, bool recursive) const
{
    for (auto m : modulators_)
    if (m->modulator() && !graph.hasObject(m->modulator()))
    {
        graph.addConnection(m->modulator(), object());

        if (recursive)
            m->modulator()->getModulatingObjects(graph, true);
    }
}


void Parameter::collectModulators()
{
    if (modulators().isEmpty())
        return;

    MO_DEBUG_MOD("Parameter("<<idName()<<")::collectModulators()");

    Object * root = object()->rootObject();

    bool error = false;
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
                error = true;
                MO_WARNING("parameter '" << idName()
                           << "' can not work with modulator '" << m->modulatorId() << "'");
            }
        }
        else
        {
            m->setModulator(0);
            error = true;
            MO_WARNING("parameter '" << idName()
                       << "' could not find modulator '" << m->modulatorId() << "'");
        }
    }

    MO_DEBUG_MOD("Parameter("<<idName()<<") found " << k << " of "
                 << modulators().size() << " modulator(s)");

    if (error)
        clearNullModulators();
}

void Parameter::clearNullModulators()
{
    QList<Modulator*> tmp;
    for (Modulator * m : modulators())
    {
        if (m->modulator())
            tmp << m;
        else
        {
            MO_DEBUG("removing modulator '" << m->nameAutomatic());
            delete m;
        }
    }

    modulators_.swap(tmp);
}

void Parameter::getFutureModulatingObjects(
        ObjectConnectionGraph& graph, const Scene *scene, bool recursive) const
{
    // get ids
    auto ids = modulatorIds();
    // and pull objects from given scene
    for (const auto &id : ids)
    {
        if (Object * o = scene->findChildObject(id.first, true))
        if (!graph.hasObject(o))
        {
            graph.addConnection(o, object());
            if (recursive)
                o->getModulatingObjects(graph, true);
        }
    }
}

QList<Object*> Parameter::getModulatingObjectsList(bool recursive) const
{
    ObjectConnectionGraph graph;
    getModulatingObjects(graph, recursive);
    auto list = graph.makeLinear();
    list.removeOne(object());
    return list;
}

QList<Object*> Parameter::getFutureModulatingObjectsList(
        const Scene *scene, bool recursive) const
{
    ObjectConnectionGraph graph;
    getFutureModulatingObjects(graph, scene, recursive);
    auto list = graph.makeLinear();
    list.removeOne(object());
    return list;
}

} // namespace MO
