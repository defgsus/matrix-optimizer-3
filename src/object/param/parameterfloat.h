/** @file parameterfloat.h

    @brief Parameter of type float (Double)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERFLOAT_H
#define MOSRC_OBJECT_PARAM_PARAMETERFLOAT_H

#include "parameter.h"
#include "types/float.h"

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

    virtual QString getDocType() const Q_DECL_OVERRIDE;

    //QString infoName() const { return QString("%1 (%2)").arg(name()).arg(value_); }

    // ---------------- getter -----------------

    Double defaultValue() const { return defaultValue_; }
    Double minValue() const { return minValue_; }
    Double maxValue() const { return maxValue_; }
    Double smallStep() const { return smallStep_; }

    Double value(Double time, uint thread) const
        { return std::max(minValue_,std::min(maxValue_, value_ + getModulationValue(time, thread) )); }
    Double baseValue() const { return value_; }

    /** Writes @p number values starting at @p time into the pointer */
    void getValues(Double time, uint thread, Double timeIncrement, uint number, Double * ptr) const;

    /** Writes @p number values starting at @p time into the pointer */
    void getValues(Double time, uint thread, Double timeIncrement, uint number, F32 * ptr) const;

    /** Writes @p number values starting at @p pos into the pointer */
    void getValues(SamplePos pos, uint thread, Double sampleRateInv, uint number, F32 * ptr) const;

    // ---------------- setter -----------------

    void setDefaultValue(Double v) { defaultValue_ = v; }
    void setSmallStep(Double v) { smallStep_ = v; }
    void setMinValue(Double v) { minValue_ = v; }
    void setMaxValue(Double v) { maxValue_ = v; }
    void setRange(Double vmin, Double vmax) { minValue_ = vmin; maxValue_ = vmax; }

    void setValue(Double v) { value_ = v; }

    void setNoMinValue() { minValue_ = infinity; }
    void setNoMaxValue() { maxValue_ = infinity; }
    void setUnlimited() { minValue_ = maxValue_ = infinity; }

    // --------- modulation -----------

    int getModulatorTypes() const Q_DECL_OVERRIDE;

    /** Receives modulation value at time */
    Double getModulationValue(Double time, uint thread) const;

    virtual Modulator * getModulator(const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE;

private:

    Double defaultValue_,
           minValue_,
           maxValue_,
           smallStep_,
           value_;

};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERFLOAT_H
