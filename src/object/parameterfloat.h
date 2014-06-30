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
    explicit ParameterFloat(QObject *parent = 0);

    MO_OBJECT_CLONE(ParameterFloat)

    const QString& className() const { static QString s(MO_OBJECTCLASSNAME_PARAMETER_FLOAT); return s; }

    QString infoName() const { return QString("%1 (%2)").arg(name()).arg(value_); }

    virtual void serialize(IO::DataStream &) const;
    virtual void deserialize(IO::DataStream &);

    Double defaultValue() const { return defaultValue_; }
    Double minValue() const { return minValue_; }
    Double maxValue() const { return maxValue_; }
    Double value(Double time) const;

    void setDefaultValue(Double v) { defaultValue_ = v; }
    void setMinValue(Double v) { minValue_ = v; }
    void setMaxValue(Double v) { maxValue_ = v; }
    void setValue(Double v) { value_ = v; }

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
