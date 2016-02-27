/** @file object.cpp

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/


#include <QDebug>

#include "object.h"
#include "util/objectfactory.h"
#include "scene.h"
#include "control/modulatorobjectfloat.h"
#include "transform/transformation.h"
#include "param/parameters.h"
#include "param/parameterselect.h"
#include "param/parameterfloat.h"
#include "audio/spatial/spatialsoundsource.h"
#include "audio/spatial/spatialmicrophone.h"
#include "util/objecteditor.h"
#include "util/audioobjectconnections.h"
#include "util/objectconnectiongraph.h"
#include "util/parameterevolution.h"
#include "math/transformationbuffer.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "tool/stringmanip.h"

namespace MO {


struct Object::PrivateObj
{
    PrivateObj(Object* o)
        : object            (o)
        , p_parent_                 (0)
        , p_parameters_             (new Parameters(object))
        , p_paramEvo_               (0)
        , p_paramActiveScope_       (0)
        , p_paramActive_            (0)
        , p_canBeDeleted_           (true)
        , p_visible_                (true)
        , p_parentObject_           (0)
        , p_childrenHaveChanged_    (false)
        , p_numberThreads_          (1)
        , p_numberSoundSources_     (0)
        , p_numberMicrophones_      (0)
        , p_sampleRate_             (44100)
        , p_sampleRateInv_          (1.0/44100.0)
        , p_aoCons_                 (0)
        , p_parentActivityScope_    (AS_ON)
        , p_currentActivityScope_   (AS_ON)
    //  , parentActivityScope_    (ActivityScope(AS_ON | AS_CLIENT_ONLY))
    //  , currentActivityScope_   (ActivityScope(AS_ON | AS_CLIENT_ONLY))
    { }

    ~PrivateObj()
    {
        delete p_paramEvo_;
        delete p_aoCons_;
        delete p_parameters_;

        // release references on childs
        for (auto c : p_childObjects_)
            c->releaseRef();
    }

    /** Implementation of Object::deserializeTree() */
    static Object * p_deserializeTree_(IO::DataStream&);

    /** Adds the object to child list, nothing else */
    Object * p_addChildObjectHelper_(Object * object, int insert_index = -1);

    /** Fills the transformationChilds() array */
    void p_collectTransformationObjects_();


    Object * object;

    // ------------- tree --------------------

    Object* p_parent_;
    std::vector<Object*> p_children_;

    // ---------- parameter s-----------------

    Parameters * p_parameters_;
    mutable ParameterEvolution* p_paramEvo_;

    void p_passDownActivityScope_(ActivityScope parent_scope);

    // --------- default parameters ----------

    ParameterSelect * p_paramActiveScope_;
    ParameterFloat * p_paramActive_;

    // ------------ properties ---------------

    QString p_idName_, p_name_;

    bool p_canBeDeleted_, p_visible_;

    QMap<QString, QMap<qint64, QVariant>> p_attachedData_;

    // ----------- tree ----------------------

    Object * p_parentObject_;
    QList<Object*> p_childObjects_;
    QList<Transformation*> p_transformationObjects_;
    bool p_childrenHaveChanged_;

    // ---------- outputs --------------------

    QMap<SignalType, uint> p_outputMap_;

    // ---------- per-thread store -----------

    uint p_numberThreads_;

    // ------------ audio --------------------

    // requested count
    uint p_numberSoundSources_,
         p_numberMicrophones_;

    uint p_sampleRate_;
    Double p_sampleRateInv_;

    AudioObjectConnections * p_aoCons_;

    // ------------ runtime ------------------

    ActivityScope
    /** activity scope passed down from parents */
        p_parentActivityScope_,
    /** current requested activity scope */
        p_currentActivityScope_;

    // ----------- position ------------------

    /** @deprecated */
    Mat4 p_transformation_;

    /** Used for deserialization errors */
    QString
    /** Used during object creation/initialization */
            p_errorStr_,
            p_ioLoadErrorStr_;
};


bool ObjectPrivate::registerObject(Object * obj)
{
    return ObjectFactory::registerObject(obj);
}

void ObjectPrivate::setObjectId(Object * o, const QString& id)
{
    o->pobj_->p_idName_ = id;
}

void ObjectPrivate::addObject(Object* parent, Object* newChild, int index)
{
    parent->addObject_(newChild, index);
}

