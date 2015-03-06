/** @file sequences.h

    @brief Sequence container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#if 0

#ifndef MOSRC_OBJECT_SEQUENCES_H
#define MOSRC_OBJECT_SEQUENCES_H

#include "object.h"

namespace MO {

class Sequences : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Sequences);

    virtual Type type() const { return T_SEQUENCEGROUP; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCES_H

#endif
