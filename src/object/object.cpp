/** @file object.cpp

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/


#include <QDebug>

#include "object.h"
#include "objectfactory.h"
#include "object/util/objecteditor.h"
#include "scene.h"
#include "transform/transformation.h"
#include "param/parameters.h"
#include "param/parameterselect.h"
#include "audio/spatial/spatialsoundsource.h"
#include "audio/spatial/spatialmicrophone.h"
#include "math/transformationbuffer.h"
#include "modulatorobjectfloat.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "tool/stringmanip.h"

namespace MO {

bool registerObject_(Object * obj)
{
    return ObjectFactory::registerObject(obj);
}

Object::Object(QObject *parent) :
    QObject                   (parent),
    p_parameters_             (0),
    p_paramActiveScope_       (0),
    p_canBeDeleted_           (true),
    p_parentObject_           (0),
    p_childrenHaveChanged_    (false),
    p_numberThreads_          (1),
    p_numberSoundSources_     (0),
    p_numberMicrophones_      (0),
    p_sampleRate_             (44100),
    p_sampleRateInv_          (1.0/44100.0),
    p_parentActivityScope_    (AS_ON),
    p_currentActivityScope_   (AS_ON)
//    parentActivityScope_    (ActivityScope(AS_ON | AS_CLIENT_ONLY)),
//    currentActivityScope_   (ActivityScope(AS_ON | AS_CLIENT_ONLY))
{
    p_parameters_ = new Parameters(this);

    // tie into Object hierarchy
    // NOTE: Has not been tested yet, and is actually never used
    if (auto o = qobject_cast<Object*>(parent))
    {
        setParentObject_(o);
    }
}

Object::~Object()
{
    delete p_parameters_;
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
    io.writeHeader("mo-tree", 2);

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
    params()->serialize(io);

    io.endSkip(startPos);

    // write childs
    io << (qint32)p_childObjects_.size();
    for (auto o : p_childObjects_)
        o->serializeTree(io);

    // v2
    // serialize after child objects
    auto future = io.reserveFutureValueInt();
    bool did = serializeAfterChilds(io);
    io.writeFutureValue(future, qint64(did ? 1 : 0));
}

Object * Object::deserializeTree(IO::DataStream & io)
{
    Object * obj = p_deserializeTree_(io);

    if (Scene * scene = qobject_cast<Scene*>(obj))
        scene->updateTree_();

    return obj;
}

Object * Object::p_deserializeTree_(IO::DataStream & io)
{
    MO_DEBUG_IO("Object::deserializeTree_()");

    // read default header
    const auto ver = io.readHeader("mo-tree", 2);

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
            o->params()->deserialize(io);
            o->onParametersLoaded();
            o->updateParameterVisibility();
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
    o->p_idName_ = o->p_orgIdName_ = idName;
    o->p_name_ = name;

    // iterate over childs
    quint32 numChilds;
    io >> numChilds;

    for (quint32 i=0; i<numChilds; ++i)
    {
        Object * child = deserializeTree(io);
        MO_ASSERT(child, "duh?");
        o->addObject_(child);//, ObjectFactory::getBestInsertIndex(o, child, -1));
    }

    if (ver >= 2)
    {
        qint64 did;
        io >> did;
        if (did)
            o->deserializeAfterChilds(io);
    }

    return o;
}

void Object::serialize(IO::DataStream & io) const
{
    io.writeHeader("obj", 2);

    io << p_canBeDeleted_;

    // v2
    io << p_attachedData_;
}

void Object::deserialize(IO::DataStream & io)
{
    const auto ver = io.readHeader("obj", 2);

    io >> p_canBeDeleted_;

    if (ver >= 2)
        io >> p_attachedData_;
}


// --------------- info ----------------------------

void Object::dumpTreeIds(std::ostream &out, const std::string& prefix) const
{
    out << prefix << idName() << std::endl;

    for (const Object * c : p_childObjects_)
        c->dumpTreeIds(out, " " + prefix);
}

int Object::objectPriority(const Object *o)
{
    if (o->isTransformation())
        return 3;
    if (o->isModulatorObject())
        return 2;
    if (o->isAudioUnit() || o->isAudioObject())
        return 1;
    return 0;
}

Object * Object::findContainingObject(const int typeFlags)
{
    if (type() & typeFlags)
        return this;

    // find parent that matches
    Object * p = parentObject();
    while (p)
    {
        if (p->type() & typeFlags)
            return p;

        p = p->parentObject();
    }

    return 0;
}

const Object * Object::findContainingObject(const int typeFlags) const
{
    if (type() & typeFlags)
        return this;

    // find parent that matches
    Object * p = parentObject();
    while (p)
    {
        if (p->type() & typeFlags)
            return p;

        p = p->parentObject();
    }

    return 0;
}


bool Object::containsTypes(const int typeFlags) const
{
    if (type() & typeFlags)
        return true;

    for (auto c : childObjects())
        if (c->containsTypes(typeFlags))
            return true;

    return false;
}

bool Object::containsObject(Object * o) const
{
    if (this == o)
        return true;

    for (auto c : childObjects())
        if (c->containsObject(o))
            return true;

    return false;
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
    for (auto p : params()->parameters())
        if (p->isModulated())
            return true;

    return false;
}

bool Object::isAudioRelevant() const
{
    if (numberMicrophones() || numberSoundSources())
        return true;

    for (auto c : p_childObjects_)
        if (c->isAudioRelevant())
            return true;

    return false;
}

QString Object::makeUniqueName(const QString &name) const
{
    QSet<QString> names;
    for (auto c : childObjects())
        names.insert(c->name());

    QString ret(name);
    while (names.contains(ret))
        increase_id_number(ret, 1);

    return ret;
}

void Object::setAttachedData(const QVariant &value, DataType type, const QString &id)
{
    // remove entry
    if (value.isNull())
    {
        auto i = p_attachedData_.find(id);
        if (i == p_attachedData_.end())
            return;

        auto map = &(*i);
        auto j = map->find(type);
        if (j != map->end())
            map->erase(j);
        return;
    }

    // create entry
    auto i = p_attachedData_.find(id);
    if (i == p_attachedData_.end())
        i = p_attachedData_.insert(id, QMap<qint64, QVariant>());

    auto map = &(*i);
    map->insert(type, value);
}

QVariant Object::getAttachedData(DataType type, const QString &id) const
{
    // remove entry
    auto i = p_attachedData_.find(id);
    if (i == p_attachedData_.end())
        return QVariant();

    auto map = &(*i);
    auto j = map->find(type);
    if (j == map->end())
        return QVariant();

    return j.value();
}

bool Object::hasAttachedData(DataType type, const QString &id) const
{
    // remove entry
    auto i = p_attachedData_.find(id);
    if (i == p_attachedData_.end())
        return false;

    auto map = &(*i);
    auto j = map->find(type);
    return (j != map->end());
}

#ifdef QT_DEBUG
void Object::dumpAttachedData() const
{
    qDebug() << "----- attached data for" << idName() << "/" << name();
    for (auto i = p_attachedData_.begin(); i != p_attachedData_.end(); ++i)
    {
        qDebug() << "-- data id" << i.key();
        for (auto j = i.value().begin(); j != i.value().end(); ++j)
        {
            qDebug() << "  " << j.key() << " " << j.value();
        }
    }
}
#endif


Object::ActivityScope Object::activityScope() const
{
    if (p_paramActiveScope_)
        return (ActivityScope)(
                    p_paramActiveScope_->baseValue() & p_parentActivityScope_);
    else
        return p_parentActivityScope_;
}

void Object::p_passDownActivityScope_(ActivityScope parent_scope)
{
    ActivityScope scope = parent_scope;
    if (p_paramActiveScope_)
        scope = (ActivityScope)(scope & p_paramActiveScope_->baseValue());

    for (auto c : p_childObjects_)
    {
        c->p_parentActivityScope_ = scope;
        c->p_passDownActivityScope_(scope);
    }
}

bool Object::active(Double /*time*/, uint /*thread*/) const
{
    // XXX active parameter not there yet
    return activityScope() & p_currentActivityScope_;
}

