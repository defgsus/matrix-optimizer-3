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

#include "angelscript_vector.h"
#include "types/vector.h"


namespace MO {

//-----------------------
// AngelScript functions
//-----------------------

static void Vec3DefaultConstructor(Vec3 *self)
{
    new(self) Vec3();
}

static void Vec3CopyConstructor(const Vec3 &other, Vec3 *self)
{
    new(self) Vec3(other);
}

static void Vec3ConvConstructor(float x, Vec3 *self)
{
    new(self) Vec3(x);
}

static void Vec3InitConstructor(float x, float y, float z, Vec3 *self)
{
    new(self) Vec3(x,y,z);
}

static void Vec3ListConstructor(float *list, Vec3 *self)
{
    new(self) Vec3(list[0], list[1], list[2]);
}

static Vec3 Vec3AddVec3(Vec3 * self, const Vec3& v) { return *self + v; }
static Vec3 Vec3SubVec3(Vec3 * self, const Vec3& v) { return *self - v; }
static Vec3 Vec3MulVec3(Vec3 * self, const Vec3& v) { return *self * v; }
static Vec3 Vec3DivVec3(Vec3 * self, const Vec3& v) { return *self / v; }

static Vec3 Vec3AddFloat(Vec3 * self, float v) { return *self + v; }
static Vec3 Vec3SubFloat(Vec3 * self, float v) { return *self - v; }
static Vec3 Vec3MulFloat(Vec3 * self, float v) { return *self * v; }
static Vec3 Vec3DivFloat(Vec3 * self, float v) { return *self / v; }

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
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,      "void f()",                             asFUNCTION(Vec3DefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,      "void f(const vec3 &in)",               asFUNCTION(Vec3CopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,      "void f(float)",                        asFUNCTION(Vec3ConvConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,      "void f(float, float, float)",          asFUNCTION(Vec3InitConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_LIST_CONSTRUCT, "void f(const int &in) {float, float, float}", asFUNCTION(Vec3ListConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

    // Register the operator overloads
    r = engine->RegisterObjectMethod("vec3", "vec3 &opAddAssign(const vec3 &in)", asMETHODPR(Vec3, operator+=, (const Vec3 &), Vec3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 &opSubAssign(const vec3 &in)", asMETHODPR(Vec3, operator-=, (const Vec3 &), Vec3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 &opMulAssign(const vec3 &in)", asMETHODPR(Vec3, operator*=, (const Vec3 &), Vec3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 &opDivAssign(const vec3 &in)", asMETHODPR(Vec3, operator/=, (const Vec3 &), Vec3&), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "bool opEquals(const vec3 &in) const", asMETHODPR(Vec3, operator==, (const Vec3 &) const, bool), asCALL_THISCALL); assert( r >= 0 );
    //r = engine->RegisterObjectMethod("vec3", "vec3 opAdd(const vec3 &in) const", asFUNCTIONPR(glm::detail::operator+, (const Vec3 &), Vec3), asCALL_CDECL); assert( r >= 0 );

    r = engine->RegisterObjectMethod("vec3", "vec3 opAdd(const vec3 &in) const", asFUNCTION(Vec3AddVec3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opSub(const vec3 &in) const", asFUNCTION(Vec3SubVec3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opMul(const vec3 &in) const", asFUNCTION(Vec3MulVec3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opDiv(const vec3 &in) const", asFUNCTION(Vec3DivVec3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    r = engine->RegisterObjectMethod("vec3", "vec3 opAdd(float) const", asFUNCTION(Vec3AddFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opSub(float) const", asFUNCTION(Vec3SubFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opMul(float) const", asFUNCTION(Vec3MulFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod("vec3", "vec3 opDiv(float) const", asFUNCTION(Vec3DivFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "vec3 opSub(const vec3 &in) const", asMETHODPR(Vec3, operator-, (const Vec3 &) const, Vec3), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "vec3 opMul(const vec3 &in) const", asMETHODPR(Vec3, operator*, (const Vec3 &) const, Vec3), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "vec3 opDiv(const vec3 &in) const", asMETHODPR(Vec3, operator/, (const Vec3 &) const, Vec3), asCALL_THISCALL); assert( r >= 0 );

    // Register the object methods
//    r = engine->RegisterObjectMethod("vec3", "float abs() const", asMETHOD(Vec3,length), asCALL_THISCALL); assert( r >= 0 );

    // Register the swizzle operators
//    r = engine->RegisterObjectMethod("vec3", "vec3 get_ri() const", asMETHOD(Vec3, get_ri), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "vec3 get_ir() const", asMETHOD(Vec3, get_ir), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "void set_ri(const Vec3 &in)", asMETHOD(Vec3, set_ri), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod("vec3", "void set_ir(const Vec3 &in)", asMETHOD(Vec3, set_ir), asCALL_THISCALL); assert( r >= 0 );
}


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
