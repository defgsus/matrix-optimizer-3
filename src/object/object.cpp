/** @file object.cpp

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/


//#include <QDebug>

#include "object.h"
#include "tool/stringmanip.h"
#include "io/error.h"
#include "io/datastream.h"
#include "io/log.h"

#include "objectfactory.h"
#include "scene.h"
#include "transform/transformation.h"
#include "param/parameterint.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "param/parameterfilename.h"
#include "param/parametertext.h"
#include "param/parametertimeline1d.h"
#include "audio/audiosource.h"
#include "audio/audiomicrophone.h"
#include "modulatorobjectfloat.h"

namespace MO {

bool registerObject_(Object * obj)
{
    return ObjectFactory::registerObject(obj);
}

Object::Object(QObject *parent) :
    QObject                 (parent),
    canBeDeleted_           (true),
    parentObject_           (0),
    childrenHaveChanged_    (false),
    numberThreads_          (1),
    bufferSize_             (1),
    paramActiveScope_       (0),
    sampleRate_             (44100),
    sampleRateInv_          (1.0/44100.0),
#ifndef MO_CLIENT
    parentActivityScope_    (AS_ON),
    currentActivityScope_   (AS_ON)
#else
    parentActivityScope_    (ActivityScope(AS_ON | AS_CLIENT_ONLY)),
    currentActivityScope_   (ActivityScope(AS_ON | AS_CLIENT_ONLY))
#endif
{
    // tie into Object hierarchy
    // NOTE: Has not been tested yet, and is actually never used
    if (auto o = qobject_cast<Object*>(parent))
    {
        setParentObject_(o);
    }
}

Object::~Object()
{
    for (auto p : parameters_)
        delete p;

    for (auto a : objAudioSources_)
        delete a;

    for (auto m : objMicrophones_)
        delete m;
}

// --------------------- io ------------------------

QByteArray Object::serializeTreeCompressed() const
{
    QByteArray data;

    IO::DataStream io(&data, QIODevice::WriteOnly);

    serializeTree(io);

    return qCompress(data, 9);
}

Object * Object::deserializeTreeCompressed(const QByteArray & cdata)
{
    QByteArray data = qUncompress(cdata);
    if (data.isEmpty())
        MO_IO_ERROR(READ, "could not uncompress object data");

    IO::DataStream io(data);
    return deserializeTree(io);
}

void Object::serializeTree(IO::DataStream & io) const
{
    MO_DEBUG_IO("Object('"<<idName()<<"')::serializeTree_()");

    // default header
    io.writeHeader("mo-tree", 1);

    // default object info
    io << className() << idName() << name();

    // keep position to skip unknown objects
    const qint64 startPos = io.beginSkip();

    // write actual object data
    serialize(io);

    // once in a while check stream for errors
    if (io.status() != QDataStream::Ok)
        MO_IO_ERROR(WRITE, "error serializing object '"<<idName()<<"'.\n"
                    "QIODevice error: '"<<io.device()->errorString()<<"'");

    // write parameters
    serializeParameters_(io, this);

    io.endSkip(startPos);

    // write childs
    io << (qint32)childObjects_.size();
    for (auto o : childObjects_)
        o->serializeTree(io);
}

Object * Object::deserializeTree(IO::DataStream & io)
{
    Object * obj = deserializeTree_(io);

    if (Scene * scene = qobject_cast<Scene*>(obj))
        scene->updateTree_();

    return obj;
}

Object * Object::deserializeTree_(IO::DataStream & io)
{
    MO_DEBUG_IO("Object::deserializeTree_()");

    // read default header
    io.readHeader("mo-tree", 1);

    // read default object info
    QString className, idName, name;
    io >> className >> idName >> name;

    qint64 objLength;
    io >> objLength;

    // create this object
    MO_DEBUG_IO("creating object '" << className << "'");
    Object * o = ObjectFactory::instance().createObject(className, false);

    if (o)
    {
        try
        {
            // read actual object data
            o->deserialize(io);
        }
        catch (Exception& e)
        {
            delete o;
            e << "\nobject creation failed for class '" << className << "'";
            throw;
        }

        // once in a while check stream for errors
        if (io.status() != QDataStream::Ok)
        {
            delete o;
            MO_IO_ERROR(READ, "error deserializing object '"<<idName<<"'.\n"
                        "QIODevice error: '"<<io.device()->errorString()<<"'");
        }

        // read parameters
        o->createParameters();
        try
        {
            deserializeParameters_(io, o);
        }
        catch (Exception& e)
        {
            delete o;
            e << "\nCould not read parameters for object '" << idName << "'";
            throw;
        }
    }
    // skip object if class not found
    else
    {
        MO_IO_WARNING(VERSION_MISMATCH, "unknown object class '" << className << "' in stream");
        io.skip(objLength);

        o = ObjectFactory::createDummy();
    }

    // set default object info
    o->idName_ = idName;
    o->name_ = name;

    // iterate over childs
    quint32 numChilds;
    io >> numChilds;

    for (quint32 i=0; i<numChilds; ++i)
    {
        Object * child = deserializeTree(io);
        MO_ASSERT(child, "duh?");
        o->addObject_(child);
    }

    // create audio objects
    o->createAudioSources();
    o->createMicrophones();

    return o;
}

void Object::serialize(IO::DataStream & io) const
{
    io.writeHeader("obj", 1);

    io << canBeDeleted_;
}

void Object::deserialize(IO::DataStream & io)
{
    io.readHeader("obj", 1);

    io >> canBeDeleted_;
}

void Object::serializeParameters_(IO::DataStream & io, const Object * o)
{
    // write parameters
    io.writeHeader("params", 1);

    io << (qint32)o->parameters_.size();

    for (auto p : o->parameters_)
    {
        io << p->idName();

        auto pos = io.beginSkip();
        p->serialize(io);
        io.endSkip(pos);
    }
}

void Object::deserializeParameters_(IO::DataStream & io, Object * o)
{
    // read parameters
    io.readHeader("params", 1);

    qint32 num;
    io >> num;

    for (int i=0; i<num; ++i)
    {
        QString id;
        io >> id;

        // length for skipping
        qint64 length;
        io >> length;

        Parameter * p = o->findParameter(id);
        if (!p)
        {
            MO_IO_WARNING(READ, "skipping unknown parameter '" << id << "' "
                                "in input stream.");
            io.skip(length);
        }
        else
            p->deserialize(io);
    }

    o->onParametersLoaded();
    o->updateParameterVisibility();
}


// --------------- info ----------------------------

void Object::dumpTreeIds(std::ostream &out, const std::string& prefix) const
{
    out << prefix << idName() << std::endl;

    for (const Object * c : childObjects_)
        c->dumpTreeIds(out, " " + prefix);
}

int Object::objectPriority(const Object *o)
{
    if (o->isTransformation())
        return 3;
    if (o->isModulatorObject())
        return 2;
    if (o->isAudioUnit())
        return 1;
    return 0;
}



// ---------------- getter -------------------------

QString Object::namePath() const
{
    Object * p = parentObject();
    if (!p)
        return name();

    QString path;
    while (p)
    {
        path.prepend("/" + p->name());
        p = p->parentObject();
    }

    return path + "/" + name();
}

QString Object::idNamePath() const
{
    Object * p = parentObject();
    if (!p)
        return idName();

    QString path;
    while (p)
    {
        path.prepend("/" + p->idName());
        p = p->parentObject();
    }

    return path + "/" + idName();
}

bool Object::isModulated() const
{
    for (auto p : parameters_)
        if (p->isModulated())
            return true;

    return false;
}

bool Object::isAudioRelevant() const
{
    if (!microphones().isEmpty() || !audioSources().isEmpty())
        return true;

    for (auto c : childObjects_)
        if (c->isAudioRelevant())
            return true;

    return false;
}

Object::ActivityScope Object::activityScope() const
{
    if (paramActiveScope_)
        return (ActivityScope)(
                    paramActiveScope_->baseValue() & parentActivityScope_);
    else
        return parentActivityScope_;
}

void Object::passDownActivityScope_(ActivityScope parent_scope)
{
    ActivityScope scope = parent_scope;
    if (paramActiveScope_)
        scope = (ActivityScope)(scope & paramActiveScope_->baseValue());

    for (auto c : childObjects_)
    {
        c->parentActivityScope_ = scope;
        c->passDownActivityScope_(scope);
    }
}

bool Object::active(Double /*time*/, uint /*thread*/) const
{
    // XXX active parameter not there yet
    return activityScope() & currentActivityScope_;
}

