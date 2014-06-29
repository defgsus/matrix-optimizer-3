/** @file transformation.h

    @brief object transformation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include "object.h"

namespace MO {


class Transformation : public Object
{
    Q_OBJECT
public:
    explicit Transformation(QObject *parent = 0);

    MO_OBJECT_CLONE(Transformation)

    virtual Type type() const { return T_TRANSFORMATION; }
    virtual bool isTransformation() const { return true; }

    virtual const QString& className() const { static QString s(MO_OBJECTCLASSNAME_TRANSFORMATION); return s; }

signals:

public slots:

};

} // namespace MO

#endif // TRANSFORMATION_H
