/** @file microphone.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MO_DISABLE_SPATIAL

#ifndef MOSRC_OBJECT_MICROPHONE_H
#define MOSRC_OBJECT_MICROPHONE_H

#include "object.h"

namespace MO {

class Microphone : public Object
{
public:
    MO_OBJECT_CONSTRUCTOR(Microphone);

    virtual Type type() const Q_DECL_OVERRIDE { return T_MICROPHONE; }
    virtual bool isMicrophone() const Q_DECL_OVERRIDE { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void processMicrophoneBuffers(
            const QList<AUDIO::SpatialMicrophone*>& microphones,
            const RenderTime& time);

private:

    ParameterFloat
        * paramAmp_,
        * paramDistFade_,
        * paramDirExp_,
        * paramDistMin_,
        * paramDistMax_;
    ParameterSelect
        * paramUseDist_;
};

} // namespace MO

#endif // MOSRC_OBJECT_MICROPHONE_H

#endif // #ifndef MO_DISABLE_SPATIAL