void ObjectPrivate::deleteObject(Object* o)
{
    if (o->parentObject())
        o->parentObject()->deleteObject_(o, true);
}

void ObjectPrivate::deleteChildren(Object* o)
{
    for (auto c : o->childObjects())
        o->deleteObject_(c, true);
}



Object::Object()
    : pobj_         (new PrivateObj(this))
{

    addEvolutionKey(tr("Parameters"));
}

Object::~Object()
{
    MO_DEBUG("Object(\"" << namePath() << "\")::~Object()");

#ifndef NDEBUG
    if (referenceCount() > 1)
    {
        MO_WARNING("Object(" << idName() << ")::~Object() with a ref-count of "
                   << referenceCount() << ".");
    }
#endif
    delete pobj_;
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
    io << (qint32)pobj_->p_childObjects_.size();
    for (auto o : pobj_->p_childObjects_)
        o->serializeTree(io);

    // v2
    // serialize after child objects
    auto future = io.reserveFutureValueInt();
    bool did = serializeAfterChilds(io);
    io.writeFutureValue(future, qint64(did ? 1 : 0));
}

Object * Object::deserializeTree(IO::DataStream & io)
{
    Object * obj = PrivateObj::p_deserializeTree_(io);

    if (Scene * scene = dynamic_cast<Scene*>(obj))
    {
        scene->updateTree_();
        //scene->clearNullModulators(true);
    }

    return obj;
}

QString Object::getIoLoadErrors() const
{
    QString e = pobj_->p_ioLoadErrorStr_;
    for (auto c : childObjects())
    {
        auto ce = c->getIoLoadErrors().trimmed();
        if (ce.size() > 1)
            e += "\n" + ce;
    }

    return e;
}

Object * Object::PrivateObj::p_deserializeTree_(IO::DataStream & io)
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
            o->releaseRef();
            e << "\nobject creation failed for class '" << className << "'";
            throw;
        }

        // once in a while check stream for errors
        if (io.status() != QDataStream::Ok)
        {
            o->releaseRef();
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
            o->releaseRef();
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
        name = name + " *missing*";

        o->pobj_->p_ioLoadErrorStr_ += tr("unknown object of type '%1'\n")
                                .arg(className);
    }

    // set default object info
    o->pobj_->p_idName_ = idName;
    o->pobj_->p_name_ = name;

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
    io.writeHeader("obj", 4);

    io << pobj_->p_canBeDeleted_;

    // v2/4
    io << pobj_->p_attachedData_;
    // v3
    io << pobj_->p_visible_;
}

void Object::deserialize(IO::DataStream & io)
{
    const auto ver = io.readHeader("obj", 4);

    io >> pobj_->p_canBeDeleted_;

    if (ver >= 2)
        io >> pobj_->p_attachedData_;
    // audio objects pre v4 where collapsed by default
    if (ver < 4 && isAudioObject())
        setAttachedData(QVariant(), DT_GRAPH_EXPANDED);

    if (ver >= 3)
        io >> pobj_->p_visible_;
}


// --------------- info ----------------------------

void Object::dumpTreeIds(std::ostream &out, const std::string& prefix) const
{
    out << prefix << idName() << std::endl;

    for (const Object * c : pobj_->p_childObjects_)
        c->dumpTreeIds(out, " " + prefix);
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

const QString& Object::idName() const { return pobj_->p_idName_; }
const QString& Object::name() const { return pobj_->p_name_; }
QString Object::infoName() const { return pobj_->p_name_; }
bool Object::isVisible() const { return pobj_->p_visible_; }
bool Object::canBeDeleted() const { return pobj_->p_canBeDeleted_; }

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

    for (auto c : pobj_->p_childObjects_)
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
    ret.replace("/", "");
    ret.replace("\\", "");
    while (names.contains(ret))
        increase_id_number(ret, 1);

    return ret;
}

void Object::setAttachedData(const QVariant &value, DataType type, const QString &id)
{
    // remove entry
    if (value.isNull())
    {
        auto i = pobj_->p_attachedData_.find(id);
        if (i == pobj_->p_attachedData_.end())
            return;

        auto map = &(*i);
        auto j = map->find(type);
        if (j != map->end())
            map->erase(j);
        return;
    }

    // create entry
    auto i = pobj_->p_attachedData_.find(id);
    if (i == pobj_->p_attachedData_.end())
        i = pobj_->p_attachedData_.insert(id, QMap<qint64, QVariant>());

    auto map = &(*i);
    map->insert(type, value);
}