bool Object::activeAtAll() const
{
    return activityScope() & p_currentActivityScope_;
}

// ------------------ setter -----------------------

void Object::setName(const QString & n)
{
    p_name_ = n;
}

void Object::setCurrentActivityScope(ActivityScope scope)
{
    p_currentActivityScope_ = scope;

    // currentActivityScope_ = ActivityScope(currentActivityScope_ | AS_CLIENT_ONLY);

    for (auto o : p_childObjects_)
        o->setCurrentActivityScope(scope);
}

// ------------- tree stuff ------------------------

const Object * Object::rootObject() const
{
    return p_parentObject_ ? p_parentObject_->rootObject() : this;
}

Object * Object::rootObject()
{
    return p_parentObject_ ? p_parentObject_->rootObject() : this;
}

const Scene * Object::sceneObject() const
{
    return qobject_cast<const Scene*>(rootObject());
}

Scene * Object::sceneObject()
{
    return qobject_cast<Scene*>(rootObject());
}

ObjectEditor * Object::editor() const
{
    if (auto s = sceneObject())
        return s->editor();
    else
        return 0;
}

int Object::numChildren(bool recursive) const
{
    if (!recursive)
        return childObjects().size();

    int n = childObjects().size();
    for (auto o : p_childObjects_)
        n += o->numChildren(true);
    return n;
}

