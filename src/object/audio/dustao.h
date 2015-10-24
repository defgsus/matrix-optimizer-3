#ifndef DUSTAO_H
#define DUSTAO_H

#include "object/audioobject.h"

namespace MO {

class DustAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(DustAO);
    ~DustAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual QString getAudioInputName(uint channel) const Q_DECL_OVERRIDE;

protected:

    void processDust(const RenderTime& time);
    void processDust2(const RenderTime& time);
    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;

private:

    class Private;
    Private *p_;
};

}

#endif // DUSTAO_H
