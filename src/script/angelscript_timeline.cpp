/** @file angelscript_timeline.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12/29/2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>
#include <cstring> // strstr
#include <string>

#include <QString>

#include "angelscript_timeline.h"
#include "angelscript.h"
#include "math/timeline1d.h"
#include "io/log.h"


#if 1
#   define MO_DEBUG_TAS(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_TAS(unused__)
#endif

namespace MO {

namespace {

class Timeline1AS
{
    MATH::Timeline1D * tl;
    int ref;
public:

    // --- factory ---

    static Timeline1AS * factory() { return new Timeline1AS; }

    void addRef() { ++ref; MO_DEBUG_TAS("Timeline1AS("<<this<<")::addRef() now = " << ref); }

    void releaseRef() { MO_DEBUG_TAS("Timeline1AS("<<this<<")::releaseRef() now = " << (ref-1)); if (--ref == 0) delete this; }

    // ----- ctor -----

    Timeline1AS()
        : tl    (new MATH::Timeline1D),
          ref   (1)
    {
        MO_DEBUG_TAS("Timeline1AS("<<this<<")::Timeline1AS()");
    }

    ~Timeline1AS()
    {
        MO_DEBUG_TAS("Timeline1AS("<<this<<")::~Timeline1AS()");
        delete tl;
    }

    // ------ interface -------

    std::string toString() const { return "Timeline()"; }
    uint count() const { return tl->size(); }
    double value(double time) const { return tl->get(time); }

    void clear() { tl->clear(); }
    void add(double time, double value) { tl->add(time, value); }
};

namespace native {

// -------------------------------- registration ------------------------------------

// replaces %1 with typ, %2 with holdtyp (const char*) and returns utf8 as const char*
#define MO__STR(str__) (QString(str__).replace("%1", typ).replace("%2", holdtyp).toUtf8().constData())
// registers a method of class Class for typ (replaces %1 in decl__ with typ)
#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod(typ, MO__STR(decl__), asMETHOD(Class,name__), asCALL_THISCALL); assert( r >= 0 );


template <class Class>
void register_timeline_tmpl(asIScriptEngine * engine, const char * typ, const char * holdtyp)
{
    int r;

    // register the type
    r = engine->RegisterObjectType(typ, 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
        MO__STR("%1@ f()"), asFUNCTION(Class::factory), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_ADDREF,
        "void f()", asMETHOD(Class,addRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_RELEASE,
        "void f()", asMETHOD(Class,releaseRef), asCALL_THISCALL); assert( r >= 0 );

    // --------------- the object methods ----------------------

    // getter
    MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("string toString() const", toString);
    MO__REG_METHOD("uint count() const", count);
    MO__REG_METHOD("%2 value(double time) const", value);

    // setter
    MO__REG_METHOD("void clear()", clear);
    MO__REG_METHOD("void add(double time, %2 value) const", add);
}

#undef MO__REG_METHOD
#undef MO__STR

} // namespace native
} // namespace



void registerAngelScript_timeline(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"timeline in AngelScript not supported on this platform");
    }
    else
    {
        native::register_timeline_tmpl<Timeline1AS>(engine, "Timeline1", "double");
    }
}



} // namespace MO

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
