/** @file parameterfloat.h

    @brief Parameter of type float (Double)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef PARAMETERFLOAT_H
#define PARAMETERFLOAT_H

#include "parameter.h"

namespace MO {

class ParameterFloat : public Parameter
{
    Q_OBJECT
public:

    /** Use to set unbounded limits */
    static Double infinity;

    explicit ParameterFloat(QObject *parent = 0);

    MO_OBJECT_CLONE(ParameterFloat)

    Type type() const { return T_PARAMETER_FLOAT; }
    const QString& className() const { static QString s(MO_OBJECTCLASSNAME_PARAMETER_FLOAT); return s; }

    QString infoName() const { return QString("%1 (%2)").arg(name()).arg(value_); }

    virtual void serialize(IO::DataStream &) const;
    virtual void deserialize(IO::DataStream &);

    Double defaultValue() const { return defaultValue_; }
    Double minValue() const { return minValue_; }
    Double maxValue() const { return maxValue_; }
    Double value(Double time) const;
    Double baseValue() const { return value_; }

    void setDefaultValue(Double v) { defaultValue_ = v; }
    void setMinValue(Double v) { minValue_ = v; }
    void setMaxValue(Double v) { maxValue_ = v; }
    void setValue(Double v) { value_ = v; }

    void setNoMinValue() { minValue_ = infinity; }
    void setNoMaxValue() { maxValue_ = infinity; }
    void setUnlimited() { minValue_ = maxValue_ = infinity; }

signals:

public slots:

private:

    Double defaultValue_,
           minValue_,
           maxValue_,
           value_;
};

} // namespace MO


#endif // PARAMETERFLOAT_H
