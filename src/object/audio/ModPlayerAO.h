/** @file modplayerao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.12.2014</p>
*/

#ifndef MO_DISABLE_DUMB

#ifndef MOSRC_OBJECT_AUDIO_MODPLAYERAO_H
#define MOSRC_OBJECT_AUDIO_MODPLAYERAO_H

#include "object/AudioObject.h"

namespace MO {

class ModPlayerAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(ModPlayerAO)

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

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_MODPLAYERAO_H

#endif // #ifndef MO_DISABLE_DUMB
