/** @file parameterao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_PARAMETERAO_H
#define MOSRC_OBJECT_AUDIO_PARAMETERAO_H

#include "object/audioobject.h"

namespace MO {

class ParameterAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(ParameterAO)

    virtual void createParameters() Q_DECL_OVERRIDE;

protected:

    virtual void processAudio(uint bufferSize, SamplePos pos, uint thread)
                                                            Q_DECL_OVERRIDE;
private:

    ParameterFloat
        * paramValue_,
        * paramRate_;

    F32 lastSample_, lastSample2_;
    SamplePos samplesWaited_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_PARAMETERAO_H
