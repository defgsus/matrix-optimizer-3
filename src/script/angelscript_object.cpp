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
#include "angelscript.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "object/scene.h"
#include "object/sequencefloat.h"
#include "object/group.h"
#include "object/model3d.h"
#include "object/camera.h"
#include "object/microphone.h"
#include "object/soundsource.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parametertext.h"
#include "object/param/parametertimeline1d.h"
#include "object/param/parameterselect.h"
#include "object/util/objecteditor.h"
#include "math/timeline1d.h"
#include "audio/tool/waveform.h"
#include "types/refcounted.h"
#include "io/log.h"


namespace MO {

//------------------------------
// AngelScript functions wrapper
//------------------------------

template <class OBJ>
class ObjectAnyAS;

typedef ObjectAnyAS<Group> GroupAS;
typedef ObjectAnyAS<Model3d> ModelAS;

class ParameterAS;

// helper to find an object by name
// (XXX needs to be extended to accept paths)
Object * find_name(Object * root, const QString& n)
{
    if (root->name() == n)
        return root;

    for (auto c : root->childObjects())
        if (auto o = find_name(c, n))
            return o;

    return 0;
}

// helper to find a specific object class with specific name
// (XXX needs to be extended to accept paths)
template <class OBJ>
OBJ * find_type_name(Object * root, const QString& n)
{
    if (root->name() == n)
        if (auto o = qobject_cast<OBJ*>(root))
            return o;

    for (auto c : root->childObjects())
        if (auto o = find_type_name<OBJ>(c, n))
            return o;

    return 0;
}


#define MO__EDITOR_WARNING \
    MO_WARNING("Operation on object without attached scene currently not supported")

class ObjectAS : public RefCounted
{
public:
    Object * o;
    ObjectAS * self;

    /** Create a wrapper for existing object */
    ObjectAS(Object * o)
        : o         (o),
          self      (this)
    {
        // The syntax checker needs a null object..
        //MO_ASSERT(o, "Can't create wrapper for NULL Object");
        if (o)
            o->addRef();
    }

private:
    ~ObjectAS()
    {
        if (o)
            o->releaseRef();
    }
public:

    // ---------------- helper -------------------

    static ObjectAS * wrap_(Object * o) { return new ObjectAS(o); }

    ObjectEditor * editor() { auto s = o->sceneObject(); return s ? s->editor() : 0; }

    bool addObject_(Object * newChild, int index = -1);

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
    int hue() const { return o->getAttachedData(Object::DT_HUE).toInt(); }
    bool isExpanded() const { return o->getAttachedData(Object::DT_GRAPH_EXPANDED).toBool(); }
    Vec2 position() const { QPoint p = o->getAttachedData(Object::DT_GRAPH_POS).toPoint(); return Vec2(p.x(), p.y()); }
    int positionX() const { return o->getAttachedData(Object::DT_GRAPH_POS).toPoint().x(); }
    int positionY() const { return o->getAttachedData(Object::DT_GRAPH_POS).toPoint().y(); }

    uint childrenCount() { return o->childObjects().size(); }
    ObjectAS * children(uint i) { return ((int)i<o->childObjects().size()) ? wrap_(o->childObjects()[i]) : 0; }

    ObjectAS * parent() { return o->parentObject() ? wrap_(o->parentObject()) : 0; }
    ObjectAS * root() { return o->rootObject() ? wrap_(o->rootObject()) : 0; }

    ObjectAS * findName(const StringAS& n) { auto obj = find_name(o, MO::toString(n)); return obj ? wrap_(obj) : 0; }

    ObjectAS * findPath(const StringAS& path) { auto obj = o->findObjectByNamePath(MO::toString(path)); return obj ? wrap_(obj) : 0; }

    bool isSequence() const { return qobject_cast<Sequence*>(o) != 0; }
    bool isGroup() const { return qobject_cast<Group*>(o) != 0; }
    bool isModel() const { return qobject_cast<Model3d*>(o) != 0; }
    bool isCamera() const { return qobject_cast<Camera*>(o) != 0; }
#ifndef MO_DISABLE_SPATIAL
    bool isMicrophone() const { return qobject_cast<Microphone*>(o) != 0; }
    bool isSoundSource() const { return qobject_cast<SoundSource*>(o) != 0; }
#endif