QVariant Object::getAttachedData(DataType type, const QString &id) const
{
    // remove entry
    auto i = pobj_->p_attachedData_.find(id);
    if (i == pobj_->p_attachedData_.end())
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
    auto i = pobj_->p_attachedData_.find(id);
    if (i == pobj_->p_attachedData_.end())
        return false;

    auto map = &(*i);
    auto j = map->find(type);
    return (j != map->end());
}

#ifdef QT_DEBUG
void Object::dumpAttachedData() const
{
    qDebug() << "----- attached data for" << idName() << "/" << name();
    for (auto i = pobj_->p_attachedData_.begin();
         i != pobj_->p_attachedData_.end(); ++i)
    {
        qDebug() << "-- data id" << i.key();
        for (auto j = i.value().begin(); j != i.value().end(); ++j)
        {
            qDebug() << "  " << j.key() << " " << j.value();
        }
    }
}
#endif


QColor Object::color() const
{
    return ObjectFactory::colorForObject(this);
}

void Object::setActivityScope(ActivityScope scope, bool sendGui)
{
    // XXX forgot what this is used for..
    pobj_->p_currentActivityScope_ = scope;

    if (pobj_->p_paramActiveScope_)
    {
        if (sendGui)
        {
            if (auto e = editor())
            {
                e->setParameterValue(pobj_->p_paramActiveScope_, scope);
                return;
            }
        }
        pobj_->p_paramActiveScope_->setValue(scope);
    }

    pobj_->p_passDownActivityScope_(activityScope());
}

Object::ActivityScope Object::activityScope() const
{
    if (pobj_->p_paramActiveScope_)
        return (ActivityScope)(
                    pobj_->p_paramActiveScope_->baseValue()
                            & pobj_->p_parentActivityScope_);
    else
        return pobj_->p_parentActivityScope_;
}

Object::ActivityScope Object::currentActivityScope() const
{ return pobj_->p_currentActivityScope_; }


void Object::PrivateObj::p_passDownActivityScope_(ActivityScope parent_scope)
{
    ActivityScope scope = parent_scope;
    if (p_paramActiveScope_)
        scope = (ActivityScope)(scope & p_paramActiveScope_->baseValue());

    for (auto c : p_childObjects_)
    {
        c->pobj_->p_parentActivityScope_ = scope;
        c->pobj_->p_passDownActivityScope_(scope);
    }
}

bool Object::active(const RenderTime& time) const
{
    return (activityScope() & pobj_->p_currentActivityScope_)
            && (!pobj_->p_paramActive_ || pobj_->p_paramActive_->value(time) > 0.);
}

bool Object::activeAtAll() const
{
    return activityScope() & pobj_->p_currentActivityScope_;
}

// ------------------ setter -----------------------

void Object::setName(const QString & n)
{
    pobj_->p_name_ = makeUniqueName(n);
}

void Object::setVisible(bool v) { pobj_->p_visible_ = v; }


void Object::setCurrentActivityScope(ActivityScope scope)
{
    pobj_->p_currentActivityScope_ = scope;

    // currentActivityScope_ = ActivityScope(currentActivityScope_ | AS_CLIENT_ONLY);

    for (auto o : pobj_->p_childObjects_)
        o->setCurrentActivityScope(scope);
}

// ------------- tree stuff ------------------------

Object * Object::parentObject() const { return pobj_->p_parentObject_; }

const Object * Object::rootObject() const
{
#if 0
    return pobj_->p_parentObject_ ? pobj_->p_parentObject_->rootObject() : this;
#else
    auto root = pobj_->p_parentObject_;
    if (!root)
        return this;
    while (root && root->pobj_->p_parentObject_)
        root = root->pobj_->p_parentObject_;
    return root;
#endif
}

Object * Object::rootObject()
{
#if 0
    return pobj_->p_parentObject_ ? pobj_->p_parentObject_->rootObject() : this;
#else
    auto root = pobj_->p_parentObject_;
    if (!root)
        return this;
    while (root && root->pobj_->p_parentObject_)
        root = root->pobj_->p_parentObject_;
    return root;
#endif
}

const Scene * Object::sceneObject() const
{
    return dynamic_cast<const Scene*>(rootObject());
}

