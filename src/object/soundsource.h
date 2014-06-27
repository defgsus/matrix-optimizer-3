/** @file soundsource.h

    @brief Basic sound Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

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

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_SOUNDSOURCE_H