bool Object::activeAtAll() const
{
    return activityScope() & currentActivityScope_;
}

// ------------------ setter -----------------------

void Object::setName(const QString & n)
{
    name_ = n;
}

void Object::setCurrentActivityScope(ActivityScope scope)
{
    currentActivityScope_ = scope;

#ifdef MO_CLIENT
    currentActivityScope_ = ActivityScope(currentActivityScope_ | AS_CLIENT_ONLY);
#endif

    for (auto o : childObjects_)
        o->setCurrentActivityScope(scope);
}

// ------------- tree stuff ------------------------

const Object * Object::rootObject() const
{
    return parentObject_ ? parentObject_->rootObject() : this;
}

Object * Object::rootObject()
{
    return parentObject_ ? parentObject_->rootObject() : this;
}

const Scene * Object::sceneObject() const
{
    return qobject_cast<const Scene*>(rootObject());
}

Scene * Object::sceneObject()
{
    return qobject_cast<Scene*>(rootObject());
}

int Object::numChildren(bool recursive) const
{
    if (!recursive)
        return childObjects().size();

    int n = childObjects().size();
    for (auto o : childObjects_)
        n += o->numChildren(true);
    return n;
}

bool Object::hasParentObject(Object *o) const
{
    if (!parentObject_)
        return false;

    return parentObject_ == o? true : parentObject_->hasParentObject(o);
}

