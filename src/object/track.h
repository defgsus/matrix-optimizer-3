/** @file track.h

    @brief Abstract Track object

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
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(Track);

    bool isTrack() const { return true; }

signals:

public slots:

private:

};

} // namespace MO

#endif // MOSRC_OBJECT_TRACK_H
