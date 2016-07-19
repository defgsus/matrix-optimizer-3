/** @file audioobject.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIOOBJECT_H
#define MOSRC_OBJECT_AUDIOOBJECT_H

#include "Object.h"

namespace MO {
namespace AUDIO { class AudioBuffer; }

class AudioObject : public Object
{
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(AudioObject)

    virtual Type type() const Q_DECL_OVERRIDE { return T_AUDIO_OBJECT; }
    virtual bool isAudioObject() const Q_DECL_OVERRIDE { return true; }

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;

    // --------- getter -------------

    /** Returns the desired number of audio input channels.
        -1 for don't care. */
    int numAudioInputs() const;
    /** Returns the desired number of audio output channels */
    uint numAudioOutputs() const;

    virtual QString getAudioInputName(uint channel) const { return QString("in %1").arg(channel + 1); }
    virtual QString getAudioOutputName(uint channel) const { return QString("out %1").arg(channel + 1); }

    /** Override of Object base method */
    bool hasAudioOutput() const Q_DECL_OVERRIDE { return numAudioOutputs() != 0; }

    // ---------- dsp ---------------

    /** Override to adjust to changes of buffersize.
        Audio thread will be halted and a different thread
        will call this function. */
    virtual void setBufferSize(uint bufferSize, uint thread) { Q_UNUSED(bufferSize); Q_UNUSED(thread); }

    /** Sets the audio buffers to use for the particular thread.
        Inputs and outputs must have the same buffer size.
        @note Unused inputs and outputs contain a NULL pointer.
        Calls virtual setAudioBuffers(). */
    virtual void setAudioBuffersBase(uint thread, uint bufferSize,
                                     const QList<AUDIO::AudioBuffer*>& inputs,
                                     const QList<AUDIO::AudioBuffer*>& outputs);

    const QList<AUDIO::AudioBuffer*> & audioInputs(uint thread) const;
    const QList<AUDIO::AudioBuffer*> & audioOutputs(uint thread) const;

    /** @todo Add sampleRate here as well??
     *  Right now it's non-reentrant in Object base */

    /** Processes dsp data. */
    void processAudioBase(const RenderTime& time);

#ifndef MO_DISABLE_CLIENT
    /** Should be called when an udp packet has been received for the object */
    void clientFakeAudio(const RenderTime & time);
#endif

    /** Called before the audio thread is going away.
        Override to clear resources that might depend on the current thread.
        Always call ancestor method in derived classes! */
    virtual void onCloseAudioThread() { }

    // ------------------ modulator outputs -------------------

    /** Provides normal float values for the non-audio threads
        of each output channel.
        @p thread is the audio thread index.
        //XXX should: Work by looking up the previous output buffer. but thread issue here
        Fortunately, it works quite well, on my system at least
        Returns 0.0 for unknown or empty channels. */
    Double getAudioOutputAsFloat(uint channel, const RenderTime& time) const;

protected: // ---------------- protected virtual interface -----------------------

    /** Call in constructor of derived class to enable a channel select parameter */
    void setNumberChannelsAdjustable(bool);

    /** Sets the desired number of channels.
        Call this in constructor or in onParameterChanged()/onParametersLoaded().
        Calls ObjectEditor::emitAudioChannelsChanged() when connected. */
    void setNumberAudioInputs(int num, bool emitSignal = true);
    /** Sets the desired number of channels.
        Call this in constructor or in onParameterChanged()/onParametersLoaded().
        Calls ObjectEditor::emitAudioChannelsChanged() when connected. */
    void setNumberAudioOutputs(uint num, bool emitSignal = true);
    /** Sets the desired number of channels.
        Call this in constructor or in onParameterChanged()/onParametersLoaded().
        Calls ObjectEditor::emitAudioChannelsChanged() when connected. */
    void setNumberAudioInputsOutputs(uint numIn, uint numOut, bool emitSignal = true);

    // ------------------- runtime virtuals ------------------------------

    /** Override to adjust to the number of channels.
        Inputs and outputs have the same buffer size.
        @note Unused inputs and outputs contain a NULL pointer.
        Also note that the number of output channels might not match your desired
        number of channels set with setNumberAudioOutputs().
        Will be called by a non-audio thread.
        @note Can't emit change signals here, so no changes to the object allowed! */
    virtual void setAudioBuffers(uint thread, uint bufferSize,
                                 const QList<AUDIO::AudioBuffer*>& inputs,
                                 const QList<AUDIO::AudioBuffer*>& outputs)
    { Q_UNUSED(thread); Q_UNUSED(bufferSize); Q_UNUSED(inputs); Q_UNUSED(outputs); }

    /** Process dsp data here.
        The inputs and outputs are in audioInputs() and audioOutputs() respectively.
        Called by audio thread. */
    virtual void processAudio(const RenderTime& time) = 0;

    /** Call this to clear the audio output channels from inside processAudio(). */
    void writeNullBlock(SamplePos pos, uint thread);

private:
    class PrivateAO;
    PrivateAO * p_ao_;
};

} // namespace MO


#endif // MOSRC_OBJECT_AUDIOOBJECT_H
