/** @file track.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#ifndef MOSRC_OBJECT_TRACK_H
#define MOSRC_OBJECT_TRACK_H

#include "object.h"

namespace MO {

class Track : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Track);

    virtual Type type() const { return T_TRACK; }



signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_TRACK_H
