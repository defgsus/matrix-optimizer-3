#ifndef IMPULSEAO_H
#define IMPULSEAO_H

#include "object/audioobject.h"

namespace MO {

class ImpulseAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(ImpulseAO);
    ~ImpulseAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual QString getAudioInputName(uint channel) const Q_DECL_OVERRIDE;

protected:

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;

private:

    class Private;
    Private *p_;
};

}

#endif // IMPULSEAO_H
