/** @file angelscript_math.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_SCRIPT_ANGELSCRIPT_MATH_H
#define MOSRC_SCRIPT_ANGELSCRIPT_MATH_H

#include <random>

#include "types/vector.h"

class asIScriptEngine;

namespace MO {

/** A ref counted rng for AngelScript */
class RandomAS
{
    std::mt19937 twister;
    int ref;

    RandomAS(const RandomAS&);
    void operator = (RandomAS&);

public:

    RandomAS(uint32_t seed = 0) : twister(seed), ref(1) { }

    // --- factory ---
    static RandomAS * factory() { return new RandomAS(); }
    static RandomAS * factorySeed(uint32_t s) { return new RandomAS(s); }
    void addRef() { ++ref; }
    void releaseRef() { if (--ref == 0) delete this; }

    // --- interface ---

    void setSeed(uint32_t s) { twister.seed(s); }

    // [0,1]
    //float get() { return *(float*)&(twister() >> 9) | 0x3f800000); }
    float get() { return float(twister()) / twister.max(); }
    float getRange(float r) { return float(twister()) / twister.max() * r; }
    float getMinMax(float mi, float ma) { return mi + float(twister()) / twister.max() * (ma - mi); }

    Vec2 getVec2() { return Vec2(get(), get()); }
    Vec2 getVec2Range(float r) { return Vec2(getRange(r), getRange(r)); }
    Vec2 getVec2MinMax(float mi, float ma) { return Vec2(getMinMax(mi,ma), getMinMax(mi,ma)); }
    Vec3 getVec3() { return Vec3(get(), get(), get()); }
    Vec3 getVec3Range(float r) { return Vec3(getRange(r), getRange(r), getRange(r)); }
    Vec3 getVec3MinMax(float mi, float ma) { return Vec3(getMinMax(mi,ma), getMinMax(mi,ma), getMinMax(mi,ma)); }
    Vec4 getVec4() { return Vec4(get(), get(), get(), get()); }
    Vec4 getVec4Range(float r) { return Vec4(getRange(r), getRange(r), getRange(r), getRange(r)); }
    Vec4 getVec4MinMax(float mi, float ma) { return Vec4(getMinMax(mi,ma), getMinMax(mi,ma), getMinMax(mi,ma), getMinMax(mi,ma)); }
};


void registerAngelScript_math(asIScriptEngine *engine);

} // namespace MO


#endif // MOSRC_SCRIPT_ANGELSCRIPT_MATH_H

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
