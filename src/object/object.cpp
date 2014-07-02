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
#include "transformation.h"
#include "parameterfloat.h"

namespace MO {

bool registerObject_(Object * obj)
{
    return ObjectFactory::registerObject(obj);
}

Object::Object(QObject *parent) :
    QObject         (parent),
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
    MO_DEBUG_IO("Object('"<<idName()<<"')::serializeTree()");

    // default header
    io.writeHeader("mo-tree", 1);

    // default object info
    io << className() << idName() << name();

    // keep position to go back later
    const qint64 startPos = io.device()->pos();
    // write temp skip marker
    io << (qint64)0;

    // write actual derived object
    serialize(io);

    // once in a while check stream for errors
    if (io.status() != QDataStream::Ok)
        MO_IO_ERROR(WRITE, "error serializing object '"<<idName()<<"'.\n"
                    "QIODevice error: '"<<io.device()->errorString()<<"'");

    const qint64 endPos = io.device()->pos();
    // write length of object
    io.device()->seek(startPos);
    io << (qint64)(endPos - startPos - sizeof(qint64));
    io.device()->seek(endPos);

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
    MO_DEBUG_IO("Object::deserializeTree()");

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
        // read derived object data
        o->deserialize(io);

        // once in a while check stream for errors
        if (io.status() != QDataStream::Ok)
            MO_IO_ERROR(WRITE, "error deserializing object '"<<idName<<"'.\n"
                        "QIODevice error: '"<<io.device()->errorString()<<"'");
    }
    // skip object if class not found
    else
    {
        MO_IO_WARNING(VERSION_MISMATCH, "unknown object class '" << className << "' in stream");
        io.device()->seek(io.device()->pos() + objLength);

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

    o->createParameters();

    return o;
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
        scene->treeChanged();

    // tell parent object
    parentObject_->childrenChanged_();
}

int Object::getInsertIndex(Object *object, int insert_index) const
{
    if (childObjects_.empty())
        return 0;

    const int lastTrans = indexOfLastChild<Transformation>();

    if (insert_index == -1)
        insert_index = childObjects_.size();

    if (object->isTransformation())
    {
        return (insert_index>=lastTrans) ?
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
        scene->treeChanged();

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
    if (t & TG_SEQUENCE)
        return true;

    if (type() & TG_SEQUENCE)
        return t & TG_PARAMETER;

    if (type() == T_SEQUENCEGROUP)
        return t == T_SEQUENCEGROUP || (t & TG_SEQUENCE);

    if (type() & TG_PARAMETER)
        return false;

    if (type() == T_TRANSFORMATION)
        return t & TG_PARAMETER;

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

ParameterFloat * Object::createFloatParameter(
        const QString& id, const QString& name, Double defaultValue)
{
    ParameterFloat * param = 0;

    // see if already there
    for (auto o : childObjects_)
    if (auto p = qobject_cast<ParameterFloat*>(o))
    {
        if (p->parameterId() == id)
        {
            param = p;
            break;
        }
    }

    // create new
    if (!param)
    {
        param = static_cast<ParameterFloat*>(
                    ObjectFactory::createObject(MO_OBJECTCLASSNAME_PARAMETER_FLOAT) );
        MO_ASSERT(param, "could not create float parameter");

        addObject(param);

        // first time init
        param->idName_ = "p_" + id;
        param->setParameterId(id);
        param->setValue(defaultValue);
    }

    // override potentially previous
    param->setName(name);
    param->setDefaultValue(defaultValue);

    return param;
}



} // namespace MO
