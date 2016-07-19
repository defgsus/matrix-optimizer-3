#ifndef PHASORAO_H
#define PHASORAO_H

#include "object/AudioObject.h"

namespace MO {

class PhasorAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(PhasorAO);

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual QString getAudioInputName(uint channel) const Q_DECL_OVERRIDE;

protected:

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;
private:

    class Private;
    Private *p_;
};

}
#endif // PHASORAO_H