bool Object::hasParentObject(Object *o) const
{
    if (!p_parentObject_)
        return false;

    return p_parentObject_ == o? true : p_parentObject_->hasParentObject(o);
}

Object * Object::findParentObject(int tflags) const
{
    if (!p_parentObject_)
        return 0;

    return p_parentObject_->type() & tflags ? p_parentObject_ : p_parentObject_->findParentObject(tflags);
}

Object * Object::findCommonParentObject(Object *other) const
{
    // get list of parents
    QList<Object*> par;
    Object * o = parentObject();
    while (o) { par << o; o = o->parentObject(); }

    // get list of other's parents
    QSet<Object*> opar;
    o = other->parentObject();
    while (o) { opar << o; o = o->parentObject(); }

    // find commons
    for (auto o : par)
        if (opar.contains(o))
            return o;

    return 0;
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

    MO_ASSERT(p_parentObject_ != parent, "trying to add object to same parent");
    MO_ASSERT(!hasParentObject(this), "trying to add object to it's own hierarchy");
    MO_ASSERT(parent->canHaveChildren(type()), "invalid child '" << idName() << "' "
                              "for object '" << parent->idName() << "'");

    // silently ignore in release mode
    if (parent == p_parentObject_ || hasParentObject(this))
        return;

    // install in QObject tree (handle memory)
    setParent(parent);

    // remove from previous parent
    if (p_parentObject_)
    {
        p_parentObject_->p_takeChild_(this);
    }
    p_parentObject_ = 0;

    // adjust idnames in new subtree
    p_makeUniqueIds_(parent->rootObject());

    // assign
    p_parentObject_ = parent;

    // and add to child list
    p_parentObject_->p_addChildObjectHelper_(this, index);

    // signal this object
    this->onParentChanged();
}


Object * Object::addObject_(Object * o, int index)
{
    MO_ASSERT(o, "trying to add a NULL child");
    MO_ASSERT(!p_childObjects_.contains(o), "duplicate addChild for '" << o->idName());

    o->setParentObject_(this, index);

    p_childrenHaveChanged_ = true;
    return o;
}

Object * Object::p_addChildObjectHelper_(Object * o, int index)
{
    MO_ASSERT(o, "trying to add a NULL child");

    if (index < 0)
        p_childObjects_.append(o);
    else
        p_childObjects_.insert(index, o);

    p_childrenHaveChanged_ = true;
    return o;
}

void Object::deleteObject_(Object * child, bool destroy)
{
    if (p_takeChild_(child))
    {
        child->setParent(0);
        if (destroy)
            delete child;
        p_childrenHaveChanged_ = true;
    }
}

