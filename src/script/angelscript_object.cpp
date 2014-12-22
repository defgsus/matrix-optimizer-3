/** @file angelscript_object.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 22.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>
#include <cstring> // strstr
#include <sstream>

#include "angelscript_object.h"
#include "script/angelscript.h"
#include "object/object.h"
#include "io/log.h"


namespace MO {

namespace {

//-----------------------
// AngelScript functions
//-----------------------

namespace
{
    AngelScriptString objectToString(Object * o)
    {
        if (o == 0)
            return "null";
        std::stringstream s;
        s << o;
        return s.str();
    }

    AngelScriptString objectName(Object * o) { return o->name().toUtf8().constData(); }
    AngelScriptString objectId(Object * o) { return o->idName().toUtf8().constData(); }

    int objectChildrenCount(Object * o) { return o->childObjects().size(); }

    Object * objectChildren(Object* o, int i) { return o->childObjects()[i]; }
}

//--------------------------------
// Registration
//-------------------------------------

static void registerAngelScript_object_native(asIScriptEngine *engine)
{
    int r;

    // register the type
    r = engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOCOUNT); assert( r >= 0 );

    // --------------- the object methods ----------------------

#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod("Object", decl__, asFUNCTION(name__), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // strings
    MO__REG_METHOD("string opImplConv() const", objectToString);
    MO__REG_METHOD("string name() const", objectName);
    MO__REG_METHOD("string id() const", objectId);

    // other
    MO__REG_METHOD("int childrenCount() const", objectChildrenCount);
    MO__REG_METHOD("Object@ children(int) const", objectChildren);


#undef MO__REG_METHOD


    // ------------ non-member object functions ----------------

#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );

//    MO__REG_FUNC("vec3 rotate(const vec3 &in, const vec3 &in, float)", vecfunc<Vec3>::rotate);


#undef MO__REG_FUNC
}




} // namespace ----------------------------------------------


void registerAngelScript_object(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"Object type for Angelscript currently not supported on this platform");
    }
    else
        registerAngelScript_object_native(engine);
}

void registerAngelScript_rootObject(asIScriptEngine *engine, Scene* root)
{
    static void * ptr;
    ptr = root;
    MO_DEBUG("register " << ptr);
    int r = 0; engine->RegisterGlobalProperty("Object@ Root", &ptr); assert( r >= 0 );
}




} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
