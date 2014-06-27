/** @file soundsource.h

    @brief Basic sound Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_SOUNDSOURCE_H
#define MOSRC_OBJECT_SOUNDSOURCE_H

#include "object3d.h"

namespace MO {

class SoundSource : public Object3d
{
    Q_OBJECT
public:
    explicit SoundSource(QObject *parent = 0);

    virtual bool isSoundSource() const { return true; }

    virtual const QString& className() const { static QString s("SoundSource"); return s; }


signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_SOUNDSOURCE_H
