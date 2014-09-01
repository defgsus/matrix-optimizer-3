/** @file microphone.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_MICROPHONE_H
#define MOSRC_OBJECT_MICROPHONE_H

#include "object.h"

namespace MO {

class Microphone : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Microphone);

    virtual Type type() const Q_DECL_OVERRIDE { return T_MICROPHONE; }
    virtual bool isMicrophone() const Q_DECL_OVERRIDE { return true; }

    virtual void createMicrophones() Q_DECL_OVERRIDE;

    virtual void updateAudioTransformations(Double time, uint thread) Q_DECL_OVERRIDE;
    virtual void updateAudioTransformations(Double time, uint blockSize, uint thread)
                                                                    Q_DECL_OVERRIDE;

    /** Spatially sample the audio source and add the result to buffer */
    //void sampleAudioSource(const AUDIO::AudioSource* src, F32* buffer, uint thread) const;


signals:

public slots:

private:

    AUDIO::AudioMicrophone * microphone_;

};

} // namespace MO

#endif // MOSRC_OBJECT_MICROPHONE_H
