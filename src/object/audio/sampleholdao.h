#ifndef SAMPLEHOLDAO_H
#define SAMPLEHOLDAO_H

#include "object/audioobject.h"

namespace MO {

class SampleHoldAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(SampleHoldAO);
    ~SampleHoldAO();

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual QString getAudioInputName(uint channel) const Q_DECL_OVERRIDE;

protected:

    virtual void processAudio(uint, SamplePos pos, uint thread) Q_DECL_OVERRIDE;

private:

    class Private;
    Private *p_;
};

}

#endif // SAMPLEHOLDAO_H
