/** @file anglescript_vector.cpp

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

#include "angelscript_vector.h"
#include "types/vector.h"
#include "math/constants.h"
#include "math/vector.h"
#include "math/advanced.h"


namespace MO {

namespace {

//-----------------------
// AngelScript functions
//-----------------------

/** Wraps some functions and makes them non-overloading
    to avoid nasty syntax when registering */
template <typename Vec>
struct vecfunc
{
    static void VecDefaultConstructor(Vec *self) { new(self) Vec(); }
    static void VecCopyConstructor(const Vec &other, Vec *self) { new(self) Vec(other); }
    static void VecConvConstructor(Float x, Vec *self) { new(self) Vec(x); }
    static void VecInitConstructor(Float x, Float y, Float z, Vec *self) { new(self) Vec(x,y,z); }
    static void VecListConstructor(Float *list, Vec *self) { new(self) Vec(list[0], list[1], list[2]); }

    static bool VecEqualsVec(Vec * self, const Vec& v) { return *self == v; }
    static Vec VecAddVec(Vec * self, const Vec& v) { return *self + v; }
    static Vec VecSubVec(Vec * self, const Vec& v) { return *self - v; }
    static Vec VecMulVec(Vec * self, const Vec& v) { return *self * v; }
    static Vec VecDivVec(Vec * self, const Vec& v) { return *self / v; }

    static Vec VecAddFloat(Vec * self, Float v) { return *self + v; }
    static Vec VecSubFloat(Vec * self, Float v) { return *self - v; }
    static Vec VecMulFloat(Vec * self, Float v) { return *self * v; }
    static Vec VecDivFloat(Vec * self, Float v) { return *self / v; }

    static Vec VecRotated(Vec * self, const Vec& a, Float deg) { return MATH::rotate(*self, a, deg); }
    static Vec VecRotatedX(Vec * self, Float deg) { return MATH::rotateX(*self, deg); }
    static Vec VecRotatedY(Vec * self, Float deg) { return MATH::rotateX(*self, deg); }
    static Vec VecRotatedZ(Vec * self, Float deg) { return MATH::rotateX(*self, deg); }

    // -- nonmembers --
    static Vec rotate(const Vec& v, const Vec& a, Float deg) { return MATH::rotate(v, a, deg); }
    static Vec rotateX(const Vec& v, Float deg) { return MATH::rotateX(v, deg); }
    static Vec rotateY(const Vec& v, Float deg) { return MATH::rotateY(v, deg); }
    static Vec rotateZ(const Vec& v, Float deg) { return MATH::rotateZ(v, deg); }

    static Vec normalize(const Vec& v) { return MATH::normalize_safe(v); }
    static Vec cross(const Vec& v, const Vec& n) { return glm::cross(v, n); }
    static Vec reflect(const Vec& v, const Vec& n) { return glm::reflect(v, n); }
    static Vec refract(const Vec& v, const Vec& n, Float eta) { return glm::refract(v, n, eta); }

    static Vec min_vv(const Vec& v, const Vec& n) { return glm::min(v, n); }
    static Vec min_vf(const Vec& v, Float f) { return glm::min(v, f); }
    static Vec min_fv(Float f, const Vec& v) { return glm::min(v, f); }
    static Vec max_vv(const Vec& v, const Vec& n) { return glm::max(v, n); }
    static Vec max_vf(const Vec& v, Float f) { return glm::max(v, f); }
    static Vec max_fv(Float f, const Vec& v) { return glm::max(v, f); }
    static Vec clamp(const Vec& v, float mi, float ma) { return glm::clamp(v, mi, ma); }
    static Vec abs(const Vec& v) { return glm::abs(v); }

    static Float length(const Vec& v) { return glm::length(v); }
    static Float distance(const Vec& v, const Vec& n) { return glm::distance(v, n); }
    static Float dot(const Vec& a, const Vec& b) { return glm::dot(a, b); }

    static Float noise(const Vec3& v) { return MATH::advanced<float>::noise_3(v.x, v.y, v.z); }
};

//--------------------------------
// Registration
//-------------------------------------

