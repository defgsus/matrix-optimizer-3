/** @file filterunit.h

    @brief AudioUnit that filters input in specific modes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef FILTERUNIT_H
#define FILTERUNIT_H

#include "audiounit.h"

namespace MO {


class FilterUnit : public AudioUnit
{
    Q_OBJECT

public:
    MO_OBJECT_CONSTRUCTOR(FilterUnit);

    virtual void createParameters() Q_DECL_OVERRIDE;

    // --------------- processing ------------

    /** Called for each block of audio data */
    virtual void processAudioBlock(const F32* input, F32* output, Double time, uint thread)
                                                                                Q_DECL_OVERRIDE;

protected:

    virtual void channelsChanged() Q_DECL_OVERRIDE { }

    ParameterFloat * freq_, * reso_;
};

} // namespace MO

#endif // FILTERUNIT_H
