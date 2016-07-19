/** @file microphonegroup.h

    @brief Group for uniformly setup microphones

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#ifndef MOSRC_OBJECT_MICROPHONEGROUP_H
#define MOSRC_OBJECT_MICROPHONEGROUP_H

#include "Object.h"

namespace MO {

class MicrophoneGroup : public Object
{
public:
    MO_OBJECT_CONSTRUCTOR(MicrophoneGroup);

    virtual Type type() const Q_DECL_OVERRIDE { return T_MICROPHONE_GROUP; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void calculateMicrophoneTransformation(
                    const TransformationBuffer * objectTransformation,
                    const QList<AUDIO::SpatialMicrophone*>&,
                    const RenderTime& time) Q_DECL_OVERRIDE;

    virtual QString infoString() const Q_DECL_OVERRIDE;


    static void getMicVectors(std::vector<Vec3>& v, const FloatMatrix& m);

private:

    ParameterFloat
        * paramMicDist_,
        * paramAmp_,
        * paramDistFade_,
        * paramDirExp_,
        * paramDistMin_,
        * paramDistMax_;
    ParameterSelect
        * paramUseDist_;
    ParameterInt
        * paramNumMics_;
    ParameterFloatMatrix
        * paramMatrix_;
};

} // namespace MO

#endif // MOSRC_OBJECT_MICROPHONEGROUP_H

