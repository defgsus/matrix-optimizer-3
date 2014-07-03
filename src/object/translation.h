/** @file translation.h

    @brief object translation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSLATION_H
#define MOSRC_OBJECT_TRANSLATION_H

#include "transformation.h"

namespace MO {


class Translation : public Transformation
{
    Q_OBJECT
public:
    explicit Translation(QObject *parent = 0);

    MO_OBJECT_CLONE(Translation)

    virtual const QString& className() const { static QString s(MO_OBJECTCLASSNAME_TRANSLATION); return s; }

    virtual void createParameters();

    virtual void applyTransformation(Mat4& matrix, Double time) const;

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

signals:

public slots:

protected:

    ParameterFloat * x_, * y_, * z_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSLATION_H
