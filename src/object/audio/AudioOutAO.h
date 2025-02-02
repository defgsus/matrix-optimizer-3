/** @file audiooutao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_AUDIOOUTAO_H
#define MOSRC_OBJECT_AUDIO_AUDIOOUTAO_H

#include "object/AudioObject.h"

namespace MO {

class AudioOutAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(AudioOutAO)

    virtual void createParameters() Q_DECL_OVERRIDE;

    /** pull into public namespace */
    void setNumberAudioInputsOutputs(int num) { AudioObject::setNumberAudioInputsOutputs(num, num, false); }

protected:

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;
private:

    ParameterFloat
        * paramAmp_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_AUDIOOUTAO_H
