/** @file parameterselect.h

    @brief Parameter for named list of ints

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERSELECT_H
#define MOSRC_OBJECT_PARAM_PARAMETERSELECT_H

#include <QStringList>

#include "parameter.h"

namespace MO {

class ParameterSelect : public Parameter
{
public:

    ParameterSelect(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("select"); return s; }

    // ------------- getter ---------------

    int defaultValue() const { return defaultValue_; }

    //int value(Double time) const { return value_ }
    int baseValue() const { return value_; }

    const QList<int>& valueList() const { return valueList_; }
    const QStringList& valueNames() const { return valueNames_; }
    const QStringList& valueIds() const { return valueIds_; }
    const QStringList& statusTips() const { return statusTips_; }

    const QString& valueName() const;
    const QString& valueId() const;
    const QString& defaultValueName() const;

    // ------------ setter ----------------

    void setDefaultValue(int v);
    void setValue(int v);
    /** Sets the value to the value in the valueList at @p index */
    void setValueFromIndex(int index);

    void setValueList(const QList<int>& valueList) { valueList_ = valueList; }
    void setValueNames(const QStringList& names) { valueNames_ = names; }
    void setValueIds(const QStringList& ids) { valueIds_ = ids; }
    void setStatusTips(const QStringList& tips) { statusTips_ = tips; }

    // --------- modulation -----------

    void collectModulators() { }

    virtual QList<Object*> getModulatingObjects() const { return QList<Object*>(); }
    virtual QList<Object*> getFutureModulatingObjects(const Scene *) const { return QList<Object*>(); }

    /** Receives modulation value at time */
    //int getModulationValue(Double time) const;
    //const QList<TrackFloat*>& modulators() const { return modulators_; }

private:

    int defaultValue_,
        value_;

    QStringList
        valueNames_,
        valueIds_,
        statusTips_;
    QList<int> valueList_;
    //QList<TrackFloat*> modulators_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_PARAMETERSELECT_H
