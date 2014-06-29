/** @file dummy.h

    @brief Dummy object for skipping unknown objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_DUMMY_H
#define MOSRC_OBJECT_DUMMY_H

#include "object.h"

namespace MO {

class Dummy : public Object
{
    Q_OBJECT
public:
    explicit Dummy(QObject *parent = 0);

    const QString& className() const { static QString s("Dummy"); return s; }

    bool isValid() const { return false; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_DUMMY_H
