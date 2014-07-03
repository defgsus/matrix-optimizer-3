/** @file microphone.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_MICROPHONE_H
#define MOSRC_OBJECT_MICROPHONE_H

#include "object.h"

namespace MO {

class Microphone : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Microphone);

    virtual Type type() const { return T_MICROPHONE; }
    virtual bool isMicrophone() const { return true; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_MICROPHONE_H