    uint parameterCount() { return o->params()->parameters().size(); }
    ParameterAS * parameter(uint index);
    ParameterAS * findParameter(const StringAS&);

    Mat4 transformation() const { return o->transformation(); }
    Double time() const { auto s = o->sceneObject(); return s ? s->sceneTime() : 0.; }

    // ------ setter ------

    void setName(const StringAS& n) { QString name = MO::toString(n); if (auto e = editor()) e->setObjectName(o, name); else o->setName(name); }
    void setHue(int degree) { o->setAttachedData(degree, Object::DT_HUE); if (auto e = editor()) emit e->objectColorChanged(o); }
    void setExpanded(bool exp) { o->setAttachedData(exp, Object::DT_GRAPH_EXPANDED); if (auto e = editor()) emit e->objectChanged(o); }
    void setPosition(int x, int y) { o->setAttachedData(QPoint(x, y), Object::DT_GRAPH_POS); if (auto e = editor()) emit e->objectChanged(o); }

    // ------ tree setter -----

    bool addObject(ObjectAS * as, int index = -1) { return addObject_(as->o, index); }
    void deleteChildren() { if (auto e = editor()) { e->deleteChildren(o); } else MO__EDITOR_WARNING; }



    // --------- internal objects --------

    Timeline1AS * timeline()
    {
        if (auto seq = qobject_cast<SequenceFloat*>(o))
            if (seq->timeline())
                return timeline_to_angelscript(*seq->timeline());
        return timeline_to_angelscript(MATH::Timeline1D());
    }

