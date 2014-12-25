/** @file soundsourceao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_SOUNDSOURCEAO_H
#define MOSRC_OBJECT_AUDIO_SOUNDSOURCEAO_H

#include "object/audioobject.h"

namespace MO {

class SoundSourceAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(SoundSourceAO)

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void calculateSoundSourceBuffer(const QList<AUDIO::SpatialSoundSource*>,
                                            uint bufferSize, SamplePos pos, uint thread)
                                            Q_DECL_OVERRIDE;
protected:

    virtual void processAudio(uint , SamplePos , uint ) Q_DECL_OVERRIDE { }
private:

};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_SOUNDSOURCEAO_H