bool Object::isSaveToAdd(Object *o, QString &error) const
{
    if (!o)
    {
        error = tr("Object is NULL");
        return false;
    }

    if (!canHaveChildren(o->type()))
    {
        error = tr("'%1' can not have a child of this type").arg(idName());
        return false;
    }

    // test for modulation loops
    QList<Object*> mods = o->getFutureModulatingObjects(sceneObject());

#ifdef MO_DO_DEBUG_MOD
    MO_DEBUG_MOD("--- " << mods.size() << " modulators for " << o->idName());
    for (auto m : mods)
        MO_DEBUG_MOD(m->idName());
#endif

    if (mods.contains((Object*)this))
    {
        error = tr("Adding '%1' as a child to '%2' would cause an infinite "
                   "modulation loop!").arg(o->idName()).arg(idName());
        return false;
    }

    return true;
}


void Object::setParentObject_(Object *parent, int index)
{
    MO_ASSERT(parent, "NULL parent given for Object");

    MO_DEBUG_TREE("Object("<<idName()<<")::SetParentObject("<<parent->idName()<<")");

    MO_ASSERT(parentObject_ != parent, "trying to add object to same parent");
    MO_ASSERT(!hasParentObject(this), "trying to add object to it's own hierarchy");
    MO_ASSERT(parent->canHaveChildren(type()), "invalid child '" << idName() << "' "
                              "for object '" << parent->idName() << "'");

    // silently ignore in release mode
    if (parent == parentObject_ || hasParentObject(this))
        return;

    // install in QObject tree (handle memory)
    setParent(parent);

    // remove from previous parent
    if (parentObject_)
    {
        parentObject_->takeChild_(this);
    }
    parentObject_ = 0;

    // adjust idnames in new subtree
    makeUniqueIds_(parent->rootObject());

    // assign
    parentObject_ = parent;

    // and add to child list
    parentObject_->addChildObjectHelper_(this, index);

    // create any output objects
    createOutputs();

    // signal this object
    this->onParentChanged();
}


Object * Object::addObject_(Object * o, int index)
{
    MO_ASSERT(o, "trying to add a NULL child");
    MO_ASSERT(!childObjects_.contains(o), "duplicate addChild for '" << o->idName());

    o->setParentObject_(this, index);

    childrenHaveChanged_ = true;
    return o;
}

Object * Object::addChildObjectHelper_(Object * o, int index)
{
    MO_ASSERT(o, "trying to add a NULL child");

    if (index < 0)
        childObjects_.append(o);
    else
        childObjects_.insert(index, o);

    childrenHaveChanged_ = true;
    return o;
}

void Object::deleteObject_(Object * child, bool destroy)
{
    if (takeChild_(child))
    {
        child->setParent(0);
        if (destroy)
            delete child;
        childrenHaveChanged_ = true;
    }
}

bool Object::takeChild_(Object *child)
{
    if (childObjects_.removeOne(child))
    {
        return childrenHaveChanged_ = true;
    }
    return false;
}

void Object::swapChildren_(int from, int to)
{
    if (from >= 0 && from < numChildren()
        && to >= 0 && to < numChildren())

    childObjects_.swap(from, to);
    childrenHaveChanged_ = true;
}

QSet<QString> Object::getChildIds(bool recursive) const
{
    QList<Object*> list = findChildObjects(TG_ALL, recursive);

    QSet<QString> ids;

    for (auto o : list)
        ids.insert(o->idName());

    return ids;
}

QString Object::getUniqueId(QString id, const QSet<QString> &existingNames, bool * existed)
{
    MO_ASSERT(!id.isEmpty(), "unset object idName detected");

    if (existed)
        *existed = false;

    // create an id if necessary
    if (id.isEmpty())
        id = "Object";

    // replace white char with underscore
    id.replace(QRegExp("\\s\\s*"), "_");

    while (existingNames.contains(id))
    {
        increase_id_number(id, 1);
        if (existed)
            *existed = true;
    }

    return id;
}

