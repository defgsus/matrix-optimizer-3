/** @file parameter.h

    @brief Abstract general purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

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
    explicit Parameter(QObject *parent = 0);

    virtual bool isParameter() const { return true; }

    virtual void serialize(IO::DataStream &) const;
    virtual void deserialize(IO::DataStream &);

    const QString& parameterId() const { return parameterId_; }
    void setParameterId(const QString& id) { parameterId_ = id; }

signals:

public slots:

private:

    QString parameterId_;

};

} // namespace MO


#endif // MOSRC_OBJECT_PARAMETER_H
