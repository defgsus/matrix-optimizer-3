/** @file audioinao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_AUDIOINAO_H
#define MOSRC_OBJECT_AUDIO_AUDIOINAO_H


#include "object/audioobject.h"

namespace MO {

class AudioInAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(AudioInAO)

    virtual void createParameters() Q_DECL_OVERRIDE;

    /** pull into public namespace */
    void setNumberAudioInputsOutputs(int num) { AudioObject::setNumberAudioInputsOutputs(num, false); }

protected:

    virtual void processAudio(uint bufferSize, SamplePos pos, uint thread)
                                                            Q_DECL_OVERRIDE;
private:

    ParameterFloat
        * paramAmp_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_AUDIOINAO_H
