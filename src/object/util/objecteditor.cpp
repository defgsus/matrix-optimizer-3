/** @file objecteditor.cpp

    @brief Class for editing objects and signalling other widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.11.2014</p>
*/

#include <QMessageBox>

#include "objecteditor.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/scenelock_p.h"
#include "object/sequence.h"
#include "object/clipcontainer.h"
#include "object/audioobject.h"
#include "object/objectfactory.h"
#include "object/util/audioobjectconnections.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parameterfilename.h"
#include "object/param/parametertimeline1d.h"
#include "math/timeline1d.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {


#define MO__CHECK_SCENE \
    if (!scene_)        \
    {                   \
        MO_WARNING("Use of ObjectEditor with no assigned scene object"); \
        return false; \
    }


ObjectEditor::ObjectEditor(QObject *parent)
    : QObject       (parent),
      scene_        (0)
{
}



// --------------------------------- tree --------------------------------------

void ObjectEditor::setObjectName(Object *object, const QString &name)
{
    MO_DEBUG_TREE("ObjectEditor::setObjectName(" << object << ", " << name << ")");

    const bool changed = object->name() != name;

    object->setName(name);

    if (changed)
        emit objectNameChanged(object);
}

bool ObjectEditor::addObject(Object *parent, Object *newChild, int insert_index)
{
    MO_DEBUG_TREE("ObjectEditor::addObject(" << parent << ", " << newChild << ", " << insert_index << ")");
    MO__CHECK_SCENE

    QString error;
    if (!parent->isSaveToAdd(newChild, error))
    {
        delete newChild;
        QMessageBox::critical(0, tr("Can't add object"),
                              tr("The object %1 could not be added to %2.\n%3")
                              .arg(newChild->name())
                              .arg(parent->name())
                              .arg(error));
        return false;
    }

    scene_->addObject(parent, newChild,
                      ObjectFactory::getBestInsertIndex(parent, newChild, insert_index)
                      );

    emit objectAdded(newChild);
    return true;
}

bool ObjectEditor::addObjects(Object *parent, const QList<Object*> newObjects, int insert_index)
{
    MO_DEBUG("ObjectEditor::addObjects(" << parent << ", [" << newObjects.size() << "], " << insert_index << ")");
    MO__CHECK_SCENE

    QSet<Object*> saveAdd;

    QString error;
    for (auto o : newObjects)
    {
        QString err;
        if (!parent->isSaveToAdd(o, err))
            error += "\n" + err;
        else
            saveAdd.insert(o);
    }

    QString err;
    for (auto o : newObjects)
    if (saveAdd.contains(o))
    {
        auto idx = ObjectFactory::getBestInsertIndex(parent, o, insert_index++);

        // XXX replace this with a more efficient version in Scene::addObjects..
        // it locks and updates for every object
        scene_->addObject(parent, o, idx);
        emit objectAdded(o);
    }
    else
        delete o;

    if (!error.isEmpty())
    {
        QMessageBox::critical(0, tr("Can't add object"),
                              (saveAdd.isEmpty()
                                ? tr("None of the objects could be added to %1.%2")
                                : tr("Some objects could not be added to %1.%2"))
                              .arg(parent->name())
                              .arg(error));
        return !saveAdd.isEmpty();
    }
    return true;
}


bool ObjectEditor::deleteObject(Object *object)
{
    MO_DEBUG_TREE("ObjectEditor::deleteObject(" << object << ")");
    MO__CHECK_SCENE

    scene_->deleteObject(object);

    emit objectDeleted(object);

    return true;
}

bool ObjectEditor::setObjectIndex(Object * object, int newIndex)
{
    MO_DEBUG_TREE("ObjectEditor::setObjectIndex(" << object << ", " << newIndex << ")");
    MO__CHECK_SCENE

    bool res = scene_->setObjectIndex(object, newIndex);

    if (res)
        emit objectChanged(object);

    return res;
}