void Object::makeUniqueIds_(Object * root)
{
    // get all existing ids
    QSet<QString> existing = root->getChildIds(true);
    existing.insert(root->idName());
    // ignore self when searching own tree
    if (root == rootObject())
        existing.remove(idName());

    // call string version
    makeUniqueIds_(existing);
}

void Object::makeUniqueIds_(QSet<QString> &existing)
{
    bool changed = false;

    // make this object's id unique
    idName_ = getUniqueId(idName_, existing, &changed);

    // add to existing ids
    if (changed)
        existing.insert(idName_);

    // go through childs
    for (auto o : childObjects_)
        o->makeUniqueIds_(existing);
}

Object * Object::findChildObject(const QString &id, bool recursive, Object * ignore) const
{
    for (auto o : childObjects_)
        if (o != ignore && o->idName() == id)
            return o;

    if (recursive)
        for (auto i : childObjects_)
            if (Object * o = i->findChildObject(id, recursive, ignore))
                return o;

    return 0;
}

QList<Object*> Object::findChildObjects(int typeFlags, bool recursive) const
{
    QList<Object*> list;
    for (auto o : childObjects_)
        if (o->type() & typeFlags)
            list.append(o);

    if (recursive)
        for (auto o : childObjects_)
            list.append( o->findChildObjects(typeFlags, true) );

    return list;
}

bool Object::canHaveChildren(Type t) const
{
    // dummy can contain/be added to everything
    if (t == T_DUMMY || type() == T_DUMMY)
        return true;

    // XXX for now: ModulatorObjects can be anywhere
    if (t & TG_MODULATOR_OBJECT)
        return true;

    // Clips belong into ClipContainer ...
    if (t == T_CLIP)
        return type() == T_CLIP_CONTAINER;

    // and nothing else
    if (type() == T_CLIP_CONTAINER)
        return t == T_CLIP;

    // XXX Currently ClipContainer only into scene
    if (t & T_CLIP_CONTAINER)
        return type() == T_SCENE;

    // microphone groups only contain microphones
    if (type() == T_MICROPHONE_GROUP)
        return t == T_MICROPHONE;

    // audio-units can contain only audio-units
    if (type() == T_AUDIO_UNIT)
        return t == T_AUDIO_UNIT;

    // audio-units can be part of scene and any object
    if (t == T_AUDIO_UNIT)
        return type() == T_SCENE || (type() & TG_REAL_OBJECT);

    // transformations are child-less
    if (type() == T_TRANSFORMATION)
        return false;

    // except for the transformation mix class
    // which holds itself and transformations
    if (type() == T_TRANSFORMATION_MIX)
        return t & TG_TRANSFORMATION;

    // sequences only belong on tracks or sequencegroups with matching type
    // or clips
    if (t & TG_SEQUENCE)
        return
               type() == T_SEQUENCEGROUP
            || type() == T_CLIP
            || (t == T_SEQUENCE_FLOAT && type() == T_TRACK_FLOAT);

    // sequencegroups belong on tracks
    if (t == T_SEQUENCEGROUP)
        return  (type() & TG_TRACK)
                // or themselfes
                || type() == T_SEQUENCEGROUP;

    // tracks contain nothing else but sequences
    if (type() & TG_TRACK)
        return false;

    // sequence contains nothing
    if (type() & TG_SEQUENCE)
        return false;

    // scene can hold anything else, except transformations
    if (type() == T_SCENE)
        return !(t & TG_TRANSFORMATION);

    return true;
}

void Object::childrenChanged_()
{
    MO_DEBUG_TREE("Object('" << idName() << "')::childrenChanged_()");

    // collect special sub-objects
    collectTransformationObjects_();

    // notify derived classes
    childrenChanged();

    passDownActivityScope_(activityScope());

    childrenHaveChanged_ = false;
}


void Object::onObjectsAboutToDelete(const QList<Object *> & list)
{
    for (Parameter * p : parameters_)
    {
        for (const Object * o : list)
            if (p->findModulator(o->idName()))
                p->removeModulator(o->idName());
    }
}


void Object::propagateRenderMode(ObjectGl *parent)
{
    for (auto c : childObjects_)
        c->propagateRenderMode(parent);
}

bool Object::verifyNumberThreads(uint num)
{
    if (numberThreads_ != num)
        return false;

    for (auto a : objAudioSources_)
        if (a->numberThreads() != num)
            return false;

    for (auto m : objMicrophones_)
        if (m->numberThreads() != num)
            return false;

    return true;
}