Scene * Object::sceneObject()
{
    return dynamic_cast<Scene*>(rootObject());
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
    for (auto o : pobj_->p_childObjects_)
        n += o->numChildren(true);
    return n;
}

bool Object::hasParentObject(Object *o) const
{
    if (!pobj_->p_parentObject_)
        return false;

    return pobj_->p_parentObject_ == o? true : pobj_->p_parentObject_->hasParentObject(o);
}

Object * Object::findParentObject(int tflags) const
{
    if (!pobj_->p_parentObject_)
        return 0;

    return pobj_->p_parentObject_->type() & tflags ? pobj_->p_parentObject_ : pobj_->p_parentObject_->findParentObject(tflags);
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

Object * Object::findChildObject(std::function<bool (Object *)> selector)
{
    if (selector(this))
        return this;
    for (auto c : childObjects())
        if (auto o = c->findChildObject(selector))
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

    if (childObjects().contains(o))
    {
        error = tr("Object '%1' is already a children of '%2'.").arg(o->name()).arg(name());
        return false;
    }

    if (hasParentObject(o) || o == this)
    {
        error = tr("Trying to add '%1' to itself").arg(o->name());
        return false;
    }

    if (!canHaveChildren(o->type()))
    {
        error = tr("'%1' can not have a child of this type").arg(name());
        return false;
    }

    // test for singleton clipcontroller
    if (o->isClipController())
        if (auto s = sceneObject())
            if (s->clipController())
            {
                error = tr("Only one clipcontainer allowed per scene");
                return false;
            }

    // test for modulation loops
    ObjectConnectionGraph graph;
    o->getFutureModulatingObjects(graph, const_cast<Scene*>(sceneObject()), true);

    if (graph.hasObject(const_cast<Object*>(this)))
    {
        error = tr("Adding '%1' as a child to '%2' would cause an infinite "
                   "modulation loop!").arg(o->name()).arg(name());
        return false;
    }

    return true;
}


void Object::setParentObject_(Object *parent, int index)
{
    MO_ASSERT(parent, "NULL parent given for Object");

    MO_DEBUG_TREE("Object("<<idName()<<")::SetParentObject("<<parent->idName()<<")");

    MO_ASSERT(pobj_->p_parentObject_ != parent, "trying to add object to same parent");
    MO_ASSERT(!hasParentObject(this), "trying to add object to it's own hierarchy");
    MO_ASSERT(parent->canHaveChildren(type()), "invalid child '" << idName() << "' "
                              "for object '" << parent->idName() << "'");

    // silently ignore in release mode
    if (parent == pobj_->p_parentObject_ || hasParentObject(this))
        return;

    // install in QObject tree (handle memory)
    /** @todo newobj setParent(parent); */
    addRef();

    // remove from previous parent
    if (pobj_->p_parentObject_)
    {
        pobj_->p_parentObject_->p_takeChild_(this);
    }
    pobj_->p_parentObject_ = 0;

    // adjust idnames in new subtree
    ObjectEditor::makeUniqueIds(parent->rootObject(), this);

    // assign
    pobj_->p_parentObject_ = parent;

    // and add to child list
    pobj_->p_parentObject_->pobj_->p_addChildObjectHelper_(this, index);

    // signal this object
    this->onParentChanged();
}


Object * Object::addObject_(Object * o, int index)
{
    MO_ASSERT(o, "trying to add a NULL child");
    MO_ASSERT(!pobj_->p_childObjects_.contains(o), "duplicate addChild for '" << o->idName());

    o->setParentObject_(this, index);

    pobj_->p_childrenHaveChanged_ = true;
    return o;
}

Object * Object::PrivateObj::p_addChildObjectHelper_(Object * o, int index)
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
        /** @todo newobj
        child->setParent(0);*/
        if (destroy)
            child->releaseRef();
        pobj_->p_childrenHaveChanged_ = true;
    }
}

bool Object::p_takeChild_(Object *child)
{
    if (pobj_->p_childObjects_.removeOne(child))
    {
        return pobj_->p_childrenHaveChanged_ = true;
    }
    return false;
}

void Object::swapChildren_(int from, int to)
{
    if (from >= 0 && from < numChildren()
        && to >= 0 && to < numChildren())

    pobj_->p_childObjects_.swap(from, to);
    pobj_->p_childrenHaveChanged_ = true;
}

