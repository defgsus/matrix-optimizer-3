/** @file waveform.cpp

    @brief Basic waveform f(x)-style calculations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#include "waveform.h"
#include "functions.h"
#include "constants.h"
#include "noiseperlin.h"

namespace MO {
namespace MATH {

NoisePerlin Waveform::noise_;


const QStringList Waveform::typeIds =
{
    "sin", "cos", "ramp", "saw", "tri", "sqr", "noise"
};

const QStringList Waveform::typeNames =
{
    "Sine", "Cosine", "Ramp", "Sawtooth", "Triangle", "Square", "Noise"
};

bool Waveform::supportsPulseWidth(Type t)
{
    return t == T_RAMP
        || t == T_SAW
        || t == T_TRIANGLE
        || t == T_SQUARE;
}

Double Waveform::waveform(Double t, Type type)
{
    Double p;

    switch (type)
    {
        case T_SINE:
            return std::sin( t * TWO_PI );

        case T_COSINE:
            return std::cos( t * TWO_PI );

        case T_RAMP:
            return moduloSigned( t, 1.0 );

        case T_SAW:
            return -1.0 + 2.0 * moduloSigned( t, 1.0 );

        case T_TRIANGLE:
            p = moduloSigned(t, 1.0);
            return (p<0.5)? p * 4.0 - 1.0 : (1.0-p) * 4.0 - 1.0;

        case T_SQUARE:
            return (moduloSigned( t, 1.0 ) >= 0.5) ? -1.0 : 1.0 ;

        case T_NOISE:
            return noise_.noise(t);

        default:
            return 0.0;
    }

}


Double Waveform::waveform(Double t, Type type, Double pw)
{
    Double p;

    switch (type)
    {
        case T_SINE:
            return std::sin( t * TWO_PI );

        case T_COSINE:
            return std::cos( t * TWO_PI );

        case T_RAMP:
            // TODO
            return moduloSigned( t, 1.0 );

        case T_SAW:
            // TODO
            return -1.0 + 2.0 * moduloSigned( t, 1.0 );

        case T_TRIANGLE:
            p = moduloSigned(t, 1.0);
            return (p<pw)? p * 2.0/pw - 1.0 : (1.0-p) * 2.0/(1.0-pw) - 1.0;

        case T_SQUARE:
            return (moduloSigned( t, 1.0 ) >= pw) ? -1.0 : 1.0 ;

        case T_NOISE:
            return noise_.noise(t);

        default:
            return 0.0;
    }

}



} // namespace MATH
} // namespace MO