void Object::setNumberThreads(uint num)
{
    MO_DEBUG_TREE("Object('" << idName() << "')::setNumberThreads(" << num << ")");

    numberThreads_ = num;

    transformation_.resize(num);
    bufferSize_.resize(num);

    for (auto a : objAudioSources_)
        a->setNumberThreads(num);

    for (auto m : objMicrophones_)
        m->setNumberThreads(num);
}


// ------------------------- 3d -----------------------

void Object::clearTransformation(uint thread, uint sample)
{
    transformation_[thread][sample] = Mat4(1.0);
}

void Object::collectTransformationObjects_()
{
    transformationObjects_.clear();

    for (auto o : childObjects_)
        if (auto t = qobject_cast<Transformation*>(o))
            transformationObjects_.append(t);
}

void Object::calculateTransformation(Mat4 &matrix, Double time, uint thread) const
{
    for (auto t : transformationObjects_)
        if (t->active(time, thread))
            t->applyTransformation(matrix, time, thread);
}

// -------------------- outputs ------------------------------

void Object::requestCreateOutputs()
{
    Scene * scene = sceneObject();
    if (!scene)
        MO_WARNING("Object('" << idName() << "')::requestCreateOutputs() "
                   "called without scene object");

    scene->callCreateOutputs_(this);
}

ModulatorObjectFloat * Object::createOutputFloat(const QString &given_id, const QString &name)
{
    MO_DEBUG_MOD("Object('" << idName() << "')::createOutputFloat('"
             << given_id << "', '" << name << "')");

    QString id = idName() + "_" + given_id;

    Object * o = findChildObject(id);

    // construct new
    if (!o)
    {
        ModulatorObjectFloat * mod = ObjectFactory::createModulatorObjectFloat();
        mod->canBeDeleted_ = false;
        mod->idName_ = id;
        mod->name_ = name;
        addObject_(mod);
        MO_DEBUG_MOD("Object('" << idName() << "')::createOutputFloat() created new '"
                 << mod->idName() << "'");
        return mod;
    }

    // see if already there
    if (ModulatorObjectFloat * mod = qobject_cast<ModulatorObjectFloat*>(o))
    {
        mod->canBeDeleted_ = false;
        mod->name_ = name;
        MO_DEBUG_MOD("Object('" << idName() << "')::createOutputFloat() reusing '"
                 << mod->idName() << "'");
        return mod;
    }

    MO_ASSERT(false, "Object::createOutputFloat() called, object '" << id << "' found "
              "but it's not of ModulatorObject class, instead: " << o);

    return 0;
}



// ---------------------- parameter --------------------------

Parameter * Object::findParameter(const QString &id)
{
    for (auto p : parameters_)
        if (p->idName() == id)
            return p;
    return 0;
}

void Object::createParameters()
{
    const static QString
            strTip(tr("Defines the scope in which the object and all of it's children are active")),
            strOff(tr("Object is inactive")),
            strOn(tr("Object is always active")),
            strClient(tr("Object is only active on clients (projectors)")),
            strPrev(tr("Object is only active in the preview modes and will not be rendered")),
            strPrev1(tr("Object is only active in preview mode 1 and will not be rendered")),
            strPrev2(tr("Object is only active in preview mode 2 and will not be rendered")),
            strPrev3(tr("Object is only active in preview mode 3 and will not be rendered")),
            strPrev1r(tr("Object is only active in preview mode 1 and when rendering")),
            strPrev2r(tr("Object is only active in preview mode 2 and when rendering")),
            strPrev3r(tr("Object is only active in preview mode 3 and when rendering")),
            strRend(tr("Object is only active when rendering"));

    beginParameterGroup("active", tr("activity"));

    paramActiveScope_ =
    createSelectParameter("_activescope", tr("activity scope"),
                         strTip,
                         { "off", "on", "client", "prev", "ren", "prev1", "prev2", "prev3",
                           "prev1r", "prev2r", "prev3r" },
                         { tr("off"), tr("on"), tr("client only"), tr("preview"), tr("render"),
                           tr("preview 1"), tr("preview 2"), tr("preview 3"),
                           tr("preview 1 + render"), tr("preview 2 + render"), tr("preview 3 + render") },
                         { strOff, strOn, strClient, strPrev, strRend, strPrev1, strPrev2, strPrev3,
                           strPrev1r, strPrev2r, strPrev3r },
                         { AS_OFF, AS_ON, AS_CLIENT_ONLY, AS_PREVIEW, AS_RENDER,
                           AS_PREVIEW_1, AS_PREVIEW_2, AS_PREVIEW_3,
                           AS_PREVIEW_1 | AS_RENDER, AS_PREVIEW_2 | AS_RENDER, AS_PREVIEW_3 | AS_RENDER },
                         AS_ON, true, false );

    endParameterGroup();
}

