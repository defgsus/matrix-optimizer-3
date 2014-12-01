/** @file oscillatorao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef OSCILLATORAO_H
#define OSCILLATORAO_H

#include "object/audioobject.h"

namespace MO {

class OscillatorAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(OscillatorAO)

    virtual void createParameters() Q_DECL_OVERRIDE;

protected:

    /** Process dsp data here.
        Inputs and outputs have the same buffer size */
    virtual void processAudio(const QList<AUDIO::AudioBuffer*>& inputs,
                              const QList<AUDIO::AudioBuffer*>& outputs,
                              uint bufferSize, SamplePos pos, uint thread)
                                                            Q_DECL_OVERRIDE;
private:

    Double phase_;
    ParameterFloat
        * paramFreq_,
        * paramPhase_,
        * paramAmp_,
        * paramOffset_,
        * paramSync_;
};

} // namespace MO


#endif // OSCILLATORAO_H
