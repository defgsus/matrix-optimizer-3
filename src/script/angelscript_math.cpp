/** @file angelscript_math.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>
#include <cstring> // strstr
#include <cmath>

#include <angelscript.h>

#include <QString>

#include "angelscript_math.h"
#include "math/advanced.h"


namespace MO {

namespace {

//--------------------------------
// Registration
//-------------------------------------

static void registerAngelScript_math_native(asIScriptEngine *engine)
{
    int r;

    // constants
    {
        static float pi = PI;
        static float two_pi = TWO_PI;
        static float half_pi = HALF_PI;
        static float sqrt2 = SQRT_2;
        static float sqrt3 = std::sqrt(3.f);
        r = engine->RegisterGlobalProperty("const float PI", &pi); assert( r>=0 );
        r = engine->RegisterGlobalProperty("const float TWO_PI", &two_pi); assert( r>=0 );
        r = engine->RegisterGlobalProperty("const float TAU", &two_pi); assert( r>=0 );
        r = engine->RegisterGlobalProperty("const float HALF", &half_pi); assert( r>=0 );
        r = engine->RegisterGlobalProperty("const float SQRT2", &sqrt2); assert( r>=0 );
        r = engine->RegisterGlobalProperty("const float SQRT3", &sqrt3); assert( r>=0 );
    }

#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );

    MO__REG_FUNC("float abs(float)", MATH::advanced<float>::abs);
    MO__REG_FUNC("float floor(float)", MATH::advanced<float>::floor);
    MO__REG_FUNC("float ceil(float)", MATH::advanced<float>::ceil);
    MO__REG_FUNC("float round(float)", MATH::advanced<float>::round);
    MO__REG_FUNC("float frac(float)", MATH::advanced<float>::frac);
    MO__REG_FUNC("float clamp(float, float, float)", MATH::advanced<float>::clamp);
    MO__REG_FUNC("float quant(float, float)", MATH::advanced<float>::quant);
    MO__REG_FUNC("float mod(float, float)", MATH::advanced<float>::mod);
    MO__REG_FUNC("float smod(float, float)", MATH::advanced<float>::smod);
    MO__REG_FUNC("float min(float, float)", MATH::advanced<float>::min);
    MO__REG_FUNC("float min(float, float, float)", MATH::advanced<float>::min2);
    MO__REG_FUNC("float min(float, float, float, float)", MATH::advanced<float>::min3);
    MO__REG_FUNC("float max(float, float)", MATH::advanced<float>::max);
    MO__REG_FUNC("float max(float, float, float)", MATH::advanced<float>::max2);
    MO__REG_FUNC("float max(float, float, float, float)", MATH::advanced<float>::max3);

    MO__REG_FUNC("float sin(float)", MATH::advanced<float>::sin);
    MO__REG_FUNC("float sinh(float)", MATH::advanced<float>::sinh);
    MO__REG_FUNC("float asin(float)", MATH::advanced<float>::asin);
    MO__REG_FUNC("float asinh(float)", MATH::advanced<float>::asinh);
    MO__REG_FUNC("float cos(float)", MATH::advanced<float>::cos);
    MO__REG_FUNC("float cosh(float)", MATH::advanced<float>::cosh);
    MO__REG_FUNC("float acos(float)", MATH::advanced<float>::acos);
    MO__REG_FUNC("float acosh(float)", MATH::advanced<float>::acosh);
    MO__REG_FUNC("float tan(float)", MATH::advanced<float>::tan);
    MO__REG_FUNC("float tanh(float)", MATH::advanced<float>::tanh);
    MO__REG_FUNC("float atan(float)", MATH::advanced<float>::atan);
    MO__REG_FUNC("float atan(float, float)", MATH::advanced<float>::atan_2);
    MO__REG_FUNC("float atanh(float)", MATH::advanced<float>::atanh);
    MO__REG_FUNC("float sinc(float)", MATH::advanced<float>::sinc);
    MO__REG_FUNC("float exp(float)", MATH::advanced<float>::exp);
    MO__REG_FUNC("float log(float)", MATH::advanced<float>::log);
    MO__REG_FUNC("float log2(float)", MATH::advanced<float>::log2);
    MO__REG_FUNC("float log10(float)", MATH::advanced<float>::log10);
    MO__REG_FUNC("float pow(float, float)", MATH::advanced<float>::pow);
    MO__REG_FUNC("float sqrt(float)", MATH::advanced<float>::sqrt);
    MO__REG_FUNC("float root(float, float)", MATH::advanced<float>::root);

    MO__REG_FUNC("float logistic(float)", MATH::advanced<float>::logistic);
    MO__REG_FUNC("float erf(float)", MATH::advanced<float>::erf);
    MO__REG_FUNC("float erfc(float)", MATH::advanced<float>::erfc);
    MO__REG_FUNC("float gauss(float, float)", MATH::advanced<float>::gauss);
    MO__REG_FUNC("float gauss(float, float, float)", MATH::advanced<float>::gauss_3);
    MO__REG_FUNC("float cauchy(float, float)", MATH::advanced<float>::cauchy);
    MO__REG_FUNC("float cauchy(float, float, float)", MATH::advanced<float>::cauchy_3);

    MO__REG_FUNC("float mix(float, float, float)", MATH::advanced<float>::mix);
    MO__REG_FUNC("float smoothstep(float, float, float)", MATH::advanced<float>::smoothstep);
    MO__REG_FUNC("float smoothladder(float, float)", MATH::advanced<float>::smootherladder);
    MO__REG_FUNC("float smootherstep(float, float, float)", MATH::advanced<float>::smoothstep);
    MO__REG_FUNC("float smootherladder(float, float)", MATH::advanced<float>::smootherladder);

    MO__REG_FUNC("float beta(float)", MATH::advanced<float>::beta);
    MO__REG_FUNC("float beta(float, float)", MATH::advanced<float>::beta_2);
    MO__REG_FUNC("float beta(float, float, float)", MATH::advanced<float>::beta_3);
    MO__REG_FUNC("float beta(float, float, float, float)", MATH::advanced<float>::beta_4);

    MO__REG_FUNC("float length(float, float)", MATH::advanced<float>::mag);
    MO__REG_FUNC("float length(float, float, float)", MATH::advanced<float>::mag_3);
    MO__REG_FUNC("float length(float, float, float, float)", MATH::advanced<float>::mag_4);

    MO__REG_FUNC("float noise(float)", MATH::advanced<float>::noise);
    MO__REG_FUNC("float noise(float, float)", MATH::advanced<float>::noise_2);
    MO__REG_FUNC("float noise(float, float, float)", MATH::advanced<float>::noise_3);


    MO__REG_FUNC("int num_div(int)", MATH::advanced_int<int>::num_div);
    MO__REG_FUNC("int sum_div(int)", MATH::advanced_int<int>::sum_div);
    MO__REG_FUNC("int prod_div(int)", MATH::advanced_int<int>::prod_div);
    MO__REG_FUNC("int next_div(int x, int d)", MATH::advanced_int<int>::next_div);
    MO__REG_FUNC("bool is_prime(int)", MATH::advanced_int<int>::is_prime);
    MO__REG_FUNC("int harmonic(int, int)", MATH::advanced_int<int>::harmonic_2);
    MO__REG_FUNC("int harmonic(int, int, int)", MATH::advanced_int<int>::harmonic_3);
    MO__REG_FUNC("bool is_harmonic(int, int)", MATH::advanced_int<int>::is_harmonic_2);
    MO__REG_FUNC("bool is_harmonic(int, int, int)", MATH::advanced_int<int>::is_harmonic_3);
    MO__REG_FUNC("int gcd(int, int)", MATH::advanced_int<int>::gcd);
    MO__REG_FUNC("int congruent(int a, int b, int m)", MATH::advanced_int<int>::congruent);
    MO__REG_FUNC("int factorial(int)", MATH::advanced_int<int>::factorial);
    MO__REG_FUNC("int num_digits(int x, int base)", MATH::advanced_int<int>::num_digits);
    MO__REG_FUNC("int ulam_spiral(int x, int y)", MATH::advanced_int<int>::ulam_spiral);
    MO__REG_FUNC("int ulam_spiral(int x, int y, int w)", MATH::advanced_int<int>::ulam_spiral_width);
    MO__REG_FUNC("int tri_spiral(int x, int y)", MATH::advanced_int<int>::ulam_spiral);
    MO__REG_FUNC("int fibonacci(int x)", MATH::advanced_int<int>::fibonacci);

#undef MO__REG_FUNC

}

} // namespace

void registerAngelScript_math(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"math for Angelscript currently not supported on this platform");
    }
    else
        registerAngelScript_math_native(engine);
}

} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
