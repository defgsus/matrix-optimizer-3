/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/19/2016</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_SHADERAO_H
#define MOSRC_OBJECT_AUDIO_SHADERAO_H

#include "object/audioobject.h"
#include "object/interface/valueshadersourceinterface.h"

namespace MO {

/** GLSL sample generator */
class ShaderAO
        : public AudioObject
        , public ValueShaderSourceInterface
{
public:
    MO_OBJECT_CONSTRUCTOR(ShaderAO);

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    // ValueShaderSourceInterface
    virtual GL::ShaderSource valueShaderSource(uint channel) const Q_DECL_OVERRIDE;

protected:

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;
    virtual void onCloseAudioThread() Q_DECL_OVERRIDE;

private:

    class Private;
    Private *p_;
};

} // namepsace MO

#endif // MOSRC_OBJECT_AUDIO_SHADERAO_H