void Object::onParameterChanged(Parameter * p)
{
    MO_DEBUG_PARAM("Object::parameterChanged('" << p->idName() << "')");

    // activity scope changed
    if (p == paramActiveScope_)
        passDownActivityScope_(activityScope());

}


void Object::beginParameterGroup(const QString &id, const QString &name)
{
    currentParameterGroupId_ = id;
    currentParameterGroupName_ = name;
}

void Object::endParameterGroup()
{
    currentParameterGroupId_.clear();
    currentParameterGroupName_.clear();
}

ParameterFloat * Object::createFloatParameter(
        const QString& id, const QString& name, const QString& statusTip,
        Double defaultValue, bool editable, bool modulateable)
{
    return createFloatParameter(id, name, statusTip, defaultValue,
                                -ParameterFloat::infinity, ParameterFloat::infinity,
                                1.0, editable, modulateable);
}

ParameterFloat * Object::createFloatParameter(
        const QString& id, const QString& name, const QString &statusTip,
        Double defaultValue, Double smallStep, bool editable, bool modulateable)
{
    return createFloatParameter(id, name, statusTip, defaultValue,
                                -ParameterFloat::infinity, ParameterFloat::infinity,
                                smallStep, editable, modulateable);
}

ParameterFloat * Object::createFloatParameter(
        const QString& id, const QString& name, const QString& statusTip,
        Double defaultValue, Double minValue, Double maxValue, Double smallStep,
        bool editable, bool modulateable)
{
    ParameterFloat * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto pf = dynamic_cast<ParameterFloat*>(p))
        {
            param = pf;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested float "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterFloat(this, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setDefaultValue(std::min(maxValue, std::max(minValue, defaultValue )));
    param->setMinValue(minValue);
    param->setMaxValue(maxValue);
    param->setSmallStep(smallStep);
    param->setStatusTip(statusTip);
    param->setEditable(editable);
    param->setModulateable(modulateable);

    param->setGroup(currentParameterGroupId_, currentParameterGroupName_);

    return param;
}




ParameterInt * Object::createIntParameter(
        const QString& id, const QString& name, const QString& statusTip,
        Int defaultValue, bool editable, bool modulateable)
{
    return createIntParameter(id, name, statusTip, defaultValue,
                                -ParameterInt::infinity, ParameterInt::infinity,
                                1, editable, modulateable);
}

ParameterInt * Object::createIntParameter(
        const QString& id, const QString& name, const QString &statusTip,
        Int defaultValue, Int smallStep, bool editable, bool modulateable)
{
    return createIntParameter(id, name, statusTip, defaultValue,
                                -ParameterInt::infinity, ParameterInt::infinity,
                                smallStep, editable, modulateable);
}

ParameterInt * Object::createIntParameter(
        const QString& id, const QString& name, const QString& statusTip,
        Int defaultValue, Int minValue, Int maxValue, Int smallStep,
        bool editable, bool modulateable)
{
    ParameterInt * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto pf = dynamic_cast<ParameterInt*>(p))
        {
            param = pf;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested int "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterInt(this, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setDefaultValue(std::min(maxValue, std::max(minValue, defaultValue )));
    param->setMinValue(minValue);
    param->setMaxValue(maxValue);
    param->setSmallStep(smallStep);
    param->setStatusTip(statusTip);
    param->setEditable(editable);
    param->setModulateable(modulateable);

    param->setGroup(currentParameterGroupId_, currentParameterGroupName_);

    return param;
}



ParameterSelect * Object::createBooleanParameter(
            const QString& id, const QString& name, const QString& statusTip,
            const QString& offStatusTip, const QString& onStatusTip,
            bool defaultValue, bool editable, bool modulateable)
{
    ParameterSelect * p = createSelectParameter(
            id, name, statusTip,
            { "off", "on" },
            { tr("off"), tr("on") },
            { offStatusTip, onStatusTip },
            { false, true },
            defaultValue, editable, modulateable);

    p->setBoolean(true);

    return p;
}

ParameterSelect * Object::createSelectParameter(
            const QString& id, const QString& name, const QString& statusTip,
            const QStringList& valueIds, const QStringList& valueNames, const QList<int> &valueList,
            int defaultValue, bool editable, bool modulateable)
{
    return createSelectParameter(id, name, statusTip,
                                 valueIds, valueNames, QStringList(), valueList,
                                 defaultValue, editable, modulateable);
}

