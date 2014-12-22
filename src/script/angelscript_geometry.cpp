/** @file angelscript_geometry.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 22.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>
#include <cstring> // strstr
#include <sstream>

#include "angelscript_geometry.h"
#include "script/angelscript.h"
#include "object/object.h"
#include "geom/geometry.h"
#include "io/log.h"


namespace MO {

namespace {

//-----------------------
// AngelScript functions
//-----------------------

namespace
{

    static void constructGeometry(GEOM::Geometry * thisPointer)
    {
        new(thisPointer) GEOM::Geometry();
    }
/*
    static void copyConstructGeometry(const GEOM::Geometry &other, GEOM::Geometry * thisPointer)
    {
        new(thisPointer) GEOM::Geometry(other);
    }
*/
    static void destructGeometry(GEOM::Geometry * thisPointer)
    {
        thisPointer->~Geometry();
    }

    static std::string geometryToString(GEOM::Geometry * g) { std::stringstream s; s << "Geometry(" << (void*)g; return s.str(); }

    static std::string geometryName(GEOM::Geometry *  ) { return "geometry"; }
    static uint geometryVertexCount(GEOM::Geometry * g) { return g->numVertices(); }
}

//--------------------------------
// Registration
//-------------------------------------

static void registerAngelScript_geometry_native(asIScriptEngine *engine)
{
    int r;

    // register the type
    r = engine->RegisterObjectType("Geometry", sizeof(GEOM::Geometry), asOBJ_VALUE | asOBJ_APP_CLASS_CD); assert( r >= 0 );

    // ----------------- constructor ---------------------------

    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(constructGeometry), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    //r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_CONSTRUCT,  "void f(const Geometry &in)",  asFUNCTION(copyConstructGeometry), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(destructGeometry),  asCALL_CDECL_OBJLAST); assert( r >= 0 );

    // --------------- the object methods ----------------------

#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod("Geometry", decl__, asFUNCTION(name__), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    MO__REG_METHOD("string name() const", geometryName);
    MO__REG_METHOD("string opImplConv() const", geometryToString);
    MO__REG_METHOD("int vertexCount() const", geometryVertexCount);


#undef MO__REG_METHOD


    // ------------ non-member object functions ----------------

#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );

//    MO__REG_FUNC("vec3 rotate(const vec3 &in, const vec3 &in, float)", vecfunc<Vec3>::rotate);


#undef MO__REG_FUNC
}




} // namespace ----------------------------------------------


void registerAngelScript_geometry(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"Geometry type for Angelscript currently not supported on this platform");
    }
    else
        registerAngelScript_geometry_native(engine);
}




} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
