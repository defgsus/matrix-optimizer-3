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
#include "math/Timeline1d.h"
#include "types/vector.h"
#include "types/Refcounted.h"
#include "io/log.h"
#include "io/error.h"

#if 0
#   define MO_DEBUG_TAS(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_TAS(unused__)
#endif

namespace MO {

/** Ref-counted wrapper of MATH::Timeline1D for AngelScript */
class Timeline1AS : public RefCounted
{
public:
    MATH::Timeline1d * tl;

    typedef MATH::Timeline1d WrappedClass;

    static const unsigned int num = 1;

    // --- factory ---

    static Timeline1AS * factory() { return new Timeline1AS; }

    // ----- ctor -----

    Timeline1AS()
        : RefCounted("Timeline1AS")
        , tl    (new MATH::Timeline1d)
    {
        MO_DEBUG_TAS("Timeline1AS("<<this<<")::Timeline1AS()");
    }

    Timeline1AS(MATH::Timeline1d * tl)
        : RefCounted("Timeline1AS")
        , tl    (tl)
    {
        MO_DEBUG_TAS("Timeline1AS("<<this<<")::Timeline1AS(" << tl << ")");
        MO_ASSERT(tl, "Can't create wrapper for NULL timeline");
    }

    ~Timeline1AS()
    {
        MO_DEBUG_TAS("Timeline1AS("<<this<<")::~Timeline1AS()");
        tl->releaseRef("Timeline1AS destroy");
    }

    void addRefWrapper() { addRef("Timeline1AS from angelscript"); }
    void releaseRefWrapper() { releaseRef("Timeline1AS from angelscript"); }

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
                s << " " << MATH::TimelinePoint::getName(p.second.type);
            }
        }
        s << ")";
        return s.str();
    }
    uint count() const { return tl->size(); }
    double start() const { return tl->empty() ? 0.0 : tl->getData().begin()->second.t; }
    double end() const { return tl->empty() ? 0.0 : tl->getData().rbegin()->second.t; }
    double length() const { return tl->empty() ? 0.0 : tl->getData().rbegin()->second.t - tl->getData().begin()->second.t; }

    double value(double time) const { return tl->get(time); }
    double derivative(double time, double range)
    { range = std::max(range, tl->timeQuantum()) * 0.5; return (value(time+range) - value(time-range)) / range; }

    // --- setter ---

    void clear() { tl->clear(); }
    void update() { tl->setAutoDerivative(); }
    void add(double time, double value) { MO_DEBUG(time << " " << value); tl->add(time, value); }
    void addType(double time, double value, MATH::TimelinePoint::Type type) { tl->add(time, value, type); }
    void changeType(MATH::TimelinePoint::Type type)
    {
        auto & data = tl->getData();
        for (auto & p : data)
            p.second.type = type;
    }

    void set(Timeline1AS * other) { if (other) *tl = *other->tl; }
};


/** Ref-counted wrapper of MATH::Timeline1D for AngelScript.
    This class supports Vec as value type.
    So long as we don't have this timeline natively we cheat by using NUM timelines
    XXX Update There is now MATH::TimelineNd, need to refacture this class
    */
template <class Vec, unsigned int NUM>
class TimelineXAS : public RefCounted
{
    MATH::Timeline1d* tl[NUM];
public:

    typedef MATH::Timeline1d WrappedClass;

    static const unsigned int num = NUM;

    // --- factory ---

    static TimelineXAS<Vec,NUM> * factory() { return new TimelineXAS<Vec,NUM>; }

    // ----- ctor -----

    TimelineXAS() : RefCounted("TimelineXAS")
    {
        MO_DEBUG_TAS("Timeline"<<NUM<<"AS("<<this<<")::Timeline1AS()");
        for (auto i = 0; i< NUM; ++i)
            tl[i] = new MATH::Timeline1d();
    }

    ~TimelineXAS()
    {
        MO_DEBUG_TAS("Timeline"<<NUM<<"AS("<<this<<")::~Timeline1AS()");
        for (auto i = 0; i<NUM; ++i)
            tl[i]->releaseRef("TimelineXAS destroy");
    }

    void addRefWrapper() { addRef("TimelineXAS from angelscript"); }
    void releaseRefWrapper() { releaseRef("TimelineXAS from angelscript"); }

    // ------ interface -------

    // --- getter ---