static void registerAngelScript_vector_native(asIScriptEngine *engine)
{
    int r;

    // Register the type
    r = engine->RegisterObjectType("vec3", sizeof(Vec3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS); assert( r >= 0 );

    // Register the object properties
    r = engine->RegisterObjectProperty("vec3", "float x", asOFFSET(Vec3, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec3", "float y", asOFFSET(Vec3, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec3", "float z", asOFFSET(Vec3, z)); assert( r >= 0 );

    // Register the constructors
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,      "void f()",                             asFUNCTION(vecfunc<Vec3>::VecDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,      "void f(const vec3 &in)",               asFUNCTION(vecfunc<Vec3>::VecCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,      "void f(float)",                        asFUNCTION(vecfunc<Vec3>::VecConvConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,      "void f(float, float, float)",          asFUNCTION(vecfunc<Vec3>::VecInitConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_LIST_CONSTRUCT, "void f(const int &in) {float, float, float}", asFUNCTION(vecfunc<Vec3>::VecListConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

    // Register the operator overloads
    r = engine->RegisterObjectMethod("vec3", "vec3 &opAddAssign(const vec3 &in)", asMETHODPR(Vec3, operator+=, (const Vec3 &), Vec3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 &opSubAssign(const vec3 &in)", asMETHODPR(Vec3, operator-=, (const Vec3 &), Vec3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 &opMulAssign(const vec3 &in)", asMETHODPR(Vec3, operator*=, (const Vec3 &), Vec3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 &opDivAssign(const vec3 &in)", asMETHODPR(Vec3, operator/=, (const Vec3 &), Vec3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "bool opEquals(const vec3 &in) const", asFUNCTION(vecfunc<Vec3>::VecEqualsVec), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    r = engine->RegisterObjectMethod("vec3", "vec3 opAdd(const vec3 &in) const", asFUNCTION(vecfunc<Vec3>::VecAddVec), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opSub(const vec3 &in) const", asFUNCTION(vecfunc<Vec3>::VecSubVec), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opMul(const vec3 &in) const", asFUNCTION(vecfunc<Vec3>::VecMulVec), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opDiv(const vec3 &in) const", asFUNCTION(vecfunc<Vec3>::VecDivVec), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    r = engine->RegisterObjectMethod("vec3", "vec3 opAdd(float) const", asFUNCTION(vecfunc<Vec3>::VecAddFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opSub(float) const", asFUNCTION(vecfunc<Vec3>::VecSubFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opMul(float) const", asFUNCTION(vecfunc<Vec3>::VecMulFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opDiv(float) const", asFUNCTION(vecfunc<Vec3>::VecDivFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // Register the object methods
#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod("vec3", decl__, asFUNCTION(name__), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    MO__REG_METHOD("vec3 rotated(const vec3 &in, float)", vecfunc<Vec3>::VecRotated);
    MO__REG_METHOD("vec3 rotatedX(float)", vecfunc<Vec3>::VecRotatedX);
    MO__REG_METHOD("vec3 rotatedY(float)", vecfunc<Vec3>::VecRotatedY);
    MO__REG_METHOD("vec3 rotatedZ(float)", vecfunc<Vec3>::VecRotatedZ);

    // non-member object functions
#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );

    MO__REG_FUNC("vec3 rotate(const vec3 &in, const vec3 &in, float)", vecfunc<Vec3>::rotate);
    MO__REG_FUNC("vec3 rotateX(const vec3 &in, float)", vecfunc<Vec3>::rotateX);
    MO__REG_FUNC("vec3 rotateY(const vec3 &in, float)", vecfunc<Vec3>::rotateY);
    MO__REG_FUNC("vec3 rotateZ(const vec3 &in, float)", vecfunc<Vec3>::rotateZ);
    MO__REG_FUNC("vec3 normalize(const vec3 &in)", vecfunc<Vec3>::normalize);
    MO__REG_FUNC("vec3 cross(const vec3 &in)", vecfunc<Vec3>::cross);
    MO__REG_FUNC("vec3 reflect(const vec3 &in, const vec3 &in)", vecfunc<Vec3>::reflect);
    MO__REG_FUNC("vec3 refract(const vec3 &in, const vec3 &in, float)", vecfunc<Vec3>::refract);

    MO__REG_FUNC("vec3 abs(const vec3 &in)", vecfunc<Vec3>::abs);
    MO__REG_FUNC("vec3 min(const vec3 &in, const vec3 &in)", vecfunc<Vec3>::min_vv);
    MO__REG_FUNC("vec3 min(const vec3 &in, float)", vecfunc<Vec3>::min_vf);
    MO__REG_FUNC("vec3 min(float, const vec3 &in)", vecfunc<Vec3>::min_fv);
    MO__REG_FUNC("vec3 max(const vec3 &in, const vec3 &in)", vecfunc<Vec3>::max_vv);
    MO__REG_FUNC("vec3 max(const vec3 &in, float)", vecfunc<Vec3>::max_vf);
    MO__REG_FUNC("vec3 max(float, const vec3 &in)", vecfunc<Vec3>::max_fv);
    MO__REG_FUNC("vec3 clamp(const vec3 &in, float, float)", vecfunc<Vec3>::clamp);



    MO__REG_FUNC("float dot(const vec3 &in)", vecfunc<Vec3>::dot);
    MO__REG_FUNC("float length(const vec3 &in)", vecfunc<Vec3>::length);
    MO__REG_FUNC("float distance(const vec3 &in, const vec3 &out)", vecfunc<Vec3>::length);
    MO__REG_FUNC("float noise(const vec3 &in)", vecfunc<Vec3>::noise);

#undef MO__REG_FUNC

//    r = engine->RegisterObjectMethod("vec3", "float abs() const", asMETHOD(Vec3,length), asCALL_THISCALL); assert( r >= 0 );

    // Register the swizzle operators
//    r = engine->RegisterObjectMethod("vec3", "vec3 get_ri() const", asMETHOD(Vec3, get_ri), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "vec3 get_ir() const", asMETHOD(Vec3, get_ir), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "void set_ri(const Vec3 &in)", asMETHOD(Vec3, set_ri), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "void set_ir(const Vec3 &in)", asMETHOD(Vec3, set_ir), asCALL_THISCALL); assert( r >= 0 );
}


} // namespace

void registerAngelScript_vector(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"vector for Angelscript currently not supported on this platform");
    }
    else
        registerAngelScript_vector_native(engine);
}

} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
