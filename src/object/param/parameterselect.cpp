/** @file parameterselect.cpp

    @brief Parameter for list of ints

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2014</p>
*/

#include <QTextStream>

#include "parameterselect.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/scene.h"


// make ParameterSelect useable in QMetaObject::invokeMethod
Q_DECLARE_METATYPE(MO::ParameterSelect*);
namespace { static int register_param = qMetaTypeId<MO::ParameterSelect*>(); }


namespace MO {

ParameterSelect::ParameterSelect(Object * object, const QString& id, const QString& name)
    :   Parameter(object, id, name),
        defaultValue_   (0),
        value_          (0),
        isBoolean_      (false)
{
}


void ParameterSelect::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("pars", 1);

    io << valueId();

}

void ParameterSelect::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("pars", 1);

    io.readEnum(value_, defaultValue_, valueIds_, valueList_);
}

void ParameterSelect::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterSelect*>(other);
    if (!p)
        return;
    setDefaultValue(p->defaultValue());
    setValue(p->baseValue());
}

QString ParameterSelect::getDocType() const
{
    QString str = isBoolean() ? "bool" : "select";
    str += ", " + QObject::tr("default") + ": " + defaultValueName();
    return str;
}

QString ParameterSelect::getDocValues() const
{
    QString str;
    QTextStream html(&str);
    html << "<b>" + QObject::tr("values") << ":</b><ul>";
    for (int i=0; i<valueNames().size(); ++i)
    {
        html << "<li>"
             << "<i>" << valueNames().at(i) << "</i>: " << statusTips().at(i)
             << "</li>";
    }
    html << "</ul>";

    return str;
}




void ParameterSelect::setDefaultValue(int v)
{
    MO_ASSERT(valueList_.contains(v),
              "ParameterSelect('" << idName() << "')::setDefaultValue("
              << v << ") value not known");
    defaultValue_ = v;
}

void ParameterSelect::setValue(int v)
{
    MO_ASSERT(valueList_.contains(v),
              "ParameterSelect('" << idName() << "')::setValue("
              << v << ") value not known");
    value_ = v;
}

void ParameterSelect::setValueFromIndex(int index)
{
    MO_ASSERT(index >= 0 && index < valueList_.size(),
              "ParameterSelect('" << idName() << "')::setValueFromIndex("
              << index << ") out of range");
    value_ = valueList_.at(index);
}

const QString& ParameterSelect::valueName() const
{
    int idx = valueList_.indexOf(value_);
    MO_ASSERT(idx>=0, "unknown value " << value_ << " in ParameterSelect('" << idName() << "')");
    return valueNames_.at(idx);
}

const QString& ParameterSelect::valueId() const
{
    int idx = valueList_.indexOf(value_);
    MO_ASSERT(idx>=0 && idx<valueIds_.size(), "unknown value " << value_ << " in ParameterSelect('" << idName() << "')");
    return valueIds_.at(idx);
}

const QString& ParameterSelect::defaultValueName() const
{
    int idx = valueList_.indexOf(defaultValue_);
    MO_ASSERT(idx>=0, "unknown defaultvalue " << defaultValue_
              << " in ParameterSelect('" << idName() << "')");
    return valueNames_.at(idx);
}

void ParameterSelect::setBoolean(bool enable)
{
    if (enable)
        MO_ASSERT(valueList_.size() == 2, "ParameterSelect('" << idName() << "')::setBoolean() with "
                  << valueList_.size() << " values");
    isBoolean_ = enable;
}

void ParameterSelect::removeByValue(int value)
{
    if (value == defaultValue_)
    {
        MO_WARNING("ParameterSelect('" << idName() << "')::removeByValue("
                   << value << ") can't remove default value");
        return;
    }

    const int idx = valueList_.indexOf(value);
    if (idx < 0)
    {
        MO_WARNING("ParameterSelect('" << idName() << "')::removeByValue("
                   << value << ") unknown value");
        return;
    }

    removeById(valueIds_[idx]);
}

void ParameterSelect::removeById(const QString &id)
{
    const int idx = valueIds_.indexOf(id);
    if (idx < 0)
    {
        MO_WARNING("ParameterSelect::removeById('" << id << "') unknown id");
        return;
    }

    if (valueList_[idx] == defaultValue_)
    {
        MO_WARNING("ParameterSelect::removeById('" << id << "') can't remove default value");
        return;
    }

    valueIds_.removeAt(idx);
    valueNames_.removeAt(idx);
    statusTips_.removeAt(idx);
    valueList_.removeAt(idx);
}

/*
Double ParameterSelect::getModulationValue(Double time) const
{
    Double m = 0;

    for (auto t : modulators_)
        m += t->value(time);

    return m;
}

void ParameterSelect::collectModulators()
{
    modulators_.clear();
    if (modulatorIds().isEmpty())
        return;

    MO_DEBUG_MOD("ParameterSelect("<<idName()<<")::collectModulators()");


    Object * root = object()->rootObject();

    for (auto const &id : modulatorIds())
    {
        Object * o = root->findChildObject(id, true);
        if (auto s = qobject_cast<TrackFloat*>(o))
            modulators_.append(s);
        else
            MO_WARNING("parameter '" << idName()
                       << "' could not find modulator '" << id << "'");
    }

    MO_DEBUG_MOD("ParameterSelect("<<idName()<<") found " << modulators_.size() << " modulator(s)");
}

QList<Object*> ParameterSelect::getModulatingObjects() const
{
    QList<Object*> list;

    for (auto m : modulators_)
        list.append(m);

    for (auto m : modulators_)
        list.append(m->getModulatingObjects());

    return list;
}

QList<Object*> ParameterSelect::getFutureModulatingObjects(const Scene *scene) const
{
    QList<Object*> mods, list;

    for (const auto &m : modulatorIds())
    {
        if (Object * o = scene->findChildObject(m, true))
            mods.append(o);
    }

    list = mods;

    for (auto m : mods)
        list.append(m->getModulatingObjects());

    return list;
}
*/

} // namespace MO
