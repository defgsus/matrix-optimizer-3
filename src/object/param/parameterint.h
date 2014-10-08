/** @file parameterint.h

    @brief Parameter of type Int

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/11/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERINT_H
#define MOSRC_OBJECT_PARAM_PARAMETERINT_H

#include "parameter.h"
#include "types/int.h"
#include "types/float.h"

namespace MO {

class ParameterInt : public Parameter
{
public:

    /** Use to set unbounded limits */
    static Int infinity;

    ParameterInt(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("int"); return s; }

    // ---------------- getter -----------------

    Int defaultValue() const { return defaultValue_; }
    Int minValue() const { return minValue_; }
    Int maxValue() const { return maxValue_; }
    Int smallStep() const { return smallStep_; }

    Int value(Double time, uint thread) const
        { return std::max(minValue_,std::min(maxValue_, value_ + getModulationValue(time, thread) )); }
    Int baseValue() const { return value_; }

    // ---------------- setter -----------------

    void setDefaultValue(Int v) { defaultValue_ = v; }
    void setSmallStep(Int v) { smallStep_ = v; }
    void setMinValue(Int v) { minValue_ = v; }
    void setMaxValue(Int v) { maxValue_ = v; }
    void setRange(Int vmin, Int vmax) { minValue_ = vmin; maxValue_ = vmax; }

    void setValue(Int v) { value_ = v; }

    void setNoMinValue() { minValue_ = infinity; }
    void setNoMaxValue() { maxValue_ = infinity; }
    void setUnlimited() { minValue_ = maxValue_ = infinity; }

    // --------- modulation -----------

    /** Receives modulation value at time */
    Int getModulationValue(Double time, uint thread) const;

    virtual Modulator * getModulator(const QString &modulatorId) Q_DECL_OVERRIDE;

private:

    Int defaultValue_,
        minValue_,
        maxValue_,
        smallStep_,
        value_;

};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERINT_H
