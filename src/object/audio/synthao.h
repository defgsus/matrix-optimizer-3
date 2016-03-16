/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/16/2016</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_SYNTHAO_H
#define MOSRC_OBJECT_AUDIO_SYNTHAO_H

#include "object/audioobject.h"

namespace MO {

class SynthAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(SynthAO)

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter * p) Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

protected:

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO


#endif // MOSRC_OBJECT_AUDIO_SYNTHAO_H
