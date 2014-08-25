/** @file waveform.h

    @brief Basic waveform f(x)-style calculations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_WAVEFORM_H
#define MOSRC_AUDIO_TOOL_WAVEFORM_H

#include <QStringList>

#include "types/float.h"

namespace MO {
namespace MATH { class NoisePerlin; }
namespace AUDIO {

class Waveform
{
    public:

    /** Types of waveforms */
    enum Type
    {
        T_SINE,
        T_COSINE,
        T_RAMP,
        T_SAW,
        T_TRIANGLE,
        T_SQUARE,
        T_NOISE
    };
    /** Number of waveform types Waveform::Type */
    const static int T_MAX_TYPES = T_NOISE + 1;

    /** PERSISTENT ids for each Type enum. */
    const static QStringList typeIds;

    /** Friendly names for each Type enum. */
    const static QStringList typeNames;

    /** Description of each Type enum */
    const static QStringList typeStatusTips;

    /** All Type enums in order */
    const static QList<int> typeList;

    static Double minPulseWidth() { return 0.00001; }
    static Double maxPulseWidth() { return 0.99999; }

    /** Limits the pulsewidth if nescessary. */
    static Double limitPulseWidth(Double pw)
        { return std::max(minPulseWidth(), std::min(maxPulseWidth(), pw )); }

    /** Returns true if the specified type supports the pulsewidth setting */
    static bool supportsPulseWidth(Type);

    /** Returns the f(x) for the specified time and type. */
    static Double waveform(Double time, Type type);

    /** Returns the f(x) for the specified time and type.
        @p pulseWidth is only used by types that support it.
        @note pulseWidth must be between minPulseWidth() and maxPulseWidth() !
        @see supportsPulseWidth() */
    static Double waveform(Double time, Type type, Double pulseWidth = 0.5);

    static Double spectralWave(Double time,
                               Double numPartials,
                               Double octaveStep,
                               Double basePhase,
                               Double phaseShift,
                               Double amplitudeMult);
private:
    /** noise class user for noise waveform */
    static MATH::NoisePerlin noise_;

};




} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_WAVEFORM_H