bool Object::setChildrenObjectIndex_(Object *child, int newIndex)
{
    auto idx = pobj_->p_childObjects_.indexOf(child);
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
    pobj_->p_childObjects_.removeAt(idx);
    // put at new location
    if (newIndex >= 0)
        pobj_->p_childObjects_.insert(newIndex, child);
    else
        pobj_->p_childObjects_.append(child);

    return true;
}

const QList<Object*>& Object::childObjects() const { return pobj_->p_childObjects_; }


QSet<QString> Object::getChildIds(bool recursive) const
{
    QList<Object*> list = findChildObjects(TG_ALL, recursive);

    QSet<QString> ids;

    for (auto o : list)
        ids.insert(o->idName());

    return ids;
}

Object * Object::findObjectByNamePath(const QString &namePath) const
{
    bool isRootPath = namePath.startsWith('/');

    QStringList names = namePath.split('/', QString::SkipEmptyParts);
    if (names.isEmpty())
        return 0;

    return findObjectByNamePath(names, 0, isRootPath);
}

Object * Object::findObjectByNamePath(const QStringList &names, int offset, bool firstIsRoot) const
{
    if (offset >= names.length())
        return 0;

    QString name = names[offset];
    Object * obj = 0;
    for (auto c : childObjects())
    if (c->name() == name)
    {
        obj = c;
        break;
    }

    // first id should be a child of this?
    if (obj == 0 && firstIsRoot)
        return 0;

    // found id?
    if (obj)
    {
        // last in path?
        if (offset+1 == names.size())
            return obj;
        // otherwise traverse obj
        return obj->findObjectByNamePath(names, offset + 1, true);
    }

    // otherwise check all children branches
    for (auto c : childObjects())
        if (auto obj = c->findObjectByNamePath(names, offset, firstIsRoot))
            return obj;

    return 0;
}

/*
void Object::pobj_->p_makeUniqueIds_(Object * root)
{
    // get all existing ids
    QSet<QString> existing = root->getChildIds(true);
    existing.insert(root->idName());
    // ignore self when searching own tree
    if (root == rootObject())
        existing.remove(idName());

    // call string version
    pobj_->p_makeUniqueIds_(existing);
}

void Object::pobj_->p_makeUniqueIds_(QSet<QString> &existing)
{
    bool changed = false;

    // make this object's id unique
    pobj_->p_idName_ = getUniqueId(pobj_->p_idName_, existing, &changed);

    // add to existing ids
    if (changed)
        existing.insert(pobj_->p_idName_);

    // go through childs
    for (auto o : pobj_->p_childObjects_)
        o->pobj_->p_makeUniqueIds_(existing);
}
*/
Object * Object::findChildObject(const QString &id, bool recursive, Object * ignore) const
{
    for (auto o : pobj_->p_childObjects_)
        if (o != ignore && o->idName() == id)
            return o;

    if (recursive)
        for (auto i : pobj_->p_childObjects_)
            if (Object * o = i->findChildObject(id, recursive, ignore))
                return o;

    return 0;
}

QList<Object*> Object::findChildObjects(int typeFlags, bool recursive) const
{
    QList<Object*> list;
    for (auto o : pobj_->p_childObjects_)
        if (o->type() & typeFlags)
            list.append(o);

    if (recursive)
        for (auto o : pobj_->p_childObjects_)
            list.append( o->findChildObjects(typeFlags, true) );

    return list;
}

bool Object::canHaveChildren(Type t) const
{
    // dummy can contain/be added to everything
    if (t == T_DUMMY || type() == T_DUMMY)
        return true;

    // ModulatorObjects can be anywhere
    if (t & TG_MODULATOR_OBJECT)
        return true;

    // Sequences can be everywhere
    if (t & TG_SEQUENCE)
        return true;

    // Clips can be everywhere
    if (t == T_CLIP)
        return true;

    // nothing goes into clip container, except maybe clips
    if (type() == T_CLIP_CONTROLLER)
        return t == T_CLIP;

    // Clips belong into ClipContainer ...
    if (t == T_CLIP)
        return type() == T_CLIP_CONTROLLER;

    // XXX Currently ClipContainer only into scene
    if (t == T_CLIP_CONTROLLER)
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
    pobj_->p_collectTransformationObjects_();

    // notify derived classes
    childrenChanged();

    pobj_->p_passDownActivityScope_(activityScope());
    pobj_->p_childrenHaveChanged_ = false;
}


