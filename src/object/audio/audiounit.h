/** @file audiounit.h

    @brief Abstract audio processing object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_AUDIOUNIT_H
#define MOSRC_OBJECT_AUDIO_AUDIOUNIT_H

#include <QStringList>

#include "object/object.h"

#define MO_MAX_AUDIO_CHANNELS 256

namespace MO {

class AudioUnit : public Object
{
    Q_OBJECT

    friend class Scene; // for processAudioBlock_()

public:

    enum ProcessMode
    {
        PM_ON,
        PM_OFF,
        PM_BYPASS
    };

    const static QStringList processModeIds;

    /** Constructs an AudioUnit.
        If @p maxChannelsIn or @p maxChannelsOut == -1,
        they will be set to MO_MAX_AUDIO_CHANNELS. */
    MO_ABSTRACT_OBJECT_CONSTRUCTOR_3(AudioUnit,
                                     int maxChannelsIn = -1, int maxChannelsOut = -1,
                                     bool sameNumInputOutputChannels = true);

    Type type() const Q_DECL_OVERRIDE { return T_AUDIO_UNIT; }

    bool isAudioUnit() const Q_DECL_OVERRIDE { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    // -------------- getter -----------------

    /** Returns the currently set process mode */
    ProcessMode processMode() const;

    uint maxChannelsIn() const { return maximumChannelsIn_; }
    uint maxChannelsOut() const { return maximumChannelsOut_; }

    uint numChannelsIn() const { return numberChannelsIn_; }
    uint numChannelsOut() const { return numberChannelsOut_; }

    bool numberInputOutputChannelsMustMatch() const { return sameNumberInputOutputChannels_; }

    // -------------- setter -----------------

    void setNumChannelsIn(uint num);
    void setNumChannelsOut(uint num);
    void setNumChannelsInOut(uint numIn, uint numOut);

    // ------------ processing ---------------

    //virtual void performAudioBlock(SamplePos pos, uint thread) Q_DECL_OVERRIDE;

    /** Called for each block of audio data.
        The format of input and output is [channels][blockSize],
        so each channel has sequential samples. */
    virtual void processAudioBlock(const F32* input, F32* output, Double time, uint thread) = 0;

protected:

    /** Called when the number of channels has changed. */
    virtual void channelsChanged() = 0;

    /** Puts @p input to @p output.
        The number of channels are considered and output is filled with zeros if
        input-channels < output-channels */
    void performBypass_(const F32 * input, F32 * output, uint thread);

private:

    void processAudioBlock_(const F32* input, F32* output, Double time, uint thread);

    ParameterSelect * processModeParameter_;

    uint maximumChannelsIn_, maximumChannelsOut_,
         numberChannelsIn_, numberChannelsOut_;

    bool sameNumberInputOutputChannels_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_AUDIOUNIT_H
