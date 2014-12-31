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
#include "angelscript_timeline.h"
#include "script/angelscript.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/sequencefloat.h"
#include "object/util/objecteditor.h"
#include "io/log.h"


namespace MO {

namespace {

//------------------------------
// AngelScript functions wrapper
//------------------------------

struct objectfunc
{
    // ------ getter ------

    static StringAS toString(Object * o)
    {
        if (o == 0)
            return "null";
        std::stringstream s;
        s << o;
        return s.str();
    }

    static StringAS name(Object * o) { return toStringAS(o->name()); }
    static StringAS id(Object * o) { return toStringAS(o->idName()); }

    static int childrenCount(Object * o) { return o->childObjects().size(); }
    static Object * children(Object* o, int i) { return o->childObjects()[i]; }

    static Object * findName(Object * o, const StringAS& n)
    {
        if (o->name().toStdString() == n)
            return o;
        for (auto c : o->childObjects())
            if (auto f = findName(c, n))
                return f;
        return 0;
    }

    // ------ setter ------

    static ObjectEditor * editor(Object * o) { auto s = o->sceneObject(); return s ? s->editor() : 0; }

    static void setName(Object * o, const StringAS& n) { auto e = editor(o); if (e) e->setObjectName(o, QString::fromUtf8(n.c_str())); }

    static Timeline1AS * timeline(Object * o)
    {
        if (auto seq = qobject_cast<SequenceFloat*>(o))
            if (seq->timeline())
                return timeline_to_angelscript(*seq->timeline());
        return 0;
    }

    static void setTimeline(Object * o, Timeline1AS * tl)
    {
        if (tl)
        if (auto seq = qobject_cast<SequenceFloat*>(o))
        {
            seq->setTimeline(get_timeline(tl));
            // gui signal
            if (auto e = editor(seq))
                emit e->sequenceChanged(seq);
        }
    }
};


//--------------------------------
// Registration
//-------------------------------------

namespace native {

#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod("Object", decl__, asFUNCTION(name__), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );


static void register_object(asIScriptEngine *engine)
{
    int r;

    // register the type
    r = engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOCOUNT); assert( r >= 0 );

    // --------------- the object methods ----------------------

    // strings
    MO__REG_METHOD("string opImplConv() const", objectfunc::toString);
    MO__REG_METHOD("string name() const", objectfunc::name);
    MO__REG_METHOD("string id() const", objectfunc::id);

    // getter
    MO__REG_METHOD("int childrenCount() const", objectfunc::childrenCount);
    MO__REG_METHOD("Object@ children(int)", objectfunc::children);
    MO__REG_METHOD("const Object@ children(int) const", objectfunc::children);
    MO__REG_METHOD("Object@ find(const string &in name)", objectfunc::findName);
    MO__REG_METHOD("const Object@ find(const string &in name) const", objectfunc::findName);

    MO__REG_METHOD("Timeline1@ getTimeline() const", objectfunc::timeline);

    // setter
    MO__REG_METHOD("void setName(const string &in)", objectfunc::setName);
    MO__REG_METHOD("void setTimeline(Timeline1@)", objectfunc::setTimeline);



    // ------------ non-member object functions ----------------


//    MO__REG_FUNC("", );


}


#undef MO__REG_FUNC
#undef MO__REG_METHOD

} // namespace native

} // namespace ----------------------------------------------


void registerAngelScript_object(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"Object type for Angelscript currently not supported on this platform");
    }
    else
        native::register_object(engine);
}

void registerAngelScript_rootObject(asIScriptEngine *engine, Scene* root, bool writeable)
{
    // XXX BIG TODO! remove this static!!
    static void * ptr;
    ptr = root;
    int r = 0;
    if (writeable)
    {
        r = engine->RegisterGlobalProperty("Object@ scene", &ptr); assert( r >= 0 );
    }
    else
    {
        r = engine->RegisterGlobalProperty("const Object@ scene", &ptr); assert( r >= 0 );
    }
}

void registerAngelScript_object(asIScriptEngine *engine, Object * obj, bool writeable)
{
    static void * ptr;
    ptr = obj;
    int r = 0;
    if (writeable)
    {
        r = engine->RegisterGlobalProperty("Object@ object", &ptr); assert( r >= 0 );
    }
    else
    {
        r = engine->RegisterGlobalProperty("const Object@ object", &ptr); assert( r >= 0 );
    }
}




} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
