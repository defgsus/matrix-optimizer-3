/** @file parameterfloat.h

    @brief Parameter of type float (Double)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERFLOAT_H
#define MOSRC_OBJECT_PARAM_PARAMETERFLOAT_H

#include "Parameter.h"
#include "types/time.h"
#include "math/Fraction.h"

namespace MO {

class ParameterFloat : public Parameter
{
public:

    /** Use to set unbounded limits */
    static Double infinity;

    ParameterFloat(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("float"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_FLOAT; }

    virtual void copyFrom(Parameter* other) Q_DECL_OVERRIDE;

    virtual QString getDocType() const Q_DECL_OVERRIDE;

    //QString infoName() const { return QString("%1 (%2)").arg(name()).arg(value_); }

    QString baseValueString(bool inShort) const override;
    QString valueString(const RenderTime& t, bool inShort) const override;

    // ---------------- getter -----------------

    Double defaultValue() const { return defaultValue_; }
    const MATH::Fraction& defaultValueFraction() const { return defFrac_; }
    Double minValue() const { return minValue_; }
    Double maxValue() const { return maxValue_; }
    Double smallStep() const { return smallStep_; }

    bool isMinLimit() const;
    bool isMaxLimit() const;
    bool isFractional() const { return isFractional_; }
    bool isDefaultFractional() const { return isDefaultFractional_; }

    bool isBaseValueEqual(Double v) const { return value_ == v; }
    bool isBaseValueEqual(const MATH::Fraction& v) const { return frac_ == v; }

    Double value(const RenderTime& time) const
        { return std::max(minValue_,std::min(maxValue_,
                                             value_ + getModulationValue(time) )); }

    Double baseValue() const { return value_; }
    const MATH::Fraction& baseValueFraction() const { return frac_; }

    /** Writes @p number values starting at @p time into the pointer */
    void getValues(const RenderTime& time, Double timeIncrement,
                   uint number, Double * ptr) const;

    /** Writes @p number values starting at @p time into the pointer */
    void getValues(const RenderTime& time, Double timeIncrement,
                   uint number, F32 * ptr) const;

    // ---------------- setter -----------------

    void setDefaultValue(Double v) { defaultValue_ = v; isDefaultFractional_ = false; }
    void setDefaultValue(const MATH::Fraction& v)
        { defFrac_ = v; defaultValue_ = defFrac_.value(); isDefaultFractional_ = true; }
    void setSmallStep(Double v) { smallStep_ = v; }
    void setMinValue(Double v) { minValue_ = v; }
    void setMaxValue(Double v) { maxValue_ = v; }
    void setRange(Double vmin, Double vmax) { minValue_ = vmin; maxValue_ = vmax; }

    void setValue(Double v) { value_ = v; isFractional_ = false; }
    void setValue(const MATH::Fraction& f)
        { frac_ = f; value_ = frac_.value(); isFractional_ = true; }

    void setNoMinValue() { minValue_ = infinity; }
    void setNoMaxValue() { maxValue_ = infinity; }
    void setUnlimited() { minValue_ = maxValue_ = infinity; }

    // --------- modulation -----------

    int getModulatorTypes() const Q_DECL_OVERRIDE;

    /** Receives modulation value at time */
    Double getModulationValue(const RenderTime& time) const;

    virtual Modulator * getModulator(
            const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE;

private:

    Double defaultValue_,
           minValue_,
           maxValue_,
           smallStep_,
           value_;
    MATH::Fraction frac_, defFrac_;
    bool isFractional_, isDefaultFractional_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERFLOAT_H
