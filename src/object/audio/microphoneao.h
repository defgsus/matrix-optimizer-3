/** @file microphoneao.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 14.06.2015</p>
*/

#ifndef MO_DISABLE_SPATIAL

#ifndef MOSRC_OBJECT_AUDIO_MICROPHONEAO_H
#define MOSRC_OBJECT_AUDIO_MICROPHONEAO_H

#include "object/audioobject.h"

namespace MO {

class MicrophoneAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(MicrophoneAO)
    ~MicrophoneAO();

    // enable calculation of transformations
    virtual Type type() const Q_DECL_OVERRIDE { return T_SOUND_OBJECT; }

    virtual void createParameters() Q_DECL_OVERRIDE;

protected:

    virtual void processMicrophoneBuffers(
            const QList<AUDIO::SpatialMicrophone*>& microphones, const RenderTime& time) Q_DECL_OVERRIDE;

    virtual void processAudio(const RenderTime& ) Q_DECL_OVERRIDE { }

private:

    AUDIO::AudioBuffer * mbuf_;

    ParameterFloat * p_amp_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_MICROPHONEAO_H

#endif // #ifndef MO_DISABLE_SPATIAL
