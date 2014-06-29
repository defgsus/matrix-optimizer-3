/** @file microphone.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_MICROPHONE_H
#define MOSRC_OBJECT_MICROPHONE_H

#include "object3d.h"

namespace MO {

class Microphone : public Object
{
    Q_OBJECT
public:
    explicit Microphone(QObject *parent = 0);

    virtual const QString& className() const { static QString s("Microphone"); return s; }

    virtual bool isMicrophone() const { return true; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_MICROPHONE_H
