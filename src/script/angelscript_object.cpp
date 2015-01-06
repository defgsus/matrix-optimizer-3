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
#include "angelscript_geometry.h"
#include "script/angelscript.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "object/scene.h"
#include "object/sequencefloat.h"
#include "object/model3d.h"
#include "object/util/objecteditor.h"
#include "io/log.h"


namespace MO {

//------------------------------
// AngelScript functions wrapper
//------------------------------


class ObjectAS
{
public:
    Object * o;
    int ref;
    bool owning;
    ObjectAS * self;

    /** Create a non-owning wrapper */
    ObjectAS(Object * o)
        : o         (o),
          ref       (1),
          owning    (false),
          self      (this)
    {
        // The syntax checker needs a null object..
        //MO_ASSERT(o, "Can't create wrapper for NULL Object");
    }

    ~ObjectAS()
    {
        if (owning)
            delete o;
    }

    static ObjectAS * factory(Object * o) { return new ObjectAS(o); }
    void addRef() { ++ref; }
    void releaseRef() { if (--ref == 0) delete this; }

    // --------------- interface -----------------

    SequenceAS * toSequence();

    // ------ getter ------

    StringAS toString()
    {
        if (o == 0)
            return "null";
        std::stringstream s;
        s << o;
        return s.str();
    }

    StringAS name() { return toStringAS(o->name()); }
    StringAS id() { return toStringAS(o->idName()); }

    int childrenCount() { return o->childObjects().size(); }
    ObjectAS * children(int i) { return (i>=0 && i<o->childObjects().size()) ? factory(o->childObjects()[i]) : 0; }

    ObjectAS * parent() { return o->parentObject() ? factory(o->parentObject()) : 0; }
    ObjectAS * root() { return o->rootObject() ? factory(o->rootObject()) : 0; }


    ObjectAS * findName(const StringAS& n)
    {
        QString qn = QString::fromUtf8(n.c_str());
        auto obj = o->findChildObject([=](Object * o){ return o->name() == qn; });
        if (obj)
            return factory(obj);
        return 0;
    }

    // ------ setter ------

    ObjectEditor * editor() { auto s = o->sceneObject(); return s ? s->editor() : 0; }

    void setName(const StringAS& n) { auto e = editor(); if (e) e->setObjectName(o, QString::fromUtf8(n.c_str())); }

    Timeline1AS * timeline()
    {
        if (auto seq = qobject_cast<SequenceFloat*>(o))
            if (seq->timeline())
                return timeline_to_angelscript(*seq->timeline());
        return 0;
    }

    void setTimeline(Timeline1AS * tl)
    {
        if (tl)
        if (auto seq = qobject_cast<SequenceFloat*>(o))
        {
            seq->setTimeline(get_timeline(tl));
            // gui signal
            if (auto e = editor())
                emit e->sequenceChanged(seq);
        }
    }

    GeometryAS * geometry()
    {
        if (auto model = qobject_cast<Model3d*>(o))
            if (model->geometry())
                return geometry_to_angelscript(model->geometry());
        return 0;
    }

    void setGeometry(GeometryAS * gas)
    {
        Model3d * model = qobject_cast<Model3d*>(o);
        if (!model)
        {
            MO_WARNING("Object " << o << " has no Geometry to set");
            return;
        }

        GEOM::Geometry * g = getGeometry(gas);
        if (!g)
        {
            MO_WARNING("No Geometry to set for object " << o);
            return;
        }

        model->setGeometry(*g);
    }

};







class SequenceAS
{
public:
    SequenceFloat * o;
    int ref;
    bool owning;
    SequenceAS * self;

    /** Create an owning wrapper */
    SequenceAS()
        : o         (ObjectFactory::createSequenceFloat()),
          ref       (1),
          owning    (true),
          self      (this)
    { }

    /** Create a non-owning wrapper */
    SequenceAS(SequenceFloat * o)
        : o         (o),
          ref       (1),
          owning    (false),
          self      (this)
    {
        MO_ASSERT(o, "Can't create wrapper for NULL Object");
    }

    ~SequenceAS()
    {
        if (owning)
            delete o;
    }

    static SequenceAS * factory(SequenceFloat * o) { return new SequenceAS(o); }
    static SequenceAS * factoryNew() { return new SequenceAS(); }

    void addRef() { ++ref; }
    void releaseRef() { if (--ref == 0) delete this; }

    ObjectEditor * editor() { auto s = o->sceneObject(); return s ? s->editor() : 0; }

    // --------------- interface -----------------

    // ------ getter ------

    Double start() const { return o->start(); }
    Double end() const { return o->end(); }
    Double length() const { return o->length(); }
    Double loopStart() const { return o->loopStart(); }
    Double loopEnd() const { return o->loopEnd(); }
    Double loopLength() const { return o->loopLength(); }
    Double speed() const { return o->speed(); }

    // ------ setter ------

#define MO__EMIT if (auto e = editor()) emit e->sequenceChanged(o)

