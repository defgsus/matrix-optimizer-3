#ifndef PANAO_H
#define PANAO_H

#include "object/audioobject.h"

namespace MO {

class PanAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(PanAO);
    ~PanAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual QString getAudioInputName(uint channel) const Q_DECL_OVERRIDE;

protected:

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;

private:

    class Private;
    Private *p_;
};

}
#endif // PANAO_H
