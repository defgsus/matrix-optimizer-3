/** @file audiounit.h

    @brief Abstract audio processing object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_AUDIOUNIT_H
#define MOSRC_OBJECT_AUDIO_AUDIOUNIT_H

#include <vector>

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

    Type type() const Q_DECL_OVERRIDE Q_DECL_FINAL { return T_AUDIO_UNIT; }

    bool isAudioUnit() const Q_DECL_OVERRIDE Q_DECL_FINAL { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    // -------------- getter -----------------

    /** Returns the currently set process mode */
    ProcessMode processMode() const;

    uint maxChannelsIn() const { return maximumChannelsIn_; }
    uint maxChannelsOut() const { return maximumChannelsOut_; }

    uint numChannelsIn() const { return numberChannelsIn_; }
    uint numChannelsOut() const { return numberChannelsOut_; }

    bool numberInputOutputChannelsMustMatch() const { return sameNumberInputOutputChannels_; }

    /** Returns write access to the output buffer */
    F32 * outputBuffer(uint thread) { return &audioOutputBuffer_[thread][0]; }
    /** Returns read access to the output buffer */
    const F32 * outputBuffer(uint thread) const { return &audioOutputBuffer_[thread][0]; }

    // -------------- setter -----------------

    /** Sets the number of channels.
        If numberInputOutputChannelsMustMatch() is true, the number of
        output channels is set to the number of input channels!
        The @p thread is only used for distinguishing the bufferSize, while
        the number of channels is equal to all threads. */
    void setNumChannelsInOut(uint numIn, uint numOut, uint thread);
    void setNumChannelsIn(uint num, uint thread);
    void setNumChannelsOut(uint num, uint thread);

    // --------- output modulators -----------

    // XXX w.i.p.
    //virtual void createModulatorObjects() { };

    // ------------ processing ---------------

    //virtual void performAudioBlock(SamplePos pos, uint thread) Q_DECL_OVERRIDE;

    /** Called for each block of audio data.
        The format of input and output is [channels][blockSize],
        so each channel has it's sequential samples.
        The output buffer is in outputBuffer().
        @note This function is only called when processMode() == PM_ON */
    virtual void processAudioBlock(const F32* input, Double time, uint thread) = 0;

protected:

    /** Called when the number of channels has changed.
        Always call ancestor's implementation before your derived code. */
    virtual void channelsChanged(uint thread);

    /** Puts @p input to @p output.
        The number of channels are considered and output is filled with zeros if
        input-channels < output-channels */
    void performBypass_(const F32 * input, F32 *output, uint thread) const;

private:

    void processAudioBlock_(const F32* input, Double time, uint thread);

    ParameterSelect * processModeParameter_;

    uint maximumChannelsIn_, maximumChannelsOut_,
         numberChannelsIn_, numberChannelsOut_;

    bool sameNumberInputOutputChannels_;

    /** [thread] [channels][bufferSize] */
    std::vector<std::vector<F32>> audioOutputBuffer_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_AUDIOUNIT_H
