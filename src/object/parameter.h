/** @file parameter.h

    @brief General purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAMETER_H
#define MOSRC_OBJECT_PARAMETER_H

#include "object.h"

namespace MO {


class Parameter : public Object
{
    Q_OBJECT
public:
    explicit Parameter(const QString& idName, QObject *parent = 0);

    virtual const QString& className() const { static QString s("Parameter"); return s; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_PARAMETER_H
