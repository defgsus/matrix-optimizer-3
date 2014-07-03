/** @file scale.h

    @brief scale transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#ifndef MOSRC_OBJECT_SCALE_H
#define MOSRC_OBJECT_SCALE_H

#include "transformation.h"

namespace MO {


class Scale : public Transformation
{
    Q_OBJECT
public:
    explicit Scale(QObject *parent = 0);

    MO_OBJECT_CLONE(Scale)

    virtual const QString& className() const { static QString s(MO_OBJECTCLASSNAME_SCALE); return s; }

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    virtual void createParameters();

    virtual void applyTransformation(Mat4& matrix, Double time) const;

signals:

public slots:

protected:

    ParameterFloat * all_, * x_, * y_, * z_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SCALE_H
