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
#include "param/parameterfloat.h"

namespace MO {

bool registerObject_(Object * obj)
{
    return ObjectFactory::registerObject(obj);
}

Object::Object(QObject *parent) :
    QObject         (parent),
    canBeDeleted_   (true),
    parentObject_   (0)
{
    // tie into Object hierarchy
    if (auto o = dynamic_cast<Object*>(parent))
    {
        setParentObject(o);
    }
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
    return deserializeTree_(io);
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
        // read actual object data
        o->deserialize(io);

        // once in a while check stream for errors
        if (io.status() != QDataStream::Ok)
            MO_IO_ERROR(WRITE, "error deserializing object '"<<idName<<"'.\n"
                        "QIODevice error: '"<<io.device()->errorString()<<"'");

        // read parameters
        o->createParameters();
        deserializeParameters_(io, o);
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
        if (child)
            o->addObject(child);
    }

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
}

// ---------------- getter -------------------------

QString Object::namePath() const
{
    Object * p = parentObject();
    if (!p)
        return "/";

    QString path;
    while (p)
    {
        path.prepend("/" + p->name());
        p = p->parentObject();
    }

    return path;
}

QString Object::idNamePath() const
{
    Object * p = parentObject();
    if (!p)
        return "/";

    QString path;
    while (p)
    {
        path.prepend("/" + p->idName());
        p = p->parentObject();
    }

    return path;
}


// ------------------ setter -----------------------

void Object::setName(const QString & n)
{
    name_ = n;
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
    MO_DEBUG("--- " << mods.size() << " modulators for " << o->idName());
    for (auto m : mods)
        MO_DEBUG(m->idName());
    if (mods.contains((Object*)this))
    {
        error = tr("Adding '%1' as a child to '%2' would cause an infinite "
                   "modulation loop!").arg(o->idName()).arg(idName());
        return false;
    }

    return true;
}


void Object::setParentObject(Object *parent, int index)
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

    // assign
    parentObject_ = parent;

    // and add to child list
    parentObject_->addChildObject_(this, index);

    // adjust idnames in new tree
    makeUniqueIds_(rootObject());

    // tell Scene
    if (Scene * scene = sceneObject())
    {
        scene->tellTreeChanged();
        scene->tellObjectAdded(this);
    }

    // tell parent object
    parentObject_->childrenChanged_();
}

/*
int Object::getInsertIndex(Object *object, int insert_index) const
{
    MO_DEBUG_TREE("Object::getInsertIndex('" << object->idName() << "', " << insert_index << ")");

    if (childObjects_.empty())
        return 0;

    const int lastTrans = indexOfLastChild<Transformation>();

    if (insert_index == -1)
        insert_index = childObjects_.size();

    if (object->isTransformation())
    {
        return (insert_index > lastTrans) ?
                    indexOfLastChild<Transformation>(insert_index) + 1
                  : insert_index;
    }
    else
    {
        return (insert_index <= lastTrans)?
                    lastTrans + 1
                  : std::min((int)childObjects_.size(), insert_index);
    }
}
*/

Object * Object::addObject(Object * o, int index)
{
    MO_ASSERT(o, "trying to add a NULL child");
    MO_ASSERT(!childObjects_.contains(o), "duplicate addChild for '" << o->idName());

    o->setParentObject(this, index);

    return o;
}

Object * Object::addChildObject_(Object * o, int index)
{
    MO_ASSERT(o, "trying to add a NULL child");

    if (index < 0)
        childObjects_.append(o);
    else
        childObjects_.insert(index, o);

    return o;
}

void Object::deleteObject(Object * child)
{
    takeChild_(child);

    // tell Scene
    if (Scene * scene = sceneObject())
        scene->tellTreeChanged();

    // tell this object
    childrenChanged_();
}

bool Object::takeChild_(Object *child)
{
    return childObjects_.removeOne(child);
}

QString Object::getUniqueId(QString id, Object * ignore) const
{
    MO_ASSERT(!id.isEmpty(), "unset object id detected, class='"
              << className() << "', name='" << name() << "'");

    // create an id if necessary
    if (id.isEmpty())
        id = className().isEmpty()? "Object" : className();

    // replace white char with _
    id.replace(QRegExp("\\s\\s*"), "_");

    auto root = rootObject();
    while (root->findChildObject(id, true, ignore))
    {
        increase_id_number(id, 1);
    }

    return id;
}

void Object::makeUniqueIds_(Object * root)
{
    idName_ = root->getUniqueId(idName_, this);

    for (auto o : childObjects_)
        o->makeUniqueIds_(root);
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

    // sequences belong on tracks or sequencegroups only
    // with matching type
    if (t & TG_SEQUENCE)
        return
            type() == T_SEQUENCEGROUP
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
        return t != T_TRANSFORMATION;

    return true;
}

void Object::childrenChanged_()
{
    // collect special sub-objects
    collectTransformationObjects_();

    // notify derived classes
    childrenChanged();
}

void Object::setNumberThreads(int num)
{
    transformation_.resize(num);
}
/*
void Object::setNumberThreadsRecursive_(int threads)
{
    setNumberThreads(threads);

    for (auto o : childObjects_)
        o->setNumberThreadsRecursive_(threads);
}*/


// ------------------------- 3d -----------------------

void Object::clearTransformation(int thread)
{
    transformation_[thread] = Mat4(1.0);
}

void Object::collectTransformationObjects_()
{
    transformationObjects_.clear();

    for (auto o : childObjects_)
        if (auto t = qobject_cast<Transformation*>(o))
            transformationObjects_.append(t);
}

void Object::calculateTransformation(Mat4 &matrix, Double time) const
{
    for (auto t : transformationObjects_)
        t->applyTransformation(matrix, time);
}

// ---------------------- parameter --------------------------

Parameter * Object::findParameter(const QString &id)
{
    for (auto p : parameters_)
        if (p->idName() == id)
            return p;
    return 0;
}

ParameterFloat * Object::createFloatParameter(const QString& id, const QString& name, Double defaultValue, bool editable)
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
                      "which is already present as parameter of another type.");
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
    param->setDefaultValue(defaultValue);
    param->setEditable(editable);

    return param;
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
