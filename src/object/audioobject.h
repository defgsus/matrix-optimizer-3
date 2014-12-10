/** @file audioobject.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef AUDIOOBJECT_H
#define AUDIOOBJECT_H

#include "object.h"

namespace MO {
namespace AUDIO { class AudioBuffer; }

class AudioObject : public Object
{
    Q_OBJECT
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(AudioObject)
    ~AudioObject();

    virtual Type type() const Q_DECL_OVERRIDE { return T_AUDIO_OBJECT; }
    virtual bool isAudioObject() const Q_DECL_OVERRIDE { return true; }

    // --------- getter -------------

    /** Returns the desired number of audio input channels.
        -1 for don't care. */
    int numAudioInputs() const;
    /** Returns the desired number of audio output channels */
    uint numAudioOutputs() const;

    virtual QString getInputName(uint channel) const { return QString("in %1").arg(channel + 1); }
    virtual QString getOutputName(uint channel) const { return QString("out %1").arg(channel + 1); }


    // ---------- dsp ---------------

    /** Override to adjust to changes of buffersize.
        Audio thread will be halted and a different thread
        will call this function */
    virtual void setBufferSize(uint bufferSize, uint thread) { Q_UNUSED(bufferSize); Q_UNUSED(thread); }

    /** @todo Add sampleRate here as well??
     *  Right now it's non-reentrant in Object base */

    /** Processes dsp data.
        Inputs and outputs must have the same buffer size.
        @note Unused inputs and outputs contain a NULL pointer. */
    void processAudioBase(const QList<AUDIO::AudioBuffer*>& inputs,
                          const QList<AUDIO::AudioBuffer*>& outputs,
                          uint bufferSize, SamplePos pos, uint thread);

protected:

    /** Process dsp data here.
        Inputs and outputs have the same buffer size.
        @note Unused inputs and outputs contain a NULL pointer.
        Also note that the number of output channels might not match your desired
        number of channels set with setNumberAudioOutputs(). */
    virtual void processAudio(const QList<AUDIO::AudioBuffer*>& inputs,
                              const QList<AUDIO::AudioBuffer*>& outputs,
                              uint bufferSize, SamplePos pos, uint thread) = 0;

    /** Sets the desired number of channels and emits ObjectEditor::objectChanged() */
    void setNumberAudioInputs(int num);
    /** Sets the desired number of channels and emits ObjectEditor::objectChanged() */
    void setNumberAudioOutputs(uint num);
    /** Sets both input and output channels and emits ObjectEditor::objectChanged() */
    void setNumberAudioInputsOutputs(uint num);

private:
    class PrivateAO;
    PrivateAO * p_ao_;
};

} // namespace MO


#endif // AUDIOOBJECT_H
