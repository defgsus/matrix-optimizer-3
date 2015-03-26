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

namespace native {



static void register_rnd(asIScriptEngine *engine)
{
    int r;
    Q_UNUSED(r)

    r = engine->RegisterObjectType("Random", 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------

    r = engine->RegisterObjectBehaviour("Random", asBEHAVE_FACTORY,
        "Random@ f()", asFUNCTION(RandomAS::factory), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Random", asBEHAVE_FACTORY,
        "Random@ f(uint32 seed)", asFUNCTION(RandomAS::factorySeed), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Random", asBEHAVE_ADDREF,
        "void f()", asMETHOD(RandomAS,addRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Random", asBEHAVE_RELEASE,
        "void f()", asMETHOD(RandomAS,releaseRef), asCALL_THISCALL); assert( r >= 0 );

    // --- interface ---

#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod("Random", decl__, asMETHOD(RandomAS,name__), asCALL_THISCALL); assert( r >= 0 );

    //MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("void setSeed(uint32)", setSeed);
    MO__REG_METHOD("float opCall() const", get);
    MO__REG_METHOD("float opCall(float range) const", getRange);
    MO__REG_METHOD("float opCall(float min, float max) const", getMinMax);
    MO__REG_METHOD("uint32 get_uint() const", getUInt);
    MO__REG_METHOD("uint32 get_uint(uint32 range) const", getUIntRange);
    MO__REG_METHOD("uint32 get_uint(uint32 min, uint32 max) const", getUIntMinMax);
    MO__REG_METHOD("int32 get_int() const", getInt);
    MO__REG_METHOD("int32 get_int(uint32 range) const", getIntRange);
    MO__REG_METHOD("int32 get_int(int32 min, int32 max) const", getIntMinMax);
    MO__REG_METHOD("vec2 get_vec2() const", getVec3);
    MO__REG_METHOD("vec2 get_vec2(float range) const", getVec3Range);
    MO__REG_METHOD("vec2 get_vec2(float min, float max) const", getVec3MinMax);
    MO__REG_METHOD("vec3 get_vec3() const", getVec3);
    MO__REG_METHOD("vec3 get_vec3(float range) const", getVec3Range);
    MO__REG_METHOD("vec3 get_vec3(float min, float max) const", getVec3MinMax);
    MO__REG_METHOD("vec4 get_vec4() const", getVec3);
    MO__REG_METHOD("vec4 get_vec4(float range) const", getVec3Range);
    MO__REG_METHOD("vec4 get_vec4(float min, float max) const", getVec3MinMax);
    MO__REG_METHOD("vec2 vec2() const", getVec3);
    MO__REG_METHOD("vec2 vec2(float range) const", getVec3Range);
    MO__REG_METHOD("vec2 vec2(float min, float max) const", getVec3MinMax);
    MO__REG_METHOD("vec3 vec3() const", getVec3);
    MO__REG_METHOD("vec3 vec3(float range) const", getVec3Range);
    MO__REG_METHOD("vec3 vec3(float min, float max) const", getVec3MinMax);
    MO__REG_METHOD("vec4 vec4() const", getVec3);
    MO__REG_METHOD("vec4 vec4(float range) const", getVec3Range);
    MO__REG_METHOD("vec4 vec4(float min, float max) const", getVec3MinMax);
}


#define MO__STR(decl__) QString(decl__).replace("%1", floattyp).replace("%2", suffix).toUtf8().constData()
#define MO__REG_PROPERTY(decl__, ptr__) \
    r = engine->RegisterGlobalProperty(MO__STR(decl__), ptr__); assert( r>=0 );
#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(MO__STR(decl__), asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );

template <typename F>
static void register_math_funcs(asIScriptEngine *engine, const char * floattyp, const char * suffix)
{
    int r; Q_UNUSED(r);

    // constants
    {
        static F pi = PI;
        static F two_pi = TWO_PI;
        static F half_pi = HALF_PI;
        static F sqrt2 = SQRT_2;
        static F sqrt3 = std::sqrt(F(3));
        static F sqrt4 = std::sqrt(F(4));
        static F sqrt5 = std::sqrt(F(5));
        static F sqrt6 = std::sqrt(F(6));
        MO__REG_PROPERTY("const %1 PI%2", &pi);
        MO__REG_PROPERTY("const %1 TWO_PI%2", &two_pi);
        MO__REG_PROPERTY("const %1 TAU%2", &two_pi);
        MO__REG_PROPERTY("const %1 HALF_PI%2", &half_pi);
        MO__REG_PROPERTY("const %1 SQRT2%2", &sqrt2);
        MO__REG_PROPERTY("const %1 SQRT3%2", &sqrt3);
        MO__REG_PROPERTY("const %1 SQRT4%2", &sqrt4);
        MO__REG_PROPERTY("const %1 SQRT5%2", &sqrt5);
        MO__REG_PROPERTY("const %1 SQRT6%2", &sqrt6);
    }

    MO__REG_FUNC("%1 abs(%1)", MATH::advanced<F>::abs);
    MO__REG_FUNC("%1 floor(%1)", MATH::advanced<F>::floor);
    MO__REG_FUNC("%1 ceil(%1)", MATH::advanced<F>::ceil);
    MO__REG_FUNC("%1 round(%1)", MATH::advanced<F>::round);
    MO__REG_FUNC("%1 frac(%1)", MATH::advanced<F>::frac);
    MO__REG_FUNC("%1 clamp(%1, %1, %1)", MATH::advanced<F>::clamp);
    MO__REG_FUNC("%1 quant(%1, %1)", MATH::advanced<F>::quant);
    MO__REG_FUNC("%1 mod(%1, %1)", MATH::advanced<F>::mod);
    MO__REG_FUNC("%1 smod(%1, %1)", MATH::advanced<F>::smod);
    MO__REG_FUNC("%1 min(%1, %1)", MATH::advanced<F>::min);
    MO__REG_FUNC("%1 min(%1, %1, %1)", MATH::advanced<F>::min3);
    MO__REG_FUNC("%1 min(%1, %1, %1, %1)", MATH::advanced<F>::min4);
    MO__REG_FUNC("%1 min(%1, %1, %1, %1, %1)", MATH::advanced<F>::min5);
    MO__REG_FUNC("%1 max(%1, %1)", MATH::advanced<F>::max);
    MO__REG_FUNC("%1 max(%1, %1, %1)", MATH::advanced<F>::max3);
    MO__REG_FUNC("%1 max(%1, %1, %1, %1)", MATH::advanced<F>::max4);
    MO__REG_FUNC("%1 max(%1, %1, %1, %1, %1)", MATH::advanced<F>::max5);

    MO__REG_FUNC("%1 sin(%1)", MATH::advanced<F>::sin);
    MO__REG_FUNC("%1 sinh(%1)", MATH::advanced<F>::sinh);
    MO__REG_FUNC("%1 asin(%1)", MATH::advanced<F>::asin);
    MO__REG_FUNC("%1 asinh(%1)", MATH::advanced<F>::asinh);
    MO__REG_FUNC("%1 cos(%1)", MATH::advanced<F>::cos);
    MO__REG_FUNC("%1 cosh(%1)", MATH::advanced<F>::cosh);
    MO__REG_FUNC("%1 acos(%1)", MATH::advanced<F>::acos);
    MO__REG_FUNC("%1 acosh(%1)", MATH::advanced<F>::acosh);
    MO__REG_FUNC("%1 tan(%1)", MATH::advanced<F>::tan);
    MO__REG_FUNC("%1 tanh(%1)", MATH::advanced<F>::tanh);
    MO__REG_FUNC("%1 atan(%1)", MATH::advanced<F>::atan);
    MO__REG_FUNC("%1 atan(%1, %1)", MATH::advanced<F>::atan_2);
    MO__REG_FUNC("%1 atanh(%1)", MATH::advanced<F>::atanh);
    MO__REG_FUNC("%1 sinc(%1)", MATH::advanced<F>::sinc);
    MO__REG_FUNC("%1 exp(%1)", MATH::advanced<F>::exp);
    MO__REG_FUNC("%1 log(%1)", MATH::advanced<F>::log);
    MO__REG_FUNC("%1 log2(%1)", MATH::advanced<F>::log2);
    MO__REG_FUNC("%1 log10(%1)", MATH::advanced<F>::log10);
    MO__REG_FUNC("%1 pow(%1 v, %1 exponent)", MATH::advanced<F>::pow);
    MO__REG_FUNC("%1 sqrt(%1)", MATH::advanced<F>::sqrt);
    MO__REG_FUNC("%1 root(%1 v, %1 root_number)", MATH::advanced<F>::root);

    MO__REG_FUNC("%1 logistic(%1)", MATH::advanced<F>::logistic);
    MO__REG_FUNC("%1 erf(%1)", MATH::advanced<F>::erf);
    MO__REG_FUNC("%1 erfc(%1)", MATH::advanced<F>::erfc);
    MO__REG_FUNC("%1 gauss(%1 x, %1 deviation)", MATH::advanced<F>::gauss);
    MO__REG_FUNC("%1 gauss(%1 x, %1 deviation, %1 center)", MATH::advanced<F>::gauss_3);
    MO__REG_FUNC("%1 cauchy(%1 x, %1 deviation)", MATH::advanced<F>::cauchy);
    MO__REG_FUNC("%1 cauchy(%1 x, %1 deviation, %1 center)", MATH::advanced<F>::cauchy_3);

    MO__REG_FUNC("%1 mix(%1 v0, %1 v1, %1 mix_val)", MATH::advanced<F>::mix);
    MO__REG_FUNC("%1 step(%1 edge0, %1 edge1, %1 v)", MATH::advanced<F>::smoothstep);
    MO__REG_FUNC("%1 smoothstep(%1 edge0, %1 edge1, %1 v)", MATH::advanced<F>::smoothstep);
    MO__REG_FUNC("%1 smoothladder(%1 v, %1 step)", MATH::advanced<F>::smoothladder);
    MO__REG_FUNC("%1 smootherstep(%1 edge0, %1 edge1, %1 v)", MATH::advanced<F>::smoothstep);
    MO__REG_FUNC("%1 smootherladder(%1 v, %1 step)", MATH::advanced<F>::smootherladder);

    MO__REG_FUNC("%1 beta(%1 x)", MATH::advanced<F>::beta);
    MO__REG_FUNC("%1 beta(%1 x, %1 y)", MATH::advanced<F>::beta_2);
    MO__REG_FUNC("%1 beta(%1 x, %1 y, %1 z)", MATH::advanced<F>::beta_3);
    MO__REG_FUNC("%1 beta(%1 x, %1 y, %1 z, %1 w)", MATH::advanced<F>::beta_4);

    MO__REG_FUNC("%1 length(%1 x, %1 y)", MATH::advanced<F>::mag);
    MO__REG_FUNC("%1 length(%1 x, %1 y, %1 z)", MATH::advanced<F>::mag_3);
    MO__REG_FUNC("%1 length(%1 x, %1 y, %1 z, %1 w)", MATH::advanced<F>::mag_4);

    MO__REG_FUNC("%1 note2freq(%1 note)", MATH::advanced<F>::note2freq_1);

    MO__REG_FUNC("%1 noise(%1 x)", MATH::advanced<F>::noise);
    MO__REG_FUNC("%1 noise(%1 x, %1 y)", MATH::advanced<F>::noise_2);
    MO__REG_FUNC("%1 noise(%1 x, %1 y, %1 z)", MATH::advanced<F>::noise_3);
    MO__REG_FUNC("%1 voronoi(%1 x, %1 y)", MATH::advanced<F>::voronoi_2);
    MO__REG_FUNC("%1 voronoi(%1 x, %1 y, %1 z)", MATH::advanced<F>::voronoi_3);
    MO__REG_FUNC("%1 svoronoi(%1 x, %1 y, %1 smooth = 32)", MATH::advanced<F>::svoronoi_2);
    MO__REG_FUNC("%1 svoronoi(%1 x, %1 y, %1 z, %1 smooth = 32)", MATH::advanced<F>::svoronoi_3);

    MO__REG_FUNC("%1 mandel(%1 real, %1 imag)", (MATH::fractal<F,int>::mandel));
    MO__REG_FUNC("%1 mandel(%1 real, %1 imag, uint max_iterations)", (MATH::fractal<F,uint>::mandel_3));
    MO__REG_FUNC("uint mandeli(%1 real, %1 imag)", (MATH::fractal<F,uint>::mandeli));
    MO__REG_FUNC("uint mandeli(%1 real, %1 imag, uint max_iterations)", (MATH::fractal<F,uint>::mandeli_3));
    MO__REG_FUNC("%1 julia(%1 real_start, %1 imag_start, %1 real, %1 imag)", (MATH::fractal<F,uint>::julia));
    MO__REG_FUNC("uint juliai(%1 real_start, %1 imag_start, %1 real, %1 imag)", (MATH::fractal<F,uint>::juliai));
    MO__REG_FUNC("%1 duckball(%1 real, %1 imag, %1 real_start = 0.5, %1 imag_start = 0.5, float bailout = 10)", (MATH::fractal<F,uint>::duckball));
}

#undef MO__REG_FUNC
#undef MO__REG_PROPERTY
#undef MO__STR

#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );


static void register_math_int_funcs(asIScriptEngine *engine)
{
    int r; Q_UNUSED(r);

    // ------- integer functions ----------

    MO__REG_FUNC("bool is_prime(int)", MATH::advanced_int<int>::is_prime);
    MO__REG_FUNC("bool is_harmonic(int x, int y)", MATH::advanced_int<int>::is_harmonic_2);
    MO__REG_FUNC("bool is_harmonic(int x, int y, int z)", MATH::advanced_int<int>::is_harmonic_3);
    MO__REG_FUNC("bool is_congruent(int a, int b, int m)", MATH::advanced_int<int>::is_congruent);
    MO__REG_FUNC("int sign(int x)", MATH::advanced_int<int>::sign);
    MO__REG_FUNC("int quant(int x, int step)", MATH::advanced_int<int>::quant);
    MO__REG_FUNC("int num_div(int)", MATH::advanced_int<int>::num_div);
    MO__REG_FUNC("int sum_div(int)", MATH::advanced_int<int>::sum_div);
    MO__REG_FUNC("int prod_div(int)", MATH::advanced_int<int>::prod_div);
    MO__REG_FUNC("int next_div(int x, int d)", MATH::advanced_int<int>::next_div);
    MO__REG_FUNC("int gcd(int, int)", MATH::advanced_int<int>::gcd);
    MO__REG_FUNC("int harmonic(int x, int y)", MATH::advanced_int<int>::harmonic_2);
    MO__REG_FUNC("int harmonic(int x, int y, int z)", MATH::advanced_int<int>::harmonic_3);
    MO__REG_FUNC("int factorial(int)", MATH::advanced_int<int>::factorial);
    MO__REG_FUNC("int num_digits(int x, int base = 10)", MATH::advanced_int<int>::num_digits);
    MO__REG_FUNC("int ulam_spiral(int x, int y)", MATH::advanced_int<int>::ulam_spiral);
    MO__REG_FUNC("int ulam_spiral(int x, int y, int width)", MATH::advanced_int<int>::ulam_spiral_width);
    MO__REG_FUNC("int tri_spiral(int x, int y)", MATH::advanced_int<int>::ulam_spiral);
    MO__REG_FUNC("int fibonacci(int num)", MATH::advanced_int<int>::fibonacci);

    MO__REG_FUNC("float noise(int x)", MATH::advanced<float>::noisei);
    MO__REG_FUNC("float noise(int x, int y)", MATH::advanced<float>::noisei_2);
    MO__REG_FUNC("float noise(int x, int y, int z)", MATH::advanced<float>::noisei_3);

}



} // namespace native

} // namespace

void registerAngelScript_math(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"math for Angelscript currently not supported on this platform");
    }
    else
    {
        native::register_math_funcs<float>(engine, "float", "");
        // XXXthis should be optional
        // as it complicates function resolving when mixing floats and doubles in one call
        //native::register_math_funcs<double>(engine, "double", "d");

        native::register_math_int_funcs(engine);

        native::register_rnd(engine);
    }
}

} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