    std::string toString() const
    {
        std::stringstream s;
        s << "Timeline" << NUM << "(";
        const auto & data = tl[0]->getData();
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
                auto it = tl[i]->find(p.second.t);
                if (it != tl[i]->getData().end())
                    s << "," << it->second.val;
            }
            s << ")";
            if (int(p.second.type) != type)
            {
                type = p.second.type;
                s << " " << MATH::TimelinePoint::getName(p.second.type);
            }
        }
        s << ")";
        return s.str();
    }
    uint count() const { uint c = tl[0]->size(); for (uint i=1; i<NUM; ++i) c = std::max(c, tl[i]->size()); return c; }
    double start() const
    {
        double st = tl[0]->empty() ? 0.0 : tl[0]->getData().begin()->second.t;
        for (uint i=1; i<NUM; ++i)
            if (!tl[i]->empty()) st = std::min(st, tl[i]->getData().begin()->second.t);
        return st;
    }
    double end() const
    {
        double st = tl[0]->empty() ? 0.0 : tl[0]->getData().rbegin()->second.t;
        for (uint i=1; i<NUM; ++i)
            if (!tl[i]->empty()) st = std::max(st, tl[i]->getData().rbegin()->second.t);
        return st;
    }
    double length() const { return (end() - start()); }

    Vec value(double time) const { Vec v; for (uint i=0; i<NUM; ++i) v[i] = tl[i]->get(time); return v; }
    Vec derivative(double time, double range)
    { range = std::max(range, tl[0]->timeQuantum()) * 0.5; return (value(time+range) - value(time-range)) / Float(range); }

    Timeline1AS * getTimeline1(uint idx)
    {
        idx = std::min(NUM-1, idx);
        auto tl1 = Timeline1AS::factory();
        *tl1->tl = *tl[idx];
        return tl1;
    }

    // --- setter ---

    void clear() { for (uint i=0; i<NUM; ++i) tl[i]->clear(); }
    void update() { for (uint i=0; i<NUM; ++i) tl[i]->setAutoDerivative(); }
    void add(double time, const Vec& v) { for (uint i=0; i<NUM; ++i) tl[i]->add(time, v[i]); }
    void addType(double time, const Vec& v, MATH::TimelinePoint::Type type)
        { for (uint i=0; i<NUM; ++i) tl[i]->add(time, v[i], type); }
    void changeType(MATH::TimelinePoint::Type type)
    {
        for (uint i=0; i<NUM; ++i)
        {
            auto & data = tl[i]->getData();
            for (auto & p : data)
                p.second.type = type;
        }
    }

    void set(TimelineXAS<Vec,NUM> * other) { for (uint i=0; i<NUM; ++i) *tl[i] = *other->tl[i]; }

/*
    void set_from_(Timeline1AS * other[])
    {
        // copy first
        tl[0] = *(other[0]->tl);
        // reassign others (at que times of first one)
        for (uint i=1; i<NUM; ++i)
        {
            tl[i].clear();
            const auto & data = tl[0].getData();
            for (const auto & j : data)
            {
                float t = j->second.t;
                tl[i].add(j, other[i]->tl->get(...))...
            }
        }
    }
*/

    // --- these are NUM dependent (not for every instantiation) ---

    void set_1_1(Timeline1AS * a, Timeline1AS * b) { assert(NUM == 2); *tl[0] = *a->tl; *tl[1] = *b->tl; }
    void set_1_1_1(Timeline1AS * a, Timeline1AS * b, Timeline1AS * c) { assert(NUM == 3); *tl[0] = *a->tl; *tl[1] = *b->tl; *tl[2] = *c->tl; }
    void set_1_1_1_1(Timeline1AS * a, Timeline1AS * b, Timeline1AS * c, Timeline1AS * d)
        { assert(NUM == 4); *tl[0] = *a->tl; *tl[1] = *b->tl; *tl[2] = *c->tl; *tl[3] = *d->tl; }


};


namespace {
namespace native {

// -------------------------------- registration ------------------------------------

// replaces %1 with typ, %2 with holdtyp (const char*) and returns utf8 as const char*
#define MO__STR(str__) (QString(str__).replace("%1", typ).replace("%2", holdtyp).toUtf8().constData())
// registers a method of class Class for typ (applies MO__STR to decl__)
#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod(typ, MO__STR(decl__), asMETHOD(Class,name__), asCALL_THISCALL); assert( r >= 0 );


void register_timeline_enums(asIScriptEngine * engine)
{
    std::string enumType = "TimelinePointType";
    int r; Q_UNUSED(r);
    r = engine->RegisterEnum(enumType.c_str()); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_CONSTANT",  MATH::TimelinePoint::CONSTANT); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_LINEAR",    MATH::TimelinePoint::LINEAR); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SMOOTH",    MATH::TimelinePoint::SMOOTH); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SYMMETRIC", MATH::TimelinePoint::SYMMETRIC); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SYMMETRIC_USER", MATH::TimelinePoint::SYMMETRIC_USER); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_HERMITE",   MATH::TimelinePoint::HERMITE); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_HERMITE_USER",   MATH::TimelinePoint::HERMITE_USER); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SPLINE4", MATH::TimelinePoint::SPLINE4); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType.c_str(), "TL_SPLINE6",   MATH::TimelinePoint::SPLINE6); assert( r >= 0 );
}

