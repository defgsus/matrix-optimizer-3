/** @file sequencefloat.h

    @brief Float sequence

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_OBJECT_SEQUENCEFLOAT_H
#define MOSRC_OBJECT_SEQUENCEFLOAT_H


#include "sequence.h"

namespace MO {

class SequenceFloat : public Sequence
{
    Q_OBJECT
public:
    explicit SequenceFloat(QObject *parent = 0);

    MO_OBJECT_CLONE(SequenceFloat)

    const QString& className() const { static QString s(MO_OBJECTCLASSNAME_SEQUENCE_FLOAT); return s; }

    virtual Type type() const { return T_SEQUENCE_FLOAT; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCEFLOAT_H
