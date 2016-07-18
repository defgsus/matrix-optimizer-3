/** @file soundsourceao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#ifndef MO_DISABLE_SPATIAL

#ifndef MOSRC_OBJECT_AUDIO_SOUNDSOURCEAO_H
#define MOSRC_OBJECT_AUDIO_SOUNDSOURCEAO_H

#include "object/audioobject.h"

namespace MO {

class SoundSourceAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(SoundSourceAO)

    virtual Type type() const Q_DECL_OVERRIDE { return T_SOUND_OBJECT; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter*) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void calculateSoundSourceBuffer(const QList<AUDIO::SpatialSoundSource*>,
                                            const RenderTime& time)
                                            Q_DECL_OVERRIDE;
protected:

    virtual void processAudio(const RenderTime& ) Q_DECL_OVERRIDE { }
private:

    ParameterFloat
        * paramAmp_;
    ParameterSelect
        * paramHasDist_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_SOUNDSOURCEAO_H

#endif // #ifndef MO_DISABLE_SPATIAL
