#ifndef PLAYBUFFERAO_H
#define PLAYBUFFERAO_H

#include "object/audioobject.h"

namespace MO {

class PlayBufferAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(PlayBufferAO);
    ~PlayBufferAO();

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual QString getAudioInputName(uint channel) const Q_DECL_OVERRIDE;
    virtual QString getAudioOutputName(uint channel) const Q_DECL_OVERRIDE;

protected:
    virtual void processAudio(uint bufferSize, SamplePos pos, uint thread) Q_DECL_OVERRIDE;

private:

    class Private;
    Private *p_;
};

}

#endif // PLAYBUFFERAO_H