    void setTimeline(Timeline1AS * tl)
    {
        if (tl)
        if (auto seq = qobject_cast<SequenceFloat*>(o))
        {
            seq->setTimeline(get_timeline(tl));
            if (!seq->sequenceType() == SequenceFloat::ST_TIMELINE)
                seq->setSequenceType(SequenceFloat::ST_TIMELINE);
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


// Wrapper for types that don't have special functions
template <class OBJ>
class ObjectAnyAS : public RefCounted
{
public:
    OBJ * o;
    ObjectAnyAS * self;

    /** Create a wrapper for new object */
    ObjectAnyAS()
        : o         (MO::create_object<OBJ>()),
          self      (this)
    {

    }

    /** Create a wrapper for existing object */
    ObjectAnyAS(OBJ * o)
        : o         (o),
          self      (this)
    {
        MO_ASSERT(o, "Can't create wrapper for NULL Object");
        o->addRef();
    }

private:
    ~ObjectAnyAS()
    {
        o->releaseRef();
    }
};



class SequenceAS : public RefCounted
{
public:
    SequenceFloat * o;
    SequenceAS * self;

    /** Create a wrapper for new object */
    SequenceAS()
        : o         (MO::create_object<SequenceFloat>()),
          self      (this)
    { }

    /** Create a wrapper for existing object */
    SequenceAS(SequenceFloat * o)
        : o         (o),
          self      (this)
    {
        MO_ASSERT(o, "Can't create wrapper for NULL Object");
        o->addRef();
    }
private:
    ~SequenceAS()
    {
        o->releaseRef();
    }
public:

    // ------------ helper -----------------------

    ObjectEditor * editor() { auto s = o->sceneObject(); return s ? s->editor() : 0; }

    // --------------- interface -----------------

    ObjectAS * toObject() { return ObjectAS::wrap_(o); return 0; }

    // ------ getter ------

    Double start() const { return o->start(); }
    Double end() const { return o->end(); }
    Double length() const { return o->length(); }
    bool looping() const { return o->looping(); }
    Double loopStart() const { return o->loopStart(); }
    Double loopEnd() const { return o->loopEnd(); }
    Double loopLength() const { return o->loopLength(); }
    Double speed() const { return o->speed(); }
    Double frequency() const { return o->frequency(); }
    Double amplitude() const { return o->amplitude(); }
    Double offset() const { return o->offset(); }
    Double phase() const { return o->phase(); }
    Double pulsewidth() const { return o->pulseWidth(); }

    int type() const { return o->sequenceType(); }
    int oscillatorType() const { return o->oscillatorMode(); }

    // ------ setter ------

#define MO__EMIT if (auto e = editor()) emit e->sequenceChanged(o)

    void setStart(Double t) { o->setStart(t); MO__EMIT; }
    void setEnd(Double t) { o->setEnd(t); MO__EMIT; }
    void setLength(Double t) { o->setLength(t); MO__EMIT; }
    void setLooping(bool b) { o->setLooping(b); MO__EMIT; }
    void setLoopStart(Double t) { o->setLoopStart(t); MO__EMIT; }
    void setLoopEnd(Double t) { o->setLoopEnd(t); MO__EMIT; }
    void setLoopLength(Double t) { o->setLoopLength(t); MO__EMIT; }
    void setSpeed(Double t) { o->setSpeed(t); MO__EMIT; }
    void setFrequency(Double t) { o->setFrequency(t); MO__EMIT; }
    void setAmplitude(Double t) { o->setAmplitude(t); MO__EMIT; }
    void setOffset(Double t) { o->setOffset(t); MO__EMIT; }
    void setPhase(Double t) { o->setPhase(t); MO__EMIT; }
    void setPulsewidth(Double t) { o->setPulseWidth(t); MO__EMIT; }

    void setType(int type) { if (type>=0 && type <SequenceFloat::ST_MAX) { o->setSequenceType((SequenceFloat::SequenceType)type); MO__EMIT; } }
    void setOscillatorType(int type) { if (type>=0 && type <AUDIO::Waveform::T_MAX_TYPES) { o->setOscillatorMode((AUDIO::Waveform::Type)type); MO__EMIT; } }

#undef MO__EMIT

};






class ParameterAS : public RefCounted
{
public:
    Object * o;
    Parameter * p;
    ParameterAS * self;

    /** Create a wrapper for existing object */
    ParameterAS(Parameter * p)
        : o         (p->object()),
          p         (p),
          self      (this)
    {
        MO_ASSERT(p, "Can't create wrapper for NULL Parameter");
        MO_ASSERT(o, "Can't create AngelScript Parameter without parent Object");
        o->addRef();
    }

private:
    ~ParameterAS()
    {
        o->releaseRef();
    }

public:

    // ---------------- helper -------------------

    static ParameterAS * wrap_(Parameter * p) { return new ParameterAS(p); }

    ObjectEditor * editor() { if (!o) return 0; auto s = o->sceneObject(); return s ? s->editor() : 0; }

    template <class P, typename T>
    void set_(P * pf, const T& v) { if (auto e = editor()) e->setParameterValue(pf, v); else pf->setValue(v); }

    // --------------- interface -----------------


    // ------ getter ------

    StringAS toString() const
    {
        if (p == 0)
            return "null";
        std::stringstream s;
        s << "Parameter(" << p->name() << ")";
        return s.str();
    }

    StringAS name() const { return toStringAS(p->name()); }
    StringAS id() const { return toStringAS(p->idName()); }

    // ----- setter -------

    void setFloat(Float v)
    {
        if (auto pf = dynamic_cast<ParameterFloat*>(p)) set_<ParameterFloat, Float>(pf, v);
        else if (auto pi = dynamic_cast<ParameterInt*>(p)) set_<ParameterInt, Int>(pi, v);
        else MO_WARNING("Parameter '" << p->name() << "' is no float parameter");
    }
    void setInt(int32_t v)
    {
        if (auto pi = dynamic_cast<ParameterInt*>(p)) set_<ParameterInt, Int>(pi, v);
        else if (auto pf = dynamic_cast<ParameterFloat*>(p)) set_<ParameterFloat, Float>(pf, v);
        else MO_WARNING("Parameter '" << p->name() << "' is no int parameter");
    }
    void setBool(bool v)
    {
        if (auto ps = dynamic_cast<ParameterSelect*>(p))
        {
            if (ps->isBoolean())
                set_<ParameterSelect, bool>(ps, v);
            else
                MO_WARNING("Parameter '" << p->name() << "' is no boolean parameter");
        }
        else if (auto pi = dynamic_cast<ParameterInt*>(p)) set_<ParameterInt, Int>(pi, v ? 1 : 0);
        else if (auto pf = dynamic_cast<ParameterFloat*>(p)) set_<ParameterFloat, Float>(pf, v ? 1.f : 0.f);
        else MO_WARNING("Parameter '" << p->name() << "' is no boolean parameter");
    }
    void setText(const StringAS& v)
    {
        if (auto pt = dynamic_cast<ParameterText*>(p)) set_<ParameterText, QString>(pt, MO::toString(v));
        else MO_WARNING("Parameter '" << p->name() << "' is no text parameter");
    }

};




// --------- ObjectAS implementations with other types --------

bool ObjectAS::addObject_(Object * newChild, int index)
{
    QString error;
    if (!o->isSaveToAdd(newChild, error))
    {
        MO_WARNING("angelscript: Can't add object " << newChild->name() << " to " << o->name() << ": "
                   << error);
        return false;
    }

    auto e = editor();
    if (!e)
    {
        MO_WARNING("Sorry, currently can't add to object without attached scene...");
        return false;
    }

    e->makeUniqueIds(o, newChild);
    bool ret = e->addObject(o, newChild, index);
    MO_ASSERT(ret, "");

    return ret;
}

ParameterAS * ObjectAS::parameter(uint index)
{
    if ((int)index >= o->params()->parameters().count())
        return 0;

    return ParameterAS::wrap_(o->params()->parameters()[index]);
}

ParameterAS * ObjectAS::findParameter(const StringAS& as)
{
    QString s = MO::toString(as);
    auto p = o->params()->findParameter(s);

    return p ? ParameterAS::wrap_(p) : 0;
}


namespace {

// ----- factories ----

template <class OBJ, class AS>
struct obj_factory
{
    static AS * factory(OBJ * o) { return new AS(o); }
    static AS * factoryNew(const StringAS& name)
    {
        auto as = new AS();
        if (!name.empty())
            as->o->setName(MO::toString(name));
        return as;
    }
};

// specialization for Object as it can't be instantiated
template <class AS>
struct obj_factory<Object, AS>
{
    static AS * factory(Object * o) { return new AS(o); }
    static AS * factoryNew(const StringAS& ) { return 0; }
};

template <class From, class To, class OBJ>
struct obj_cast
{
    static To * cast(From * as) { if (auto c = qobject_cast<OBJ*>(as->o)) return obj_factory<OBJ, To>::factory(c); return 0; }

//    static ObjectAS * castToObjectAS(From * as) { return obj_factory<Object, ObjectAS>::factory(as->o); }
};

// template to get or create a specific instance
template <class OBJ>
struct obj_getcreate
{
    template <class Wrapper>
    static Wrapper * get(ObjectAS * self, const StringAS& nas)
    {
        QString n = MO::toString(nas);

        // return existing
        if (auto obj = find_type_name<OBJ>(self->o, n))
            return new Wrapper(obj);

        // create new
        auto e = self->editor();
        if (!e) { MO__EDITOR_WARNING; return 0; }
        OBJ * newobj = create_object<OBJ>(n);
        MO_ASSERT(newobj, "wrong class? " << typeid(OBJ).name());
        e->addObject(self->o, newobj);
        return new Wrapper(newobj);
    }
};

// ------- nonmembers -----

struct obj_nonmem
{

};



//--------------------------------
// Registration
//-------------------------------------

namespace native {

#define MO__REG_METHOD(decl__, func__) \
    r = engine->RegisterObjectMethod(typ, decl__, asMETHOD(Wrapper, func__), asCALL_THISCALL); assert( r >= 0 );
#define MO__REG_METHOD_F(decl__, func__) \
    r = engine->RegisterObjectMethod(typ, decl__, asFUNCTION(func__), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
#define MO__REG_FUNC(decl__, func__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(func__), asCALL_CDECL); assert( r >= 0 );



// --- base object interface ---
template <class Wrapper>
static void register_object_base(asIScriptEngine *engine, const char * typ)
{
    int r; Q_UNUSED(r);

    // -------------------- types ------------------------------

    //r = engine->RegisterObjectType(typ, 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------

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
    MO__REG_METHOD("int hue() const", hue);
    MO__REG_METHOD("bool isExpanded() const", isExpanded);
    MO__REG_METHOD("vec2 position() const", position);
    MO__REG_METHOD("int positionX() const", positionX);
    MO__REG_METHOD("int positionY() const", positionY);

    MO__REG_METHOD("bool isSequence() const", isSequence);
    MO__REG_METHOD("bool isGroup() const", isGroup);
    MO__REG_METHOD("bool isModel() const", isModel);
    MO__REG_METHOD("bool isCamera() const", isCamera);
#ifndef MO_DISABLE_SPATIAL
    MO__REG_METHOD("bool isMicrophone() const", isMicrophone);
    MO__REG_METHOD("bool isSoundSource() const", isSoundSource);
#endif

    MO__REG_METHOD("uint childrenCount() const", childrenCount);
    MO__REG_METHOD("Object@ children(uint)", children);
    MO__REG_METHOD("const Object@ children(uint) const", children);
    MO__REG_METHOD("Object@ parent()", parent);
    MO__REG_METHOD("const Object@ parent() const", parent);
    MO__REG_METHOD("Object@ root()", root);
    MO__REG_METHOD("const Object@ root() const", root);

    MO__REG_METHOD("Object@ find(const string &in name)", findName);
    MO__REG_METHOD("const Object@ find(const string &in name) const", findName);

    MO__REG_METHOD("Object@ findPath(const string &in path)", findPath);
    MO__REG_METHOD("const Object@ findPath(const string &in path) const", findPath);

    MO__REG_METHOD_F("Sequence@ getSequence(const string &in name)", obj_getcreate<SequenceFloat>::get<SequenceAS>);
    MO__REG_METHOD_F("Model@ getModel(const string &in name)", obj_getcreate<Model3d>::get<ModelAS>);
    MO__REG_METHOD_F("Group@ getGroup(const string &in name)", obj_getcreate<Group>::get<GroupAS>);

    MO__REG_METHOD("uint parameterCount() const", parameterCount);
    MO__REG_METHOD("Parameter@ parameter(uint index) const", parameter);
    MO__REG_METHOD("Parameter@ parameter(const string &in id) const", findParameter);

    MO__REG_METHOD("double time() const", time);
    MO__REG_METHOD("mat4 transformation() const", transformation);

    MO__REG_METHOD("Timeline1@ getTimeline() const", timeline);
    MO__REG_METHOD("Geometry@ getGeometry() const", geometry);

    // setter
    MO__REG_METHOD("void setName(const string &in)", setName);
    MO__REG_METHOD("void setHue(int degree)", setHue);
    MO__REG_METHOD("void setExpanded(bool expanded = true)", setExpanded);
    MO__REG_METHOD("void setPosition(int x, int y)", setPosition);

    // internal object setter
    MO__REG_METHOD("void setTimeline(const Timeline1@)", setTimeline);
    MO__REG_METHOD("void setGeometry(const Geometry@)", setGeometry);

    // tree setter
    MO__REG_METHOD("void deleteChildren()", deleteChildren);
    MO__REG_METHOD("bool addObject(Object@ newChild, int insert_index = -1)", addObject);

    // ------------ non-member object functions ----------------

    //MO__REG_FUNC("Sequence@ createSequence(const string &in name)", obj_nonmem::createSequence);
}


template <class OBJ, class AS>
static void register_object_construct(asIScriptEngine *engine, const char * typ)
{
    int r; Q_UNUSED(r);

    // ----------- type ------------

    //r = engine->RegisterObjectType(typ, 0, asOBJ_REF); assert( r >= 0 );

    // -------- constructor --------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
        QString("%1@ f(const string &in name = \"\")").arg(typ).toUtf8().constData(),
        asFUNCTION((obj_factory<OBJ, AS>::factoryNew)), asCALL_CDECL); assert( r >= 0 );
}


template <class Wrapper>
static void register_casts(asIScriptEngine * engine, const char * typ)
{
    int r; Q_UNUSED(r);

#define MO__REG_CAST(decl__, ClassAS__, Class__) \
    r = engine->RegisterObjectMethod(typ, decl__, \
        asFUNCTION((obj_cast<Wrapper, ClassAS__, Class__>::cast)), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // ------------ member -----------

    if (!strstr(typ, "Object"))
    {
        MO__REG_CAST("Object@ toObject()", ObjectAS, Object);
        MO__REG_CAST("Object@ opImplConv()", ObjectAS, Object);
    }
    MO__REG_CAST("Sequence@ toSequence()", SequenceAS, SequenceFloat);
    MO__REG_CAST("Group@ toGroup()", ObjectAnyAS<Group>, Group);

#undef MO__REG_CAST
}


// -- basic sequence interface --
template <class Wrapper>
static void register_sequence_base(asIScriptEngine *engine, const char * typ)
{
    int r; Q_UNUSED(r);

    // ---------------- constructor ----------------------------

//    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
//        "Sequence@ f()", asFUNCTION(Wrapper::factoryNew), asCALL_CDECL); assert( r >= 0 );

    // -------------------- enums ------------------------------

    {
        const char * enumType = "SequenceType";
        r = engine->RegisterEnum(enumType); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_CONSTANT",  SequenceFloat::ST_CONSTANT); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_TIMELINE",  SequenceFloat::ST_TIMELINE); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_OSCILLATOR",  SequenceFloat::ST_OSCILLATOR); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_OSCILLATOR_WT",  SequenceFloat::ST_OSCILLATOR_WT); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_OSCILLATOR_ADD",  SequenceFloat::ST_ADD_OSC); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_OSCILLATOR_ADD_WT",  SequenceFloat::ST_ADD_WT); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_SPECTRAL",  SequenceFloat::ST_SPECTRAL_WT); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_SOUNDFILE",  SequenceFloat::ST_SOUNDFILE); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_EQUATION",  SequenceFloat::ST_EQUATION); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "SEQ_EQUATION_WT",  SequenceFloat::ST_EQUATION_WT); assert( r >= 0 );
    }

    {
        const char * enumType = "WaveformType";
        r = engine->RegisterEnum(enumType); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "WF_SINE",  AUDIO::Waveform::T_SINE); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "WF_COSINE",  AUDIO::Waveform::T_COSINE); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "WF_RAMP",  AUDIO::Waveform::T_RAMP); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "WF_SAW_UP",  AUDIO::Waveform::T_SAW_RISE); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "WF_SAW_DOWN",  AUDIO::Waveform::T_SAW_DECAY); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "WF_TRIANGLE",  AUDIO::Waveform::T_TRIANGLE); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "WF_SQUARE",  AUDIO::Waveform::T_SQUARE); assert( r >= 0 );
        r = engine->RegisterEnumValue(enumType, "WF_NOISE",  AUDIO::Waveform::T_NOISE); assert( r >= 0 );
    }

    // --------------- the object methods ----------------------

    // getter

    MO__REG_METHOD("double start() const", start);
    MO__REG_METHOD("double end() const", end);
    MO__REG_METHOD("double length() const", length);
    MO__REG_METHOD("bool looping() const", looping);
    MO__REG_METHOD("double loopStart() const", loopStart);
    MO__REG_METHOD("double loopEnd() const", loopEnd);
    MO__REG_METHOD("double loopLength() const", loopLength);
    MO__REG_METHOD("double speed() const", speed);
    MO__REG_METHOD("double frequency() const", frequency);
    MO__REG_METHOD("double amplitude() const", amplitude);
    MO__REG_METHOD("double offset() const", offset);
    MO__REG_METHOD("double phase() const", phase);
    MO__REG_METHOD("double pulsewidth() const", pulsewidth);

    MO__REG_METHOD("SequenceType type() const", type);
    MO__REG_METHOD("WaveformType waveformType() const", oscillatorType);

    // setter

    MO__REG_METHOD("void setStart(double second)", setStart);
    MO__REG_METHOD("void setEnd(double second)", setEnd);
    MO__REG_METHOD("void setLength(double second)", setLength);
    MO__REG_METHOD("void setLooping(bool)", setLooping);
    MO__REG_METHOD("void setLoopStart(double second)", setLoopStart);
    MO__REG_METHOD("void setLoopEnd(double second)", setLoopEnd);
    MO__REG_METHOD("void setLoopLength(double second)", setLoopLength);
    MO__REG_METHOD("void setSpeed(double speed)", setSpeed);
    MO__REG_METHOD("void setFrequency(double hz)", setFrequency);
    MO__REG_METHOD("void setAmpltide(double )", setAmplitude);
    MO__REG_METHOD("void setOffset(double )", setOffset);
    MO__REG_METHOD("void setPhase(double )", setPhase);
    MO__REG_METHOD("void setPulsewidth(double )", setPulsewidth);

    MO__REG_METHOD("void setType(SequenceType type)", setType);
    MO__REG_METHOD("void setType(WaveformType type)", setOscillatorType);
    MO__REG_METHOD("void setWaveformType(WaveformType type)", setOscillatorType);
}

