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
    { }

    ~ObjectAS()
    {
        if (owning)
            delete o;
    }

    static ObjectAS * factory(Object * o) { return new ObjectAS(o); }
    void addRef() { ++ref; }
    void releaseRef() { if (--ref == 0) delete this; }

    // --------------- interface -----------------

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
    ObjectAS * children(int i) { return factory(o->childObjects()[i]); }

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


namespace {



//--------------------------------
// Registration
//-------------------------------------

namespace native {

#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod("Object", decl__, asMETHOD(ObjectAS, name__), asCALL_THISCALL); assert( r >= 0 );
#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );


static void register_object(asIScriptEngine *engine)
{
    int r;

    // register the type
    r = engine->RegisterObjectType("Object", 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------

    //r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
    //    MO__STR("ObjectAS@ f()"), asFUNCTION(ObjectAS::factory), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Object", asBEHAVE_ADDREF,
        "void f()", asMETHOD(ObjectAS,addRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Object", asBEHAVE_RELEASE,
        "void f()", asMETHOD(ObjectAS,releaseRef), asCALL_THISCALL); assert( r >= 0 );

    // --------------- the object methods ----------------------

    // strings
    MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("string name() const", name);
    MO__REG_METHOD("string id() const", id);

    // getter
    MO__REG_METHOD("int childrenCount() const", childrenCount);
    MO__REG_METHOD("Object@ children(int)", children);
    MO__REG_METHOD("const Object@ children(int) const", children);
    MO__REG_METHOD("Object@ find(const string &in name)", findName);
    MO__REG_METHOD("const Object@ find(const string &in name) const", findName);

    MO__REG_METHOD("Timeline1@ getTimeline() const", timeline);

    // setter
    MO__REG_METHOD("void setName(const string &in)", setName);
    MO__REG_METHOD("void setTimeline(const Timeline1@)", setTimeline);
    MO__REG_METHOD("void setGeometry(const Geometry@)", setGeometry);



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