    void setStart(Double t) { o->setStart(t); MO__EMIT; }
    void setEnd(Double t) { o->setEnd(t); MO__EMIT; }
    void setLength(Double t) { o->setLength(t); MO__EMIT; }
    void setLoopStart(Double t) { o->setLoopStart(t); MO__EMIT; }
    void setLoopEnd(Double t) { o->setLoopEnd(t); MO__EMIT; }
    void setLoopLength(Double t) { o->setLoopLength(t); MO__EMIT; }
    void setSpeed(Double t) { o->setSpeed(t); MO__EMIT; }

#undef MO__EMIT

};




// ---- cast -----

SequenceAS * ObjectAS::toSequence() { if (auto c = qobject_cast<SequenceFloat*>(o)) return SequenceAS::factory(c); return 0; }




namespace {



//--------------------------------
// Registration
//-------------------------------------

namespace native {

#define MO__REG_METHOD(decl__, func__) \
    r = engine->RegisterObjectMethod(typ, decl__, asMETHOD(Wrapper, func__), asCALL_THISCALL); assert( r >= 0 );
#define MO__REG_METHOD_F(decl__, func__) \
    r = engine->RegisterObjectMethod(typ, decl__, asFUNCTION(func__), asCALL_CDECL); assert( r >= 0 );
#define MO__REG_FUNC(decl__, func__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(func__), asCALL_CDECL); assert( r >= 0 );



// --- base object interface ---
template <class Wrapper>
static void register_object_base(asIScriptEngine *engine, const char * typ)
{
    int r;

    // -------------------- types ------------------------------

    r = engine->RegisterObjectType(typ, 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------

    //r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
    //    MO__STR("Wrapper@ f()"), asFUNCTION(Wrapper::factory), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_ADDREF,
        "void f()", asMETHOD(Wrapper,addRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_RELEASE,
        "void f()", asMETHOD(Wrapper,releaseRef), asCALL_THISCALL); assert( r >= 0 );

    // --------------- the object methods ----------------------

    // strings
    MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("string name() const", name);
    MO__REG_METHOD("string id() const", id);

    // getter
    MO__REG_METHOD("int childrenCount() const", childrenCount);
    MO__REG_METHOD("Object@ children(int)", children);
    MO__REG_METHOD("const Object@ children(int) const", children);    
    MO__REG_METHOD("Object@ parent()", parent);
    MO__REG_METHOD("const Object@ parent() const", parent);
    MO__REG_METHOD("Object@ root()", root);
    MO__REG_METHOD("const Object@ root() const", root);

    MO__REG_METHOD("Object@ find(const string &in name)", findName);
    MO__REG_METHOD("const Object@ find(const string &in name) const", findName);

    MO__REG_METHOD("Timeline1@ getTimeline() const", timeline);
    MO__REG_METHOD("Geometry@ getGeometry() const", geometry);

    // setter
    MO__REG_METHOD("void setName(const string &in)", setName);
    MO__REG_METHOD("void setTimeline(const Timeline1@)", setTimeline);
    MO__REG_METHOD("void setGeometry(const Geometry@)", setGeometry);



    // ------------ non-member object functions ----------------

//    MO__REG_FUNC("", );

}

// -- basic sequence interface --
template <class Wrapper>
static void register_sequence_base(asIScriptEngine *engine, const char * typ)
{
    int r;

    // ---------------- constructor ----------------------------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
        "Sequence@ f()", asFUNCTION(Wrapper::factoryNew), asCALL_CDECL); assert( r >= 0 );

    // --------------- the object methods ----------------------

    // getter
    MO__REG_METHOD("double start() const", start);
    MO__REG_METHOD("double end() const", end);
    MO__REG_METHOD("double length() const", length);
    MO__REG_METHOD("double loopStart() const", loopStart);
    MO__REG_METHOD("double loopEnd() const", loopEnd);
    MO__REG_METHOD("double loopLength() const", loopLength);
    MO__REG_METHOD("double speed() const", speed);

    // setter

    MO__REG_METHOD("void setStart(double second)", setStart);
    MO__REG_METHOD("void setEnd(double second)", setEnd);
    MO__REG_METHOD("void setLength(double second)", setLength);
    MO__REG_METHOD("void setLoopStart(double second)", setLoopStart);
    MO__REG_METHOD("void setLoopEnd(double second)", setLoopEnd);
    MO__REG_METHOD("void setLoopLength(double second)", setLoopLength);
    MO__REG_METHOD("void setSpeed(double speed)", setSpeed);

}

template <class Wrapper>
static void register_casts(asIScriptEngine * engine, const char * typ)
{
    int r;

    // cast
    //MO__REG_METHOD_F("const Sequence@ toSequence() const", (object_to_<SequenceFloat, SequenceAS>));
    MO__REG_METHOD("Sequence@ toSequence()", toSequence);

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
    {
        // base object type for each class
        native::register_object_base<ObjectAS>(engine, "Object");
        native::register_object_base<ObjectAS>(engine, "Sequence");

        // specialized types
        native::register_sequence_base<SequenceAS>(engine, "Sequence");

        native::register_casts<ObjectAS>(engine, "Object");
    }
}



// ---------------------------- instantiations ---------------------------------


void registerAngelScript_rootObject(asIScriptEngine *engine, Scene* root, bool writeable)
{
    int r;
    ObjectAS * as = ObjectAS::factory(root);
    if (writeable)
    {
        r = engine->RegisterGlobalProperty("Object@ scene", &as->self); assert( r >= 0 );
    }
    else
    {
        r = engine->RegisterGlobalProperty("const Object@ scene", &as->self); assert( r >= 0 );
    }
}

void registerAngelScript_object(asIScriptEngine *engine, Object * obj, bool writeable, bool withRoot)
{
    int r;
    ObjectAS * as = ObjectAS::factory(obj);
    if (writeable)
    {
        r = engine->RegisterGlobalProperty("Object@ object", &as->self); assert( r >= 0 );
    }
    else
    {
        r = engine->RegisterGlobalProperty("const Object@ object", &as->self); assert( r >= 0 );
    }

    if (withRoot)
    {
        if (!obj || !obj->sceneObject())
            registerAngelScript_rootObject(engine, 0, writeable);
        else
            registerAngelScript_rootObject(engine, obj->sceneObject(), writeable);
    }
}




} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
