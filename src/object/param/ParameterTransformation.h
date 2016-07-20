/** @file parametertransformation.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#if 1

#ifndef MOSRC_OBJECT_PARAM_PARAMETERTRANSFORMATION_H
#define MOSRC_OBJECT_PARAM_PARAMETERTRANSFORMATION_H

#include "Parameter.h"
#include "types/vector.h"

namespace MO {

/** Currently only serves as input */
class ParameterTransformation : public Parameter
{
public:

    ParameterTransformation(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("transformation"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_TRANSFORMATION; }

    virtual void copyFrom(Parameter* other) Q_DECL_OVERRIDE;

    //virtual QString getDocType() const Q_DECL_OVERRIDE;

    //QString infoName() const { return QString("%1 (%2)").arg(name()).arg(value_); }

    QString baseValueString(bool ) const override { return "XXX"; }
    QString valueString(const RenderTime& , bool ) const override { return "XXX"; }

    // ---------------- getter -----------------

    const Mat4& defaultValue() const { return defaultValue_; }

    Mat4 value(const RenderTime& time) const;

    bool isBaseValueEqual(const Mat4& v) const { return value_ == v; }

    /* Writes @p number values starting at @p time into the pointer */
    //void getValues(Double time, uint thread, Double timeIncrement, uint number, Double * ptr) const;

    // ---------------- setter -----------------

    void setDefaultValue(const Mat4& m) { defaultValue_ = m; }

    void setValue(const Mat4& m) { value_ = m; }

    // --------- modulation -----------

    int getModulatorTypes() const Q_DECL_OVERRIDE;

    virtual Modulator * getModulator(const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE;

private:

    Mat4 defaultValue_,
         value_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_PARAMETERTRANSFORMATION_H

#endif // 0
