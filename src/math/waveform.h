/** @file waveform.h

    @brief Basic waveform f(x)-style calculations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#ifndef MOSRC_MATH_WAVEFORM_H
#define MOSRC_MATH_WAVEFORM_H

#include <QStringList>

#include "types/float.h"

namespace MO {
namespace MATH {

class NoisePerlin;

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
    const static int WT_MAX_TYPES = T_NOISE + 1;

    /** PERSISTENT ids for each Type enum. */
    const static QStringList typeIds;

    /** Friendly names for each Type enum. */
    const static QStringList typeNames;

    /** Returns the f(x) for the specified time and type. */
    static Double waveform(Double time, Type type, Double pulseWidth = 0.5);

private:
    /** noise class user for noise waveform */
    static NoisePerlin noise_;

};




} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_WAVEFORM_H
