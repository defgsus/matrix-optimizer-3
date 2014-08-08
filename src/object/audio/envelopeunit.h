/** @file envelopeunit.h

    @brief Envelope follower as AudioUnit

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_ENVELOPEUNIT_H
#define MOSRC_OBJECT_AUDIO_ENVELOPEUNIT_H

#include "audiounit.h"
#include "audio/audio_fwd.h"

namespace MO {


class EnvelopeUnit : public AudioUnit
{
    Q_OBJECT

public:
    MO_OBJECT_CONSTRUCTOR(EnvelopeUnit);
    ~EnvelopeUnit();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    // --------------- processing ------------

    /** Called for each block of audio data */
    virtual void processAudioBlock(const F32* input, Double time, uint thread) Q_DECL_OVERRIDE;

protected:

    virtual void channelsChanged(uint thread) Q_DECL_OVERRIDE;

private:

    void createFollowers_();

    ParameterFloat * fadeIn_, * fadeOut_;

    /** [numChannelsIn][numberThreads] */
    std::vector<AUDIO::EnvelopeFollower*> follower_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_ENVELOPEUNIT_H
