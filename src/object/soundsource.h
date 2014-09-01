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

    virtual Type type() const Q_DECL_OVERRIDE { return T_SOUNDSOURCE; }
    virtual bool isSoundSource() const Q_DECL_OVERRIDE { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void createAudioSources() Q_DECL_OVERRIDE;

    virtual void updateAudioTransformations(Double time, uint thread) Q_DECL_OVERRIDE;
    virtual void updateAudioTransformations(Double time, uint blockSize, uint thread)
                                                                    Q_DECL_OVERRIDE;

    virtual void performAudioBlock(SamplePos pos, uint thread) Q_DECL_OVERRIDE;
signals:

public slots:

private:

    ParameterFloat * audioTrack_;

    AUDIO::AudioSource * audio_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SOUNDSOURCE_H