void Object::onObjectsAboutToDelete(const QList<Object *> & list)
{
    for (Parameter * p : params()->parameters())
    {
        for (const Object * o : list)
            p->removeAllModulators(o->idName());
    }
}


void Object::idNamesChanged(const QMap<QString, QString> & map)
{
    // tell parameters
    for (Parameter * p : params()->parameters())
        p->idNamesChanged(map);

    // derived code
    onIdNamesChanged(map);

    // children
    for (auto c : childObjects())
        c->idNamesChanged(map);

    // attached audio connections
    // (of pasted/loaded objects)
    if (pobj_->p_aoCons_)
        pobj_->p_aoCons_->idNamesChanged(map);
}

void Object::propagateRenderMode(ObjectGl *parent)
{
    for (auto c : pobj_->p_childObjects_)
        c->propagateRenderMode(parent);
}

uint Object::numberThreads() const { return pobj_->p_numberThreads_; }

bool Object::verifyNumberThreads(uint num)
{
    if (pobj_->p_numberThreads_ != num)
        return false;

    return true;
}

void Object::setNumberThreads(uint num)
{
    MO_DEBUG_TREE("Object('" << idName() << "')::setNumberThreads(" << num << ")");

    pobj_->p_numberThreads_ = num;
}

void Object::getIdMap(QMap<QString, Object *> &idMap) const
{
    idMap.insert(idName(), const_cast<Object*>(this));
    for (auto c : childObjects())
        c->getIdMap(idMap);
}

bool Object::haveChildrenChanged() const
{
    return pobj_->p_childrenHaveChanged_;
}

// ------------------------- 3d -----------------------

void Object::clearTransformation() { pobj_->p_transformation_ = Mat4(1); }
void Object::setTransformation(const Mat4& mat) { pobj_->p_transformation_ = mat; }

const Mat4& Object::transformation() const { return pobj_->p_transformation_; }
const QList<Transformation*>& Object::transformationObjects() const
    { return pobj_->p_transformationObjects_; }

Mat4 Object::valueTransformation(
        uint /*channel*/, const RenderTime& /*time*/) const
{ return pobj_->p_transformation_; }

/** Returns the position of this object */
Vec3 Object::position() const
    { return Vec3(pobj_->p_transformation_[3][0],
                  pobj_->p_transformation_[3][1],
                  pobj_->p_transformation_[3][2]); }


void Object::PrivateObj::p_collectTransformationObjects_()
{
    p_transformationObjects_.clear();

    for (auto o : p_childObjects_)
        if (auto t = dynamic_cast<Transformation*>(o))
            p_transformationObjects_.append(t);
}

void Object::calculateTransformation(Mat4 &matrix, const RenderTime& time) const
{
    for (auto t : pobj_->p_transformationObjects_)
        if (t->active(time))
            t->applyTransformation(matrix, time);
}




// ---------------------- parameter --------------------------

const Parameters * Object::params() const { return pobj_->p_parameters_; }
Parameters * Object::params() { return pobj_->p_parameters_; }

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

        pobj_->p_paramActiveScope_ =
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
        pobj_->p_paramActiveScope_->setDefaultEvolvable(false);

        /// @todo live activity parameter should apply to children as well!
        pobj_->p_paramActive_ = params()->createFloatParameter("_active_f", tr("active"),
                            tr("A value greater than 0.0 makes the object active"
                               " - this does CURRENTLY NOT apply to child objects"),
                            1., 1.);
        pobj_->p_paramActive_->setDefaultEvolvable(false);

    params()->endParameterGroup();
}

void Object::onParameterChanged(Parameter * p)
{
    MO_DEBUG_PARAM("Object::parameterChanged('" << p->idName() << "')");

    // activity scope changed
    if (p == pobj_->p_paramActiveScope_)
        pobj_->p_passDownActivityScope_(activityScope());

}


void Object::initParameterGroupExpanded(const QString &groupId, bool expanded)
{
    if (!hasAttachedData(DT_PARAM_GROUP_EXPANDED, groupId))
        setAttachedData(expanded, DT_PARAM_GROUP_EXPANDED, groupId);
}

// ----------------- audio sources ---------------------

