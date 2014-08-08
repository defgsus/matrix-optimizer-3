/** @file filterunit.h

    @brief AudioUnit that filters input in specific modes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_FILTERUNIT_H
#define MOSRC_OBJECT_AUDIO_FILTERUNIT_H

#include "audiounit.h"
#include "audio/audio_fwd.h"

namespace MO {


class FilterUnit : public AudioUnit
{
    Q_OBJECT

public:
    MO_OBJECT_CONSTRUCTOR(FilterUnit);
    ~FilterUnit();

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    // --------------- processing ------------

    /** Called for each block of audio data */
    virtual void processAudioBlock(const F32* input, F32* output, Double time, uint thread)
                                                                                Q_DECL_OVERRIDE;

protected:

    virtual void channelsChanged() Q_DECL_OVERRIDE;

private:

    void createFilters_();

    ParameterFloat * freq_, * reso_;

    /** [numChannelsIn][numberThreads] */
    std::vector<AUDIO::MultiFilter*> filter_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_FILTERUNIT_H