ParameterSelect * Object::createSelectParameter(
            const QString& id, const QString& name, const QString& statusTip,
            const QStringList& valueIds, const QStringList& valueNames, const QStringList& statusTips,
            const QList<int> &valueList,
            int defaultValue, bool editable, bool modulateable)
{
    ParameterSelect * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterSelect*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested select "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterSelect(this, id, name);
        parameters_.append(param);

        // first time init
        param->setValueList(valueList);
        param->setValueIds(valueIds);
        param->setValueNames(valueNames);
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(modulateable);
    param->setStatusTip(statusTip);
    param->setEditable(editable);
    param->setValueList(valueList);
    param->setValueIds(valueIds);
    param->setValueNames(valueNames);
    param->setStatusTips(statusTips);
    param->setDefaultValue(defaultValue);

    param->setGroup(currentParameterGroupId_, currentParameterGroupName_);

    return param;
}

ParameterText * Object::createTextParameter(
            const QString& id, const QString& name, const QString& statusTip,
            const QString& defaultValue,
            bool editable, bool modulateable)
{
    return createTextParameter(id, name, statusTip, TT_PLAIN_TEXT,
                               defaultValue, editable, modulateable);
}

ParameterText * Object::createTextParameter(
            const QString& id, const QString& name, const QString& statusTip,
            TextType textType,
            const QString& defaultValue,
            bool editable, bool modulateable)
{
    ParameterText * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterText*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested text "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterText(this, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(modulateable);
    param->setEditable(editable);
    param->setTextType(textType);
    param->setDefaultValue(defaultValue);
    param->setStatusTip(statusTip);

    param->setGroup(currentParameterGroupId_, currentParameterGroupName_);

    return param;
}


ParameterFilename * Object::createFilenameParameter(
            const QString& id, const QString& name, const QString& statusTip,
            IO::FileType fileType, const QString& defaultValue, bool editable)
{
    ParameterFilename * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto ps = dynamic_cast<ParameterFilename*>(p))
        {
            param = ps;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested filename "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterFilename(this, id, name);
        parameters_.append(param);

        // first time init
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setModulateable(false);
    param->setEditable(editable);
    param->setFileType(fileType);
    param->setDefaultValue(defaultValue);
    param->setStatusTip(statusTip);

    param->setGroup(currentParameterGroupId_, currentParameterGroupName_);

    return param;
}


ParameterTimeline1D * Object::createTimeline1DParameter(
        const QString& id, const QString& name, const QString& statusTip,
        const MATH::Timeline1D * defaultValue,
        bool editable)
{
    return createTimeline1DParameter(id, name, statusTip,
                              defaultValue,
                              -ParameterTimeline1D::infinity,
                              +ParameterTimeline1D::infinity,
                              -ParameterTimeline1D::infinity,
                              +ParameterTimeline1D::infinity,
                              editable);
}

ParameterTimeline1D * Object::createTimeline1DParameter(
        const QString& id, const QString& name, const QString& statusTip,
        const MATH::Timeline1D * defaultValue,
        Double minTime, Double maxTime,
        bool editable)
{
    return createTimeline1DParameter(id, name, statusTip,
                              defaultValue,
                              minTime, maxTime,
                              -ParameterTimeline1D::infinity,
                              +ParameterTimeline1D::infinity,
                              editable);
}

ParameterTimeline1D * Object::createTimeline1DParameter(
        const QString& id, const QString& name, const QString& statusTip,
        const MATH::Timeline1D * defaultValue,
        Double minTime, Double maxTime, Double minValue, Double maxValue,
        bool editable)
{
    ParameterTimeline1D * param = 0;

    // see if already there

    if (auto p = findParameter(id))
    {
        if (auto pf = dynamic_cast<ParameterTimeline1D*>(p))
        {
            param = pf;
        }
        else
        {
            MO_ASSERT(false, "object '" << idName() << "' requested timeline1d "
                      "parameter '" << id << "' "
                      "which is already present as parameter of type " << p->typeName());
        }
    }

    // create new
    if (!param)
    {
        param = new ParameterTimeline1D(this, id, name);
        parameters_.append(param);

        // first time init
        if (defaultValue)
            param->setTimeline(*defaultValue);
    }

    // override potentially previous
    param->setName(name);
    if (defaultValue)
        param->setDefaultTimeline(*defaultValue);
    param->setMinTime(minTime);
    param->setMaxTime(maxTime);
    param->setMinValue(minValue);
    param->setMaxValue(maxValue);
    param->setStatusTip(statusTip);
    param->setEditable(editable);

    param->setGroup(currentParameterGroupId_, currentParameterGroupName_);

    return param;
}