bool Object::p_takeChild_(Object *child)
{
    if (p_childObjects_.removeOne(child))
    {
        return p_childrenHaveChanged_ = true;
    }
    return false;
}

void Object::swapChildren_(int from, int to)
{
    if (from >= 0 && from < numChildren()
        && to >= 0 && to < numChildren())

    p_childObjects_.swap(from, to);
    p_childrenHaveChanged_ = true;
}

bool Object::setChildrenObjectIndex_(Object *child, int newIndex)
{
    auto idx = p_childObjects_.indexOf(child);
    if (idx < 0)
        return false;

    //MO_DEBUG("move " << idx << " to " << newIndex);

    if (newIndex > 0
        // dont move before self or to same position
        && (newIndex == idx || newIndex == (idx+1)))
        return false;

    // if we delete 'child' before new insert position
    if (newIndex > idx)
        --newIndex;

    // remove at old location
    p_childObjects_.removeAt(idx);
    // put at new location
    if (newIndex >= 0)
        p_childObjects_.insert(newIndex, child);
    else
        p_childObjects_.append(child);

    return true;
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

void Object::p_makeUniqueIds_(Object * root)
{
    // get all existing ids
    QSet<QString> existing = root->getChildIds(true);
    existing.insert(root->idName());
    // ignore self when searching own tree
    if (root == rootObject())
        existing.remove(idName());

    // call string version
    p_makeUniqueIds_(existing);
}

void Object::p_makeUniqueIds_(QSet<QString> &existing)
{
    bool changed = false;

    // make this object's id unique
    p_idName_ = getUniqueId(p_idName_, existing, &changed);

    // add to existing ids
    if (changed)
        existing.insert(p_idName_);

    // go through childs
    for (auto o : p_childObjects_)
        o->p_makeUniqueIds_(existing);
}

Object * Object::findChildObject(const QString &id, bool recursive, Object * ignore) const
{
    for (auto o : p_childObjects_)
        if (o != ignore && o->idName() == id)
            return o;

    if (recursive)
        for (auto i : p_childObjects_)
            if (Object * o = i->findChildObject(id, recursive, ignore))
                return o;

    return 0;
}

QList<Object*> Object::findChildObjects(int typeFlags, bool recursive) const
{
    QList<Object*> list;
    for (auto o : p_childObjects_)
        if (o->type() & typeFlags)
            list.append(o);

    if (recursive)
        for (auto o : p_childObjects_)
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

    // XXX test123: Let's place sequences everywhere
    if (t & TG_SEQUENCE)
        return true;

    // Clips belong into ClipContainer ...
    if (t == T_CLIP)
        return type() == T_CLIP_CONTAINER;

    // and nothing else
    if (type() == T_CLIP_CONTAINER)
        return t == T_CLIP;

    // XXX Currently ClipContainer only into scene
    if (t == T_CLIP_CONTAINER)
        return type() == T_SCENE;

    // Clips only contain sequences
    if (type() == T_CLIP)
        return t & TG_SEQUENCE;

    // microphone groups only contain microphones
    if (type() == T_MICROPHONE_GROUP)
        return t == T_MICROPHONE;

    // audio-units can be part of scene and any object
    if (t == T_AUDIO_OBJECT)
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

void Object::p_childrenChanged_()
{
    MO_DEBUG_TREE("Object('" << idName() << "')::childrenChanged_()");

    // collect special sub-objects
    p_collectTransformationObjects_();

    // notify derived classes
    childrenChanged();

    p_passDownActivityScope_(activityScope());

    p_childrenHaveChanged_ = false;
}


void Object::onObjectsAboutToDelete(const QList<Object *> & list)
{
    for (Parameter * p : params()->parameters())
    {
        for (const Object * o : list)
            p->removeAllModulators(o->idName());
    }
}


void Object::propagateRenderMode(ObjectGl *parent)
{
    for (auto c : p_childObjects_)
        c->propagateRenderMode(parent);
}

bool Object::verifyNumberThreads(uint num)
{
    if (p_numberThreads_ != num)
        return false;

    return true;
}

void Object::setNumberThreads(uint num)
{
    MO_DEBUG_TREE("Object('" << idName() << "')::setNumberThreads(" << num << ")");

    p_numberThreads_ = num;
}

void Object::getIdMap(QMap<QString, Object *> &idMap) const
{
    idMap.insert(idName(), const_cast<Object*>(this));
    for (auto c : childObjects())
        c->getIdMap(idMap);
}


// ------------------------- 3d -----------------------

void Object::p_collectTransformationObjects_()
{
    p_transformationObjects_.clear();

    for (auto o : p_childObjects_)
        if (auto t = qobject_cast<Transformation*>(o))
            p_transformationObjects_.append(t);
}

void Object::calculateTransformation(Mat4 &matrix, Double time, uint thread) const
{
    for (auto t : p_transformationObjects_)
        if (t->active(time, thread))
            t->applyTransformation(matrix, time, thread);
}




// ---------------------- parameter --------------------------

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

    params()->beginParameterGroup("active", tr("activity"));

    p_paramActiveScope_ =
    params()->createSelectParameter("_activescope", tr("activity scope"),
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

    params()->endParameterGroup();
}

void Object::onParameterChanged(Parameter * p)
{
    MO_DEBUG_PARAM("Object::parameterChanged('" << p->idName() << "')");

    // activity scope changed
    if (p == p_paramActiveScope_)
        p_passDownActivityScope_(activityScope());

}




// ----------------- audio sources ---------------------

void Object::setSampleRate(uint samplerate)
{
    MO_ASSERT(samplerate>0, "bogus samplerate");

    p_sampleRate_ = std::max((uint)1, samplerate);
    p_sampleRateInv_ = 1.0 / p_sampleRate_;
}

void Object::setNumberSoundSources(uint num)
{
    p_numberSoundSources_ = num;
}

void Object::setNumberMicrophones(uint num)
{
    p_numberMicrophones_ = num;
}

void Object::calculateSoundSourceTransformation(
        const TransformationBuffer * objectTransform,
        const QList<AUDIO::SpatialSoundSource *> list,
        uint , SamplePos , uint )
{
    MO_ASSERT(list.size() == (int)numberSoundSources(), "number of sound sources does not match "
              << list.size() << "/" << numberSoundSources());

    for (auto s : list)
        TransformationBuffer::copy(objectTransform, s->transformationBuffer());
}

void Object::calculateMicrophoneTransformation(
        const TransformationBuffer * objectTransform,
        const QList<AUDIO::SpatialMicrophone*> list,
        uint , SamplePos , uint )
{
    MO_ASSERT(list.size() == (int)numberMicrophones(), "number of microphones does not match "
              << list.size() << "/" << numberMicrophones());

    for (auto m : list)
        TransformationBuffer::copy(objectTransform, m->transformationBuffer());
}

// -------------------- modulators ---------------------

QList<Modulator*> Object::getModulators() const
{
    QList<Modulator*> mods;
    for (auto p : params()->parameters())
    {
        mods.append( p->modulators() );
    }
    return mods;
}

QList<Object*> Object::getModulatingObjects() const
{
    QList<Object*> list;

    for (auto p : params()->parameters())
    {
        list.append(p->getModulatingObjects());
    }

    return list;
}

QList<Object*> Object::getFutureModulatingObjects(const Scene *scene) const
{
    QList<Object*> list;

    for (auto p : params()->parameters())
    {
        list.append(p->getFutureModulatingObjects(scene));
    }

    return list;
}

QList<QPair<Parameter*, Object*>> Object::getModulationPairs() const
{
    QList<QPair<Parameter*, Object*>> pairs;

    for (auto p : params()->parameters())
    {
        const QList<Object*> list = p->getModulatingObjects();

        for (auto o : list)
            pairs.append(qMakePair(p, o));
    }

    return pairs;
}

void Object::setOutputIds(const QStringList & ids)
{
    p_outputIds_ = ids;
    if (editor())
        emit editor()->objectChanged(this);
}

} // namespace MO
