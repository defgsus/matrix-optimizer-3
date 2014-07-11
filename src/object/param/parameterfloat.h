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

    //QString infoName() const { return QString("%1 (%2)").arg(name()).arg(value_); }

    Double defaultValue() const { return defaultValue_; }
    Double minValue() const { return minValue_; }
    Double maxValue() const { return maxValue_; }
    Double smallStep() const { return smallStep_; }

    Double value(Double time) const { return value_ + getModulationValue(time); }
    Double baseValue() const { return value_; }

    void setDefaultValue(Double v) { defaultValue_ = v; }
    void setMinValue(Double v) { minValue_ = v; }
    void setMaxValue(Double v) { maxValue_ = v; }
    void setSmallStep(Double v) { smallStep_ = v; }

    void setValue(Double v) { value_ = v; }

    void setNoMinValue() { minValue_ = infinity; }
    void setNoMaxValue() { maxValue_ = infinity; }
    void setUnlimited() { minValue_ = maxValue_ = infinity; }

    // --------- modulation -----------

    /** Receives modulation value at time */
    Double getModulationValue(Double time) const;

    void collectModulators();

    const QList<TrackFloat*>& modulators() const { return modulators_; }

    virtual QList<Object*> getModulatingObjects() const;
    virtual QList<Object*> getFutureModulatingObjects(const Scene * scene) const;

private:

    Double defaultValue_,
           minValue_,
           maxValue_,
           smallStep_,
           value_;

    QList<TrackFloat*> modulators_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERFLOAT_H