bool ObjectEditor::moveObject(Object *object, Object *newParent, int newIndex)
{
    MO_DEBUG_TREE("ObjectEditor::moveObject(" << object << ", " << newParent << ", " << newIndex << ")");
    MO__CHECK_SCENE

    QString error;
    if (!newParent->isSaveToAdd(object, error))
    {
        QMessageBox::critical(0, tr("Can't move object"),
                              tr("The object %1 could not be added to %2.\n%3")
                              .arg(object->name())
                              .arg(newParent->name())
                              .arg(error));
        return false;
    }

    auto oldParent = object->parentObject();

    if (oldParent == newParent)
    {
        bool res = scene_->setObjectIndex(object, newIndex);
        if (res)
            emit objectChanged(object);
        return res;
    }
    else
    {
        scene_->moveObject(object, newParent, newIndex);
        emit objectMoved(object, oldParent);
    }

    return true;
}






// ----------------------- params ---------------------------

namespace
{
    template <class P, typename V>
    void setParameterVal(ObjectEditor * edit, P p, V v)
    {
        {
            ScopedSceneLockWrite lock(edit->scene());
            p->setValue(v);
            p->object()->onParameterChanged(p);
            p->object()->updateParameterVisibility();
        }
        emit edit->parameterChanged(p);
        if (Sequence * seq = qobject_cast<Sequence*>(p->object()))
            emit edit->sequenceChanged(seq);
        edit->scene()->render();
    }
}

void ObjectEditor::setParameterValue(ParameterInt *p, Int v)
{
    setParameterVal(this, p, v);
}

void ObjectEditor::setParameterValue(ParameterFloat *p, Double v)
{
    setParameterVal(this, p, v);
}

void ObjectEditor::setParameterValue(ParameterSelect *p, int v)
{
    setParameterVal(this, p, v);
}

void ObjectEditor::setParameterValue(ParameterFilename *p, const QString& v)
{
    setParameterVal(this, p, v);
}

void ObjectEditor::setParameterValue(ParameterText *p, const QString& v)
{
    setParameterVal(this, p, v);
}

void ObjectEditor::setParameterValue(ParameterTimeline1D *p, const MATH::Timeline1D& v)
{
    setParameterVal(this, p, v);
}


void ObjectEditor::addModulator(Parameter *p, const QString &idName)
{
    Modulator * m;
    {
        ScopedSceneLockWrite lock(scene_);
        m = p->addModulator(idName);
        p->collectModulators();
        p->object()->onParameterChanged(p);
        p->object()->updateParameterVisibility();
    }
    emit modulatorAdded(m);
    emit parameterChanged(p);
    scene_->render();
}

void ObjectEditor::removeModulator(Parameter *p, const QString &idName)
{
    Modulator * m;
    {
        ScopedSceneLockWrite lock(scene_);
        m = p->findModulator(idName);
        p->removeModulator(idName);
        p->collectModulators();
        p->object()->onParameterChanged(p);
        p->object()->updateParameterVisibility();
    }
    emit modulatorDeleted(m);
    emit parameterChanged(p);
    scene_->render();
}

void ObjectEditor::removeAllModulators(Parameter *p)
{
    auto mods = p->modulators();
    {
        ScopedSceneLockWrite lock(scene_);
        p->removeAllModulators();
        p->collectModulators();
        p->object()->onParameterChanged(p);
        p->object()->updateParameterVisibility();
    }
    emit modulatorsDeleted(mods);
    emit parameterChanged(p);
    scene_->render();
}




// ---------------------------------- audio cons -------------------------------------------

bool ObjectEditor::connectAudioObjects(AudioObject *from, AudioObject *to,
                                       uint outChannel, uint inChannel,
                                       uint numChannels)
{
    if (!scene_->audioConnections()->isSaveToAdd(from, to))
    {
        QMessageBox::critical(0, tr("Can't add connection"),
                              tr("Connecting %1 and %2 would create an infinite loop")
                              .arg(from->name())
                              .arg(to->name()));
        return false;
    }

    scene_->audioConnections()->connect(from, to, outChannel, inChannel, numChannels);

    emit audioConnectionsChanged();
    return true;
}

