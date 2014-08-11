/** @file filterbankunit.h

    @brief Filterbank AudioUnit object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/11/2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_FILTERBANKUNIT_H
#define MOSRC_OBJECT_AUDIO_FILTERBANKUNIT_H

#include "audiounit.h"
#include "audio/audio_fwd.h"

namespace MO {


class FilterBankUnit : public AudioUnit
{
    Q_OBJECT

public:
    MO_OBJECT_CONSTRUCTOR(FilterBankUnit);
    ~FilterBankUnit();

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;

    // --------------- processing ------------

    /** Called for each block of audio data */
    virtual void processAudioBlock(const F32* input, Double time, uint thread) Q_DECL_OVERRIDE;

protected:

    virtual void channelsChanged() Q_DECL_OVERRIDE;

private:

    void createFilters_();

    ParameterInt * numOut_;
    ParameterSelect * type_;
    ParameterFloat * baseFreq_, *linearFreq_, *quadFreq_, * reso_;

    /** [numChannelsOut][numberThreads] */
    std::vector<AUDIO::MultiFilter*> filter_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_FILTERBANKUNIT_H