uint Object::sampleRate() const { return pobj_->p_sampleRate_; }
Double Object::sampleRateInv() const { return pobj_->p_sampleRateInv_; }
uint Object::numberSoundSources() const { return pobj_->p_numberSoundSources_; }
uint Object::numberMicrophones() const { return pobj_->p_numberMicrophones_; }

void Object::setSampleRate(uint samplerate)
{
    MO_ASSERT(samplerate>0, "bogus samplerate");

    pobj_->p_sampleRate_ = std::max((uint)1, samplerate);
    pobj_->p_sampleRateInv_ = 1.0 / pobj_->p_sampleRate_;
}

void Object::setNumberSoundSources(uint num)
{
    pobj_->p_numberSoundSources_ = num;
    auto e = editor();
    if (e)
        emit e->audioConnectionsChanged();
}

void Object::setNumberMicrophones(uint num)
{
    /** @todo update audio dsp path on soundSource/microphone change. */
    pobj_->p_numberMicrophones_ = num;
    if (auto e = editor())
        emit e->audioConnectionsChanged();
}

void Object::calculateSoundSourceTransformation(const TransformationBuffer * objectTransform,
        const QList<AUDIO::SpatialSoundSource*>& list,
        const RenderTime&)
{
    MO_ASSERT(list.size() == (int)numberSoundSources(), "number of sound sources does not match "
              << list.size() << "/" << numberSoundSources());

    for (auto s : list)
        TransformationBuffer::copy(objectTransform, s->transformationBuffer());
}

void Object::calculateMicrophoneTransformation(const TransformationBuffer * objectTransform,
        const QList<AUDIO::SpatialMicrophone*>& list,
        const RenderTime&)
{
    MO_ASSERT(list.size() == (int)numberMicrophones(), "number of microphones does not match "
              << list.size() << "/" << numberMicrophones());

    for (auto m : list)
        TransformationBuffer::copy(objectTransform, m->transformationBuffer());
}

void Object::assignAudioConnections(AudioObjectConnections * ao)
{
    delete pobj_->p_aoCons_;
    pobj_->p_aoCons_ = ao;
}

AudioObjectConnections * Object::getAssignedAudioConnections() const
{
    return pobj_->p_aoCons_;
}

// -------------------- modulators ---------------------

void Object::removeNullModulators(bool recursive)
{
    for (auto p : params()->parameters())
        p->clearNullModulators();

    if (recursive)
        for (auto c : childObjects())
            c->removeNullModulators(true);
}

void Object::removeOutsideModulators(bool recursive)
{
    // -- get a list of ids of objects that are modulators
    //    but are not in this tree --

    QSet<QString> ids;

    if (!recursive)
    {
        auto modids = params()->getModulatorIds();
        for (auto id : modids)
            // modulation from outside ourselfes?
            if (!findChildObject(id))
                ids.insert(id);
    }
    else
    {
        // whole self tree
        auto all = findChildObjects(TG_ALL, true);
        all.prepend(this);

        for (Object * o : all)
        {
            auto modids = o->params()->getModulatorIds();
            for (auto id : modids)
                // modulation from outside ourselfes?
                if (!findChildObject(id))
                    ids.insert(id);
        }
    }

    if (!ids.isEmpty())
        removeModulators(ids.toList(), recursive);
}

void Object::removeModulators(const QList<QString> &modulatorIds, bool recursive)
{
    params()->removeModulators(modulatorIds);

    if (recursive)
        for (auto c : childObjects())
            c->removeModulators(modulatorIds, recursive);
}

QList<Modulator*> Object::getModulators(bool recursive) const
{
    QList<Modulator*> mods;
    // params
    for (auto p : params()->parameters())
        mods << p->modulators();

    // childs
    if (recursive)
        for (auto c : childObjects())
            mods << c->getModulators(true);

    return mods;
}

void Object::getModulatingObjects(ObjectConnectionGraph& graph, bool recursive) const
{
    for (auto p : params()->parameters())
        p->getModulatingObjects(graph, recursive);
}

QList<Object*> Object::getModulatingObjectsList(bool recursive) const
{
    ObjectConnectionGraph graph;
    getModulatingObjects(graph, recursive);
    auto list = graph.makeLinear();
    list.removeOne(const_cast<Object*>(this));
    return list;
}