template <class Class>
void register_timeline_tmpl(asIScriptEngine * engine, const char * typ, const char * holdtyp)
{
    int r; Q_UNUSED(r);

    // ------- register the type -------------------------------

    r = engine->RegisterObjectType(typ, 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
        MO__STR("%1@ f()"), asFUNCTION(Class::factory), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_ADDREF,
        "void f()", asMETHOD(Class,addRefWrapper), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_RELEASE,
        "void f()", asMETHOD(Class,releaseRefWrapper), asCALL_THISCALL); assert( r >= 0 );

    // --------------- the object methods ----------------------

    // getter
    MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("string toString() const", toString);
    MO__REG_METHOD("uint count() const", count);
    MO__REG_METHOD("double start() const", start);
    MO__REG_METHOD("double end() const", end);
    MO__REG_METHOD("double length() const", length);
    MO__REG_METHOD("%2 value(double time) const", value);
    MO__REG_METHOD("%2 derivative(double time, double range = 0.01) const", derivative);

    // setter
    MO__REG_METHOD("void clear()", clear);
    MO__REG_METHOD("void update()", update);
    MO__REG_METHOD("void add(double time, %2 value)", add);
    MO__REG_METHOD("void add(double time, %2 value, TimelinePointType type)", addType);
    MO__REG_METHOD("void changeType(TimelinePointType type)", changeType);

    MO__REG_METHOD("void set(%1@)", set);
}

// ---- for Timeline>1 only --------

template <class Class>
void register_timeline_tmpl_1plus(asIScriptEngine * engine, const char * typ, const char * holdtyp)
{
    int r; Q_UNUSED(r);
    MO__REG_METHOD("Timeline1@ getTimeline1(uint index)", getTimeline1);
}

// ---- unique per NUM vectors -----

template <class Class>
void register_timeline_tmpl_2(asIScriptEngine * engine, const char * typ, const char * holdtyp)
{
    int r; Q_UNUSED(r);
    MO__REG_METHOD("void set(Timeline1@ x, Timeline1@ y)", set_1_1);
    register_timeline_tmpl_1plus<Class>(engine, typ, holdtyp);
}

template <class Class>
void register_timeline_tmpl_3(asIScriptEngine * engine, const char * typ, const char * holdtyp)
{
    int r; Q_UNUSED(r);
    MO__REG_METHOD("void set(Timeline1@ x, Timeline1@ y, Timeline1@ z)", set_1_1_1);
    register_timeline_tmpl_1plus<Class>(engine, typ, holdtyp);
}

template <class Class>
void register_timeline_tmpl_4(asIScriptEngine * engine, const char * typ, const char * holdtyp)
{
    int r; Q_UNUSED(r);
    MO__REG_METHOD("void set(Timeline1@ x, Timeline1@ y, Timeline1@ z, Timeline1@ w)", set_1_1_1_1);
    register_timeline_tmpl_1plus<Class>(engine, typ, holdtyp);
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

        native::register_timeline_tmpl_2<TimelineXAS<Vec2,2>>(engine, "Timeline2", "vec2");
        native::register_timeline_tmpl_3<TimelineXAS<Vec3,3>>(engine, "Timeline3", "vec3");
        native::register_timeline_tmpl_4<TimelineXAS<Vec4,4>>(engine, "Timeline4", "vec4");
    }
}

Timeline1AS * timeline_to_angelscript(MATH::Timeline1d * tl)
{
    return new Timeline1AS(tl);
}

Timeline1AS * timeline_to_angelscript(const MATH::Timeline1d & tl)
{
    auto as = new Timeline1AS();
    *as->tl = tl;
    return as;
}

Timeline1AS * timeline_to_angelscript()
{
    auto as = new Timeline1AS();
    return as;
}

const MATH::Timeline1d& get_timeline(const Timeline1AS*as)
{
    return *(as->tl);
}

} // namespace MO

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
