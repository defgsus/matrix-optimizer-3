/** @file sequences.h

    @brief Sequence container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_OBJECT_SEQUENCES_H
#define MOSRC_OBJECT_SEQUENCES_H

#include "object.h"

namespace MO {

class Sequences : public Object
{
    Q_OBJECT
public:
    explicit Sequences(QObject *parent = 0);

    MO_OBJECT_CLONE(Sequences)

    const QString& className() const { static QString s(MO_OBJECTCLASSNAME_SEQUENCEGROUP); return s; }

    virtual Type type() const { return T_SEQUENCEGROUP; }

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCES_H
