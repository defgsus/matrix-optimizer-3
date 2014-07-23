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
    MO_OBJECT_CONSTRUCTOR(SoundSource);

    virtual Type type() const { return T_SOUNDSOURCE; }
    virtual bool isSoundSource() const { return true; }

    virtual void createParameters();

    virtual void createAudioSources();

    virtual void sampleStep(Double time, int thread);
signals:

public slots:

private:

    ParameterFloat * audioTrack_;

    AUDIO::AudioSource * audio_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SOUNDSOURCE_H
