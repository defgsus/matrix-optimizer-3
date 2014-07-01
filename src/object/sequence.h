/** @file sequence.h

    @brief Sequence class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_OBJECT_SEQUENCE_H
#define MOSRC_OBJECT_SEQUENCE_H

#include "object.h"

namespace MO {

class Sequence : public Object
{
    Q_OBJECT
public:
    explicit Sequence(QObject *parent = 0);

    MO_OBJECT_CLONE(Sequence)

    const QString& className() const { static QString s(MO_OBJECTCLASSNAME_SEQUENCE); return s; }

    bool isSequence() const { return true; }
    virtual Type type() const { return T_SEQUENCE; }

signals:

public slots:

};

} // namespace MO


#endif // MOSRC_OBJECT_SEQUENCE_H
