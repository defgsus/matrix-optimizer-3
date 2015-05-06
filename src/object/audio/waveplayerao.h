/** @file waveplayerao.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_WAVEPLAYERAO_H
#define MOSRC_OBJECT_AUDIO_WAVEPLAYERAO_H

#include "object/audioobject.h"

namespace MO {

/** A straight-forward SoundFile player */
class WavePlayerAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(WavePlayerAO)
    ~WavePlayerAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList & files) Q_DECL_OVERRIDE;

protected:

    virtual void setAudioBuffers(uint thread, uint bufferSize,
                                 const QList<AUDIO::AudioBuffer*>& inputs,
                                 const QList<AUDIO::AudioBuffer*>& outputs)
                                                            Q_DECL_OVERRIDE;

    virtual void processAudio(uint bufferSize, SamplePos pos, uint thread)
                                                            Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_WAVEPLAYERAO_H
