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
    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual QString getInputName(uint channel) const Q_DECL_OVERRIDE;

protected:

    /** Process dsp data here.
        Inputs and outputs have the same buffer size.
        @note Unused inputs and outputs contain a NULL pointer.*/
    virtual void processAudio(const QList<AUDIO::AudioBuffer*>& inputs,
                              const QList<AUDIO::AudioBuffer*>& outputs,
                              uint bufferSize, SamplePos pos, uint thread)
                                                            Q_DECL_OVERRIDE;
private:

    std::vector<Double> phase_;
    ParameterFloat
        * paramFreq_,
        * paramPhase_,
        * paramAmp_,
        * paramOffset_,
        * paramSync_;
};

} // namespace MO


#endif // OSCILLATORAO_H
