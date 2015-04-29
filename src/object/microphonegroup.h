/** @file microphonegroup.h

    @brief Group for uniformly setup microphones

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#ifndef MO_DISABLE_EXP

#ifndef MOSRC_OBJECT_MICROPHONEGROUP_H
#define MOSRC_OBJECT_MICROPHONEGROUP_H

#include "object.h"

namespace MO {

class MicrophoneGroup : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(MicrophoneGroup);

    virtual Type type() const Q_DECL_OVERRIDE { return T_MICROPHONE_GROUP; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;

    virtual void calculateMicrophoneTransformation(
                    const TransformationBuffer * objectTransformation,
                    const QList<AUDIO::SpatialMicrophone*>&,
                    uint bufferSize, SamplePos pos, uint thread) Q_DECL_OVERRIDE;

signals:

public slots:

private:

    ParameterInt
        * pNumMics_;
    ParameterFloat
        * pDistance_, * pDirExp_;

    //QList<AUDIO::AudioMicrophone*> micros_;
};

} // namespace MO

#endif // MOSRC_OBJECT_MICROPHONEGROUP_H

#endif // MO_DISABLE_EXP
