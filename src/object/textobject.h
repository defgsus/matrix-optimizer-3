/** @file textobject.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTOBJECT_H
#define MOSRC_OBJECT_TEXTOBJECT_H

#include "object.h"

namespace MO {

class TextObject : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(TextObject);

    virtual Type type() const Q_DECL_OVERRIDE { return T_TEXT; }
    virtual bool isText() const Q_DECL_OVERRIDE { return true; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTOBJECT_H
