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
#include "param/parameterselect.h"
#include "audio/audiosource.h"

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
    parentActivityScope_    (AS_ON),
    currentActivityScope_   (AS_ON)
{
    // tie into Object hierarchy
    if (auto o = qobject_cast<Object*>(parent))
    {
        setParentObject_(o);
    }
}

Object::~Object()
{
    for (auto p : parameters_)
        delete p;

    for (auto a : audioSources_)
        delete a;
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

    // assign
    parentObject_ = parent;

    // and add to child list
    parentObject_->addChildObjectHelper_(this, index);

    // adjust idnames in new tree
    makeUniqueIds_(rootObject());

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

void Object::deleteObject_(Object * child)
{
    if (takeChild_(child))
    {
        child->setParent(0);
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

    // transformations are child-less
    if (type() == T_TRANSFORMATION)
        return false;

    // except for the transformation mix class
    // which holds itself and transformations
    if (type() == T_TRANSFORMATION_MIX)
        return t & TG_TRANSFORMATION;

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

void Object::setNumberThreads(uint num)
{
    MO_DEBUG_TREE("Object('" << idName() << "')::setNumberThreads(" << num << ")");

    numberThreads_ = num;

    transformation_.resize(num);
    bufferSize_.resize(num);

    for (auto a : audioSources_)
        a->setNumberThreads(num);
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
            strPrev(tr("Object is only active in the preview modes and will not be rendered")),
            strPrev1(tr("Object is only active in preview mode 1 and will not be rendered")),
            strPrev2(tr("Object is only active in preview mode 2 and will not be rendered")),
            strPrev3(tr("Object is only active in preview mode 3 and will not be rendered")),
            strPrev1r(tr("Object is only active in preview mode 1 and when rendering")),
            strPrev2r(tr("Object is only active in preview mode 2 and when rendering")),
            strPrev3r(tr("Object is only active in preview mode 3 and when rendering")),
            strRend(tr("Object is only active when rendering"));

    paramActiveScope_ =
    createSelectParameter("_activescope", tr("activity scope"),
                         strTip,
                         { "off", "on", "prev", "ren", "prev1", "prev2", "prev3",
                           "prev1r", "prev2r", "prev3r" },
                         { tr("off"), tr("on"), tr("preview"), tr("render"),
                           tr("preview 1"), tr("preview 2"), tr("preview 3"),
                           tr("preview 1 + render"), tr("preview 2 + render"), tr("preview 3 + render") },
                         { strOff, strOn, strPrev, strRend, strPrev1, strPrev2, strPrev3,
                           strPrev1r, strPrev2r, strPrev3r },
                         { AS_OFF, AS_ON, AS_PREVIEW, AS_RENDER,
                           AS_PREVIEW_1, AS_PREVIEW_2, AS_PREVIEW_3,
                           AS_PREVIEW_1 | AS_RENDER, AS_PREVIEW_2 | AS_RENDER, AS_PREVIEW_3 | AS_RENDER },
                         AS_ON, true, false );
}

void Object::onParameterChanged(Parameter * p)
{
    MO_DEBUG_PARAM("Object::parameterChanged('" << p->idName() << "')");

    // activity scope changed
    if (p == paramActiveScope_)
        passDownActivityScope_(activityScope());

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
    param->setModulateable(modulateable);
    param->setDefaultValue(std::min(maxValue, std::max(minValue, defaultValue )));
    param->setMinValue(minValue);
    param->setMaxValue(maxValue);
    param->setSmallStep(smallStep);
    param->setStatusTip(statusTip);
    param->setEditable(editable);

    return param;
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
            MO_ASSERT(false, "object '" << idName() << "' requested float "
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

    return param;
}


// ----------------- audio sources ---------------------

void Object::setBufferSize(uint bufferSize, uint thread)
{
    bufferSize_[thread] = bufferSize;
    transformation_[thread].resize(bufferSize);

    for (auto a : audioSources_)
        a->setBufferSize(bufferSize, thread);
}

void Object::setSampleRate(uint samplerate)
{
    sampleRate_ = std::max((uint)1, samplerate);
    sampleRateInv_ = 1.0 / sampleRate_;
}

AUDIO::AudioSource * Object::createAudioSource(const QString& id)
{
    auto a = new AUDIO::AudioSource(id, this);

    audioSources_.append(a);

    return a;
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
