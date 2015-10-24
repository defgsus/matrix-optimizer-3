/** @file convolveao.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.05.2015</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_CONVOLVEAO_H
#define MOSRC_OBJECT_AUDIO_CONVOLVEAO_H

#include "object/audioobject.h"

namespace MO {

class ConvolveAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(ConvolveAO)
    ~ConvolveAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void setSampleRate(uint samplerate) Q_DECL_OVERRIDE;
protected:

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;
private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_CONVOLVEAO_H
