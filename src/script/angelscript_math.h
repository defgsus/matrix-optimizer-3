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

    RandomAS() : twister(0), ref(1) { }

    // --- factory ---
    static RandomAS * factory() { return new RandomAS(); }
    void addRef() { ++ref; }
    void releaseRef() { if (--ref == 0) delete this; }

    // --- interface ---

    void setSeed(uint32_t s) { twister.seed(s); }

    // [0,1]
    //float get() { return *(float*)&(twister() >> 9) | 0x3f800000); }
    float get() { return float(twister()) / twister.max(); }
    float getRange(float r) { return float(twister()) / twister.max() * r; }
    float getMinMax(float mi, float ma) { return mi + float(twister()) / twister.max() * (ma - mi); }
};


void registerAngelScript_math(asIScriptEngine *engine);

} // namespace MO


#endif // MOSRC_SCRIPT_ANGELSCRIPT_MATH_H

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
