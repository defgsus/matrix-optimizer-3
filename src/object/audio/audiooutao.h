/** @file audiooutao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#ifndef AUDIOOUTAO_H
#define AUDIOOUTAO_H

#include <QObject>

#include "object/audioobject.h"

namespace MO {

class AudioOutAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(AudioOutAO)

    virtual void createParameters() Q_DECL_OVERRIDE;

protected:

    /** Process dsp data here.
        Inputs and outputs have the same buffer size.
        @note Unused inputs and outputs contain a NULL pointer.*/
    virtual void processAudio(const QList<AUDIO::AudioBuffer*>& inputs,
                              const QList<AUDIO::AudioBuffer*>& outputs,
                              uint bufferSize, SamplePos pos, uint thread)
                                                            Q_DECL_OVERRIDE;
private:

    void clear_();

    ParameterFloat
        * paramAmp_;
};

} // namespace MO

#endif // AUDIOOUTAO_H
