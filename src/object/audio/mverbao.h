/** @file mverbao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 04.12.2014</p>
*/

#ifndef MO_DISABLE_EXP

#ifndef MOSRC_OBJECT_AUDIO_MVERBAO_H
#define MOSRC_OBJECT_AUDIO_MVERBAO_H

#include <QObject>

#include "object/audioobject.h"

namespace MO {

class MVerbAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(MVerbAO)
    ~MVerbAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint count) Q_DECL_OVERRIDE;

protected:

    virtual void processAudio(uint bufferSize, SamplePos pos, uint thread)
                                                            Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO


#endif // MOSRC_OBJECT_AUDIO_MVERBAO_H

#endif // #ifndef MO_DISABLE_EXP
