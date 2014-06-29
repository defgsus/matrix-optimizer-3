/** @file soundsource.h

    @brief Basic sound Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_SOUNDSOURCE_H
#define MOSRC_OBJECT_SOUNDSOURCE_H

#include "object.h"

namespace MO {

class SoundSource : public Object
{
    Q_OBJECT
public:
    explicit SoundSource(QObject *parent = 0);

    MO_OBJECT_CLONE(SoundSource)

    virtual Type type() const { return T_SOUNDSOURCE; }
    virtual bool isSoundSource() const { return true; }

    virtual const QString& className() const { static QString s(MO_OBJECTCLASSNAME_SOUNDSOURCE); return s; }


signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_SOUNDSOURCE_H
