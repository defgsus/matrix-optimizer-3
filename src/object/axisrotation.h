/** @file axisrotation.h

    @brief Rotation about arbitrary axis

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef MOSRC_OBJECT_AXISROTATION_H
#define MOSRC_OBJECT_AXISROTATION_H


#include "transformation.h"

namespace MO {


class AxisRotation : public Transformation
{
    Q_OBJECT
public:
    explicit AxisRotation(QObject *parent = 0);

    MO_OBJECT_CLONE(AxisRotation)

    virtual const QString& className() const { static QString s(MO_OBJECTCLASSNAME_AXISROTATION); return s; }

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    virtual void createParameters();

    virtual void applyTransformation(Mat4& matrix, Double time) const;

signals:

public slots:

protected:

    ParameterFloat * angle_, * x_, * y_, * z_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AXISROTATION_H
