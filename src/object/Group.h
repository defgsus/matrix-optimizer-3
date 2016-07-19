/** @file group.h

    @brief Group for all kinds of objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2014</p>
*/

#ifndef MOSRC_OBJECT_GROUP_H
#define MOSRC_OBJECT_GROUP_H

#include "Object.h"

namespace MO {

class Group : public Object
{
public:
    MO_OBJECT_CONSTRUCTOR(Group);

    virtual Type type() const { return T_GROUP; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_GROUP_H