template <class Wrapper>
static void register_parameter_base(asIScriptEngine *engine, const char * typ)
{
    int r; Q_UNUSED(r);

    // -------------------- types ------------------------------

    //r = engine->RegisterObjectType(typ, 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------

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

    // setter
    MO__REG_METHOD("void setValue(float value)", setFloat);
    MO__REG_METHOD("void setValue(int32 value)", setInt);
    MO__REG_METHOD("void setValue(bool value)", setBool);
    MO__REG_METHOD("void setValue(const string &in value)", setText);
}

#undef MO__REG_FUNC
#undef MO__REG_METHOD
#undef MO__REG_METHOD_F
#undef MO__EDITOR_WARNING

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
        // type forwards
        int r; Q_UNUSED(r);
        r = engine->RegisterObjectType("Parameter", 0, asOBJ_REF); assert( r >= 0 );
        r = engine->RegisterObjectType("Object", 0, asOBJ_REF); assert( r >= 0 );
        r = engine->RegisterObjectType("Sequence", 0, asOBJ_REF); assert( r >= 0 );
        r = engine->RegisterObjectType("Group", 0, asOBJ_REF); assert( r >= 0 );
        r = engine->RegisterObjectType("Model", 0, asOBJ_REF); assert( r >= 0 );

        // base object type for each class
        native::register_object_base<ObjectAS>(engine, "Object");
        native::register_object_base<ObjectAS>(engine, "Sequence");
        native::register_object_base<ObjectAS>(engine, "Group");
        native::register_object_base<ObjectAS>(engine, "Model");

        native::register_object_construct<Sequence, SequenceAS>(engine, "Sequence");
        native::register_object_construct<Group, GroupAS>(engine, "Group");
        native::register_object_construct<Model3d, ModelAS>(engine, "Model");

        native::register_casts<ObjectAS>(engine, "Object");
        native::register_casts<SequenceAS>(engine, "Sequence");
        native::register_casts<GroupAS>(engine, "Group");
        native::register_casts<ModelAS>(engine, "Model");

        // specialized types
        native::register_sequence_base<SequenceAS>(engine, "Sequence");

        // parameters
        native::register_parameter_base<ParameterAS>(engine, "Parameter");
    }
}



// ---------------------------- instantiations ---------------------------------


void registerAngelScript_rootObject(asIScriptEngine *engine, Scene* root, bool writeable)
{
    int r; Q_UNUSED(r);
    ObjectAS * as = ObjectAS::wrap_(root);
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
    int r; Q_UNUSED(r);
    ObjectAS * as = ObjectAS::wrap_(obj);
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
