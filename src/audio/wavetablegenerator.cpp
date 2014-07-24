/** @file wavetablegenerator.cpp

    @brief Wavetable generator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#include "wavetablegenerator.h"
#include "math/constants.h"

namespace MO {
namespace AUDIO {

WavetableGenerator::WavetableGenerator()
    : size_         (1<<15),
      numPartials_  (4),
      baseOctave_   (1),
      octaveStep_   (1),
      basePhase_    (0.0),
      phaseShift_   (0.0),
      amplitudeMult_(0.5),
      freqFac_      (TWO_PI),
      phaseFac_     (0.0),
      phaseShiftFac_(0.0)
{

}


void WavetableGenerator::setBaseOctave(uint oct)
{
    baseOctave_ = oct;
    freqFac_ = TWO_PI * oct;
}

void WavetableGenerator::setBasePhase(Double phase)
{
    basePhase_ = phase;
    phaseFac_ = TWO_PI * phase / 360.0;
}

void WavetableGenerator::setPhaseShift(Double shift)
{
    phaseShift_ = shift;
    phaseShiftFac_ = TWO_PI * shift / 360.0;
}


} // namespace AUDIO
} // namespace MO

