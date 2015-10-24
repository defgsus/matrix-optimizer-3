#ifndef NOISEAO_H
#define NOISEAO_H

#include "object/audioobject.h"

namespace MO {

class NoiseAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(NoiseAO);
    ~NoiseAO();

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual QString getAudioInputName(uint channel) const Q_DECL_OVERRIDE;

protected:

    void processWhiteNoise(const RenderTime& time);
    void processPinkNoise(const RenderTime & time);

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;

private:

    class Private;
    Private *p_;
};

}

#endif // NOISEAO_H