// ----------------- audio sources ---------------------

bool Object::verifyBufferSize(uint thread, uint bufferSize)
{
    if (bufferSize_.size() < thread
        || bufferSize_[thread] != bufferSize)
        return false;

    for (auto a : objAudioSources_)
        if (a->bufferSize(thread) != bufferSize)
            return false;

    for (auto m : objMicrophones_)
        if (m->bufferSize(thread) != bufferSize)
            return false;

    return true;
}

void Object::setBufferSize(uint bufferSize, uint thread)
{
    bufferSize_[thread] = bufferSize;
    transformation_[thread].resize(bufferSize);

    for (auto a : objAudioSources_)
        a->setBufferSize(bufferSize, thread);

    for (auto m : objMicrophones_)
        m->setBufferSize(bufferSize, thread);
}

void Object::setSampleRate(uint samplerate)
{
    MO_ASSERT(samplerate>0, "bogus samplerate");

    sampleRate_ = std::max((uint)1, samplerate);
    sampleRateInv_ = 1.0 / sampleRate_;

    for (auto m : objMicrophones_)
        m->setSampleRate(sampleRate_);
}

void Object::requestCreateAudioSources()
{
    Scene * s = sceneObject();
    if (!s)
        createAudioSources();
    else
        s->callCreateAudioSources_(this);
}

void Object::requestCreateMicrophones()
{
    Scene * s = sceneObject();
    if (!s)
        createMicrophones();
    else
        s->callCreateMicrophones_(this);
}


AUDIO::AudioSource * Object::createAudioSource(const QString& id)
{
    auto a = new AUDIO::AudioSource(id, this);

    objAudioSources_.append(a);

    return a;
}

QList<AUDIO::AudioSource*> Object::createOrDeleteAudioSources(const QString &id, uint number)
{
    //MO_DEBUG("Object::createOrDeleteAudioSources(" << id << ", " << number << ")");

    // get all that exist with the given id
    QMap<QString, AUDIO::AudioSource*> exist;
    for (auto m : objAudioSources_)
        if (m->idName().startsWith(id))
            exist.insert(m->idName(), m);

    QList<AUDIO::AudioSource*> list;

    // create or reuse
    for (uint i=0; i<number; ++i)
    {
        QString curid = QString("%1_%2").arg(id).arg(i+1);

        if (exist.contains(curid))
        {
            list.append(exist[curid]);
            exist.remove(curid);
        }
        else
        {
            list.append( createAudioSource(curid) );
        }
    }

    // delete the ones not needed
    for (auto m : exist)
    {
        objAudioSources_.removeOne(m);
        delete m;
    }

    return list;
}

AUDIO::AudioMicrophone * Object::createMicrophone(const QString &id)
{
    auto m = new AUDIO::AudioMicrophone(id, this);

    objMicrophones_.append(m);

    // update with current buffer info
    m->setSampleRate(sampleRate_);
    m->setNumberThreads(numberThreads());
    for (uint i=0; i<numberThreads(); ++i)
        m->setBufferSize(bufferSize(i), i);

    return m;
}

QList<AUDIO::AudioMicrophone*> Object::createOrDeleteMicrophones(const QString &id, uint number)
{
    // get all that exist with the given id
    QMap<QString, AUDIO::AudioMicrophone*> exist;
    for (auto m : objMicrophones_)
        if (m->idName().startsWith(id))
            exist.insert(m->idName(), m);

    QList<AUDIO::AudioMicrophone*> list;

    // create or reuse
    for (uint i=0; i<number; ++i)
    {
        QString curid = QString("%1_%2").arg(id).arg(i+1);

        if (exist.contains(curid))
        {
            list.append(exist[curid]);
            exist.remove(curid);
        }
        else
        {
            list.append( createMicrophone(curid) );
        }
    }

    // delete the ones not needed
    for (auto m : exist)
    {
        objMicrophones_.removeOne(m);
        delete m;
    }

    return list;
}

// -------------------- modulators ---------------------

QList<Object*> Object::getModulatingObjects() const
{
    QList<Object*> list;

    for (auto p : parameters())
    {
        list.append(p->getModulatingObjects());
    }

    return list;
}

QList<Object*> Object::getFutureModulatingObjects(const Scene *scene) const
{
    QList<Object*> list;

    for (auto p : parameters())
    {
        list.append(p->getFutureModulatingObjects(scene));
    }

    return list;
}


} // namespace MO