void Object::getFutureModulatingObjects(
        ObjectConnectionGraph& graph, const Scene *scene, bool recursive) const
{
    for (auto p : params()->parameters())
        p->getFutureModulatingObjects(graph, scene, recursive);
}

QList<Object*> Object::getFutureModulatingObjectsList(
            const Scene *scene, bool recursive) const
{
    ObjectConnectionGraph graph;
    getFutureModulatingObjects(graph, scene, recursive);
    auto list = graph.makeLinear();
    list.removeOne(const_cast<Object*>(this));
    return list;
}

QList<QPair<Parameter*, Object*>> Object::getModulationPairs() const
{
    QList<QPair<Parameter*, Object*>> pairs;

    for (auto p : params()->parameters())
    {
        const QList<Object*> list = p->getModulatingObjectsList(false);

        for (auto o : list)
            pairs.append(qMakePair(p, o));
    }

    return pairs;
}

const QMap<SignalType, uint>& Object::getNumberOutputs() const
{
    return pobj_->p_outputMap_;
}

uint Object::getNumberOutputs(SignalType t) const
{
    auto it = pobj_->p_outputMap_.find(t);
    return it == pobj_->p_outputMap_.end() ? 0 : it.value();
}

QString Object::getSignalName(SignalType t)
{
    switch (t)
    {
        case ST_FLOAT: return tr("float");
        case ST_INT: return tr("int");
        case ST_SELECT: return tr("select");
        case ST_TEXT: return tr("text");
        case ST_FILENAME: return tr("filename");
        case ST_TRANSFORMATION: return tr("transformation");
        case ST_TEXTURE: return tr("texture");
        case ST_TIMELINE1D: return tr("timeline1d");
        case ST_AUDIO: return tr("audio");
        case ST_CALLBACK: return tr("callback");
        case ST_GEOMETRY: return tr("geometry");
        case ST_FONT: return tr("font");
    }
    return QString();
}

QString Object::getOutputName(SignalType t, uint channel) const
{
    if (t == ST_TRANSFORMATION)
        return QString("transf. %1").arg(channel);
    else
        return QString("%1 %2").arg(getSignalName(t)).arg(channel);
}

void Object::setNumberOutputs(SignalType t, uint num, bool emitSignal)
{
    MO_DEBUG_MOD("Object::setNumberOutputs(" << t << ", " << num << ", " << emitSignal << ")");

    // check for change
    auto it = pobj_->p_outputMap_.find(t);
    if (it != pobj_->p_outputMap_.end() && num == it.value())
        return;

    pobj_->p_outputMap_.insert(t, num);

    /** @todo Need a signal for changing outputs */
    if (emitSignal && editor())
        editor()->emitAudioChannelsChanged(this);
}

void Object::emitConnectionsChanged()
{
    if (editor())
        editor()->emitAudioChannelsChanged(this);
}


bool Object::hasError() const { return !pobj_->p_errorStr_.isEmpty(); }
const QString& Object::errorString() const { return pobj_->p_errorStr_; }
void Object::clearError() { pobj_->p_errorStr_.clear(); }

void Object::setErrorMessage(const QString &errorString) const
{
    if (errorString.isEmpty())
        return;

    if (!pobj_->p_errorStr_.isEmpty())
        pobj_->p_errorStr_.append("\n");

    pobj_->p_errorStr_.append(errorString);
}


const EvolutionBase* Object::getEvolution(const QString& key) const
{
    if (key == tr("Parameters"))
    {
        if (!pobj_->p_paramEvo_)
            pobj_->p_paramEvo_ = new ParameterEvolution(const_cast<Object*>(this));
        else
            pobj_->p_paramEvo_->updateFromObject();
        return pobj_->p_paramEvo_;
    }
    return nullptr;
}

/** Sets new specimen */
void Object::setEvolution(const QString& key, const EvolutionBase* evobase)
{
    if (key == tr("Parameters"))
    {
        auto evo = dynamic_cast<const ParameterEvolution*>(evobase);
        if (!evo)
            return;
        MO_ASSERT(evo->object() == this, "Wrong ParameterEvolution given to Object::setEvolution()");
        if (evo->object() != this)
            return;

        if (!pobj_->p_paramEvo_)
            pobj_->p_paramEvo_ = evo->createClone();
        else
            pobj_->p_paramEvo_->copyFrom(evo);

        evo->applyParametersToObject(true);
    }
}

} // namespace MO
