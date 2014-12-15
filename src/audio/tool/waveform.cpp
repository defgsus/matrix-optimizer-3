/** @file waveform.cpp

    @brief Basic waveform f(x)-style calculations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#include <QObject> // for tr()

#include "waveform.h"
#include "math/functions.h"
#include "math/constants.h"
#include "math/noiseperlin.h"

namespace MO {
namespace AUDIO {

MATH::NoisePerlin Waveform::noise_;

const QList<int> Waveform::typeList =
{ T_SINE, T_COSINE, T_RAMP, T_SAW_RISE, T_SAW_DECAY, T_TRIANGLE, T_SQUARE, T_NOISE };

const QStringList Waveform::typeIds =
{
    "sin", "cos", "ramp", "saw", "sawd", "tri", "sqr", "noise"
};

const QStringList Waveform::typeNames =
{
    QObject::tr("Sine"), QObject::tr("Cosine"),
    QObject::tr("Ramp"),
    QObject::tr("Sawtooth up"), QObject::tr("Sawtooth down"),
    QObject::tr("Triangle"), QObject::tr("Square"),
    QObject::tr("Noise")
};

const QStringList Waveform::typeStatusTips =
{
    QObject::tr("A sine oscillator [-1,1]"),
    QObject::tr("A cosine oscillator [-1,1]"),
    QObject::tr("A positive ramp [0,1]"),
    QObject::tr("A sawtooth oscillator with rising edge [-1,1]"),
    QObject::tr("A sawtooth oscillator with decaying edge [-1,1]"),
    QObject::tr("A triangle oscillator [-1,1]"),
    QObject::tr("A square-wave oscillator [-1,1]"),
    QObject::tr("A continous noise function [-1,1]")
};


const QList<int> Waveform::typeAudioList =
{ T_SINE, T_COSINE, T_SAW_RISE, T_SAW_DECAY, T_TRIANGLE, T_SQUARE };

const QStringList Waveform::typeAudioIds =
{
    "sin", "cos", "saw", "sawd", "tri", "sqr"
};

const QStringList Waveform::typeAudioNames =
{
    QObject::tr("Sine"), QObject::tr("Cosine"),
    QObject::tr("Sawtooth up"), QObject::tr("Sawtooth down"),
    QObject::tr("Triangle"), QObject::tr("Square")
};

const QStringList Waveform::typeAudioStatusTips =
{
    QObject::tr("A sine oscillator [-1,1]"),
    QObject::tr("A cosine oscillator [-1,1]"),
    QObject::tr("A sawtooth oscillator with rising edge [-1,1]"),
    QObject::tr("A sawtooth oscillator with decaying edge [-1,1]"),
    QObject::tr("A triangle oscillator [-1,1]"),
    QObject::tr("A square-wave oscillator [-1,1]"),
};



bool Waveform::supportsPulseWidth(Type t)
{
    return //t == T_RAMP
        //|| t == T_SAW
           t == T_TRIANGLE
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
            return MATH::moduloSigned( t, 1.0 );

        case T_SAW_RISE:
            return -1.0 + 2.0 * MATH::moduloSigned( t, 1.0 );

        case T_SAW_DECAY:
            return 1.0 - 2.0 * MATH::moduloSigned( t, 1.0 );

        case T_TRIANGLE:
            p = MATH::moduloSigned(t, 1.0);
            return (p<0.5)? p * 4.0 - 1.0 : (1.0-p) * 4.0 - 1.0;

        case T_SQUARE:
            return (MATH::moduloSigned( t, 1.0 ) >= 0.5) ? -1.0 : 1.0 ;

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
            return MATH::moduloSigned( t, 1.0 );

        case T_SAW_RISE:
            return -1.0 + 2.0 * MATH::moduloSigned( t, 1.0 );

        case T_SAW_DECAY:
            return 1.0 - 2.0 * MATH::moduloSigned( t, 1.0 );

        case T_TRIANGLE:
            p = MATH::moduloSigned(t, 1.0);
            return (p<pw)? p * 2.0/pw - 1.0 : (1.0-p) * 2.0/(1.0-pw) - 1.0;

        case T_SQUARE:
            return (MATH::moduloSigned( t, 1.0 ) >= pw) ? -1.0 : 1.0 ;

        case T_NOISE:
            return noise_.noise(t);

        default:
            return 0.0;
    }

}



Double Waveform::spectralWave(
                        Double time,
                        Double numPartials,
                        Double octaveStep,
                        Double phase,
                        Double phaseShift,
                        Double amplitudeMult)
{
    time = TWO_PI * time;
    phase *= TWO_PI;
    phaseShift *= TWO_PI;

    Double sam = 0.0,
           amp = 1.0,
           oct = 1.0;

    int num = numPartials;
    for (int i=0; i<num; ++i)
    {
        sam += amp * std::sin(time * oct + phase);

        phase += phaseShift;
        amp *= amplitudeMult;
        oct += octaveStep;
    }

    Double f = numPartials - num;
    if (f>0.0)
        sam += f * amp * std::sin(time * oct + phase);

    return sam;
}

} // namespace AUDIO
} // namespace MO