void ObjectEditor::disconnectAudioObjects(AudioObject *from, AudioObject *to,
                                       uint outChannel, uint inChannel,
                                       uint numChannels)
{
    if (scene_->audioConnections()->disconnect(from, to, outChannel, inChannel, numChannels))
        emit audioConnectionsChanged();
}




// ----------------------------------- modulator objects -----------------------------------

QString ObjectEditor::modulatorName(Parameter *param, bool longName)
{
    // construct name
    QString name( param->name() );
    if (Object * obj = param->object())
    {
        // always prepend the object of modulator
        name.prepend( obj->name() + "." );
        if (longName)
        {
            // if the parent is not a "real object" then use the grandparent's name as well
            if (!( (obj->type() & Object::TG_REAL_OBJECT)
                   || (obj->type() & Object::TG_SEQUENCE))
                && obj->parentObject())
                    name.prepend(obj->parentObject()->name() + ".");
        }
    }

    return ">" + name;
}

TrackFloat * ObjectEditor::createFloatTrack(Parameter * param)
{
    MO_DEBUG_TREE("ObjectEditor::createFloatTrack('" << param->idName() << "')");

    MO_ASSERT(scene_, "can't edit");

    Object * obj = param->object();
    MO_ASSERT(obj, "missing object for parameter '" << param->idName() << "'");



    // find a place for the modulation track
    while (obj && !obj->canHaveChildren(Object::T_TRACK_FLOAT))
    {
        obj = obj->parentObject();
    }
    MO_ASSERT(obj, "Could not find an object to create a float track in.");

    // create track
    auto track = ObjectFactory::createObject("TrackFloat");
    track->setName(modulatorName(param, obj != param->object()));

    // add to parent
    addObject(obj, track, -1);

    // modulate parameter
    addModulator(param, track->idName());

    return (TrackFloat*)track;
}


Object * ObjectEditor::createInClip(const QString& className, Clip * parent)
{
    MO_DEBUG_TREE("ObjectEditor::createInClip('" << className << ", " << parent << "')");

    MO_ASSERT(scene_, "can't edit");

    Object * obj = ObjectFactory::createObject(className);
    if (!obj)
        MO_ERROR("Can't create object '" << className << "'");

    if (parent && !parent->canHaveChildren(obj->type()))
    {
        delete obj;
        MO_ERROR("Can't add '" << obj->name() << "' to " << parent->name());
    }

    // create a clip
    if (!parent)
    {
        ClipContainer * con = 0;

        // take first found container
        auto clipcons = scene_->findChildObjects<ClipContainer>(QString(), true);
        if (!clipcons.isEmpty())
            con = clipcons[0];
        // or create new
        else
        {
            con = static_cast<ClipContainer*>(ObjectFactory::createObject("ClipContainer"));
            if (!con)
                MO_ERROR("Could not create ClipContainer");
            addObject(scene_, con, 0);
        }

        // create clip
        parent = static_cast<Clip*>(ObjectFactory::createObject("Clip"));
        if (!parent)
            MO_ERROR("Could not create Clip");
        addObject(con, parent);
    }

    if (!parent->canHaveChildren(obj->type()))
    {
        delete obj;
        MO_ERROR("Can't add '" << obj->name() << "' to " << parent->name());
    }

    // add to parent
    addObject(parent, obj, -1);

    return obj;
}

SequenceFloat * ObjectEditor::createFloatSequenceFor(MO::Parameter * param)
{
    MO_DEBUG_TREE("ObjectEditor::createFloatSequenceFor('" << param->idName() << "')");

    MO_ASSERT(scene_ && param->object(), "can't edit");

    // find insertion point
    Object * parent = param->object();
    if (parent->parentObject())
        parent = parent->parentObject();
    while (parent && !parent->canHaveChildren(Object::T_SEQUENCE_FLOAT))
        parent = parent->parentObject();

    if (!parent)
    {
        QMessageBox::critical(0, tr("Can't add object"),
                              tr("No object found to place float sequence for parameter %1")
                              .arg(param->name()));
        return 0;
    }

    // creat sequence
    auto seq = ObjectFactory::createSequenceFloat(modulatorName(param));

    addObject(parent, (Object*)seq, -1);

    return seq;
}

} // namespace MO
