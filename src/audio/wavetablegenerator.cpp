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
    : size_         (8192),
      numPartials_  (1),
      baseOctave_   (1),
      octaveStep_   (1),
      amplitudeMult_(0.5),
      freqFac_      (TWO_PI)
{

}


} // namespace AUDIO
} // namespace MO

