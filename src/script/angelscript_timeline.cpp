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
#include <sstream>

#include <QString>

#include "angelscript_timeline.h"
#include "angelscript.h"
#include "math/timeline1d.h"
#include "types/vector.h"
#include "io/log.h"


#if 1
#   define MO_DEBUG_TAS(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_TAS(unused__)
#endif

namespace MO {

namespace {

/** Ref-counted wrapper of MATH::Timeline1D for AngelScript */
class Timeline1AS
{
    MATH::Timeline1D * tl;
    int ref;
public:

    typedef MATH::Timeline1D WrappedClass;

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

    std::string toString() const
    {
        std::stringstream s;
        s << "Timeline1(";
        const auto & data = tl->getData();
        int type = -1;
        bool first = true;
        for (auto & p : data)
        {
            if (!first)
                s << ", ";
            first = false;
            s << p.second.t << ":" << p.second.val;
            if (int(p.second.type) != type)
            {
                type = p.second.type;
                s << " " << WrappedClass::Point::getName(p.second.type);
            }
        }
        s << ")";
        return s.str();
    }
    uint count() const { return tl->size(); }
    double value(double time) const { return tl->get(time); }

    void clear() { tl->clear(); }
    void update() { tl->setAutoDerivative(); }
    void add(double time, double value) { tl->add(time, value); }
    void addType(double time, double value, WrappedClass::Point::Type type) { tl->add(time, value, type); }
    void changeType(WrappedClass::Point::Type type)
    {
        auto & data = tl->getData();
        for (auto & p : data)
            p.second.type = type;
    }
};


/** Ref-counted wrapper of MATH::Timeline1D for AngelScript.
    This class supports Vec as value type.
    So long as we don't have this timeline natively we cheat by using x timelines */
template <class Vec, uint NUM>
class TimelineXAS
{
    MATH::Timeline1D tl[NUM];
    int ref;
public:

    typedef MATH::Timeline1D WrappedClass;

    // --- factory ---

    static TimelineXAS<Vec,NUM> * factory() { return new TimelineXAS<Vec,NUM>; }

    void addRef() { ++ref; MO_DEBUG_TAS("Timeline"<<NUM<<"AS("<<this<<")::addRef() now = " << ref); }
    void releaseRef() { MO_DEBUG_TAS("Timeline"<<NUM<<"AS("<<this<<")::releaseRef() now = " << (ref-1));
                        if (--ref == 0) delete this; }

    // ----- ctor -----

    TimelineXAS()
        : ref   (1)
    {
        MO_DEBUG_TAS("Timeline"<<NUM<<"AS("<<this<<")::Timeline1AS()");
    }

    ~TimelineXAS()
    {
        MO_DEBUG_TAS("Timeline"<<NUM<<"AS("<<this<<")::~Timeline1AS()");
    }

    // ------ interface -------

    std::string toString() const
    {
        std::stringstream s;
        s << "Timeline" << NUM << "(";
        const auto & data = tl[0].getData();
        int type = -1;
        bool first = true;
        for (auto & p : data)
        {
            if (!first)
                s << ", ";
            first = false;
            s << p.second.t << ":(" << p.second.val;
            for (uint i=1; i<NUM; ++i)
            {
                auto it = tl[i].find(p.second.t);
                if (it != tl[i].getData().end())
                    s << "," << it->second.val;
            }
            s << ")";
            if (int(p.second.type) != type)
            {
                type = p.second.type;
                s << " " << WrappedClass::Point::getName(p.second.type);
            }
        }
        s << ")";
        return s.str();
    }
    uint count() const { return tl->size(); }
    Vec value(double time) const { Vec v; for (uint i=0; i<NUM; ++i) v[i] = tl[i].get(time); return v; }

    void clear() { for (uint i=0; i<NUM; ++i) tl[i].clear(); }
    void update() { for (uint i=0; i<NUM; ++i) tl[i].setAutoDerivative(); }
    void add(double time, const Vec& v) { for (uint i=0; i<NUM; ++i) tl[i].add(time, v[i]); }
    void addType(double time, const Vec& v, WrappedClass::Point::Type type)
        { for (uint i=0; i<NUM; ++i) tl[i].add(time, v[i], type); }
    void changeType(WrappedClass::Point::Type type)
    {
        for (uint i=0; i<NUM; ++i)
        {
            auto & data = tl[i].getData();
            for (auto & p : data)
                p.second.type = type;
        }
    }
};



namespace native {

// -------------------------------- registration ------------------------------------

// replaces %1 with typ, %2 with holdtyp (const char*) and returns utf8 as const char*
#define MO__STR(str__) (QString(str__).replace("%1", typ).replace("%2", holdtyp).toUtf8().constData())
// registers a method of class Class for typ (replaces %1 in decl__ with typ)
#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod(typ, MO__STR(decl__), asMETHOD(Class,name__), asCALL_THISCALL); assert( r >= 0 );


void register_timeline_enums(asIScriptEngine * engine)
{
    std::string enumType = "TimelinePointType";
    int r;
    r = engine->RegisterEnum(enumType.c_str()); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_CONSTANT",  MATH::Timeline1D::Point::CONSTANT); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_LINEAR",    MATH::Timeline1D::Point::LINEAR); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SMOOTH",    MATH::Timeline1D::Point::SMOOTH); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SYMMETRIC", MATH::Timeline1D::Point::SYMMETRIC); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_HERMITE",   MATH::Timeline1D::Point::SYMMETRIC2); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SPLINE4",   MATH::Timeline1D::Point::SPLINE4_SYM); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SPLINE4_2", MATH::Timeline1D::Point::SPLINE4); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SPLINE6",   MATH::Timeline1D::Point::SPLINE6); assert( r >= 0 );
}

template <class Class>
void register_timeline_tmpl(asIScriptEngine * engine, const char * typ, const char * holdtyp)
{
    int r;

    // ------- register the type -------------------------------

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
    MO__REG_METHOD("void update()", update);
    MO__REG_METHOD("void add(double time, %2 value)", add);
    MO__REG_METHOD("void add(double time, %2 value, TimelinePointType type)", addType);
    MO__REG_METHOD("void changeType(TimelinePointType type)", changeType);
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
        native::register_timeline_enums(engine);
        native::register_timeline_tmpl<Timeline1AS>(engine, "Timeline1", "double");
        native::register_timeline_tmpl<TimelineXAS<Vec2,2>>(engine, "Timeline2", "vec2");
        native::register_timeline_tmpl<TimelineXAS<Vec3,3>>(engine, "Timeline3", "vec3");
        native::register_timeline_tmpl<TimelineXAS<Vec4,4>>(engine, "Timeline4", "vec4");
    }
}



} // namespace MO

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
