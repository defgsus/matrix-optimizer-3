/** @file microphonegroup.h

    @brief Group for uniformly setup microphones

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

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
    //virtual void onParametersLoaded() Q_DECL_OVERRIDE;

    virtual void createMicrophones() Q_DECL_OVERRIDE;

    virtual void updateAudioTransformations(Double time, uint thread)
                                                            Q_DECL_OVERRIDE;
    virtual void updateAudioTransformations(Double time, uint blocksize, uint thread)
                                                            Q_DECL_OVERRIDE;
signals:

public slots:

private:

    Mat4 getMicroTransformation_(uint index, Float dist) const;
    Mat4 getMicroTransformation_(uint index, Double time, uint thread) const;

    ParameterInt
        * pNumMics_;
    ParameterFloat
        * pDistance_;

    QList<AUDIO::AudioMicrophone*> micros_;

};

} // namespace MO

#endif // MOSRC_OBJECT_MICROPHONEGROUP_H
