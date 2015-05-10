/** @file objecteditor.cpp

    @brief Class for editing objects and signalling other widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.11.2014</p>
*/

#include <QMessageBox>
#include <QPoint>

#include "objecteditor.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/scenelock_p.h"
#include "object/control/sequence.h"
#include "object/control/sequencefloat.h"
#include "object/control/trackfloat.h"
#include "object/control/clipcontroller.h"
#include "object/audioobject.h"
#include "object/objectfactory.h"
#include "object/control/modulatorobject.h"
#include "object/texture/textureobjectbase.h"
#include "object/shaderobject.h"
#include "object/util/audioobjectconnections.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parameterfilename.h"
#include "object/param/parametertimeline1d.h"
#include "math/timeline1d.h"
#include "tool/stringmanip.h"
#include "gui/item/abstractfrontitem.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {


#define MO__CHECK_SCENE \
    if (!scene_)        \
    {                   \
        MO_WARNING("Use of ObjectEditor with no assigned scene object\n" \
                    __FILE__ << ":" << __LINE__); \
        return false; \
    }

#if 0
#   define MO_DEBUG_OBJ_EDITOR(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_OBJ_EDITOR(unused__) { }
#endif


ObjectEditor::ObjectEditor(QObject *parent)
    : QObject       (parent),
      scene_        (0)
{
}


// --------------------------------- ids ---------------------------------------

QString ObjectEditor::getUniqueId(QString id, const QSet<QString> &existingNames, bool * existed)
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


void ObjectEditor::makeUniqueIds(const Object* root, Object* newBranch)
{
    // get set of existing ids
    QSet<QString> existing = root->getChildIds(true);
    existing.insert(root->idName());

    // get all new objects
    QList<Object*> list = newBranch->findChildObjects(Object::TG_ALL, true);
    list.prepend(newBranch);

    // keep track of changes
    QMap<QString, QString> changedIds;

    // adjust id for each object
    for (Object * o : list)
    {
        QString newid = getUniqueId(o->idName(), existing);

        if (newid != o->idName())
        {
            // add new id to stock
            existing.insert(newid);
            // remember old id
            changedIds.insert(o->idName(), newid);
            // and change
            Private::set_object_id_(o, newid);
        }
    }

    // tell the branch about changed ids
    if (!changedIds.isEmpty())
        newBranch->idNamesChanged(changedIds);
}

void ObjectEditor::makeUniqueIds(const Object* root, const QList<Object*> newBranches)
{
    // get set of existing ids
    QSet<QString> existing = root->getChildIds(true);
    existing.insert(root->idName());

    // get all new objects
    QList<Object*> list;
    for (auto o : newBranches)
    {
        list << o;
        list << o->findChildObjects(Object::TG_ALL, true);
    }

    // keep track of changes
    QMap<QString, QString> changedIds;

    // adjust id for each object
    for (Object * o : list)
    {
        QString newid = getUniqueId(o->idName(), existing);

        if (newid != o->idName())
        {
            // add new id to stock
            existing.insert(newid);
            // remember old id
            changedIds.insert(o->idName(), newid);
            // and change
            Private::set_object_id_(o, newid);
        }
    }

    // tell the branch about changed ids
    if (!changedIds.isEmpty())
        for (auto o : newBranches)
            o->idNamesChanged(changedIds);
}




// ----------------------------- signal wrapper --------------------------------

void ObjectEditor::emitObjectChanged(Object * o)
{
    emit objectChanged(o);
    emit sceneChanged(scene_);
}

void ObjectEditor::emitAudioChannelsChanged(Object * o)
{
    emit objectChanged(o);
    emit audioConnectionsChanged();
    emit sceneChanged(scene_);
}




// --------------------------------- tree --------------------------------------

void ObjectEditor::setObjectName(Object *object, const QString &name)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::setObjectName(" << object << ", " << name << ")");

    const bool changed = object->name() != name;

    object->setName(name);

    if (changed)
        emit objectNameChanged(object);
}

void ObjectEditor::setObjectHue(Object *object, int hue)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::setObjectHue(" << object << ", " << hue << ")");

    object->setAttachedData(hue, Object::DT_HUE);

    emit objectColorChanged(object);
}

bool ObjectEditor::addObject(Object *parent, Object *newChild, int insert_index)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::addObject(" << parent << ", " << newChild << ", " << insert_index << ")");
    MO__CHECK_SCENE

    QString error;
    if (!parent->isSaveToAdd(newChild, error))
    {
        QString errtext = tr("The object %1 could not be added to %2.\n%3")
                            .arg(newChild->name())
                            .arg(parent->name())
                            .arg(error);
        delete newChild;
        QMessageBox::critical(0, tr("Can't add object"), errtext);

        MO_DEBUG(errtext);

        return false;
    }

    scene_->addObject(parent, newChild,
                      ObjectFactory::getBestInsertIndex(parent, newChild, insert_index)
                      );

    emit objectAdded(newChild);
    emit sceneChanged(scene_);
    return true;
}

bool ObjectEditor::addObjects(Object *parent, const QList<Object*> newObjects, int insert_index)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::addObjects(" << parent << ", [" << newObjects.size() << "], " << insert_index << ")");
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

    // we need to make all the branches unique at the same time
    // so their idNamesChanged() functions get called with ALL
    // changed ids from all branches.
    makeUniqueIds(parent, newObjects);

#if 0
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
#else
    QList<Object*> actualObjects;
    QString err;
    for (auto o : newObjects)
    if (saveAdd.contains(o))
        actualObjects.append(o);
    else
        delete o;

    if (!actualObjects.isEmpty())
    {
        scene_->addObjects(parent, actualObjects, insert_index);
        emit objectsAdded(actualObjects);
    }

#endif
    emit sceneChanged(scene_);

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
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::deleteObject(" << object << ")");
    MO__CHECK_SCENE

    scene_->deleteObject(object);

    emit objectDeleted(object);
    emit sceneChanged(scene_);

    return true;
}

bool ObjectEditor::deleteObjects(const QList<Object*>& list)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::deleteObjects(" << list.size() << ")");
    MO__CHECK_SCENE

    scene_->deleteObjects(list);

    emit objectsDeleted(list);
    emit sceneChanged(scene_);

    return true;
}

bool ObjectEditor::deleteChildren(Object *object)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::deleteChildren(" << object << ")");
    MO__CHECK_SCENE

    auto list = object->childObjects();
    return deleteObjects(list);
}

bool ObjectEditor::setObjectIndex(Object * object, int newIndex)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::setObjectIndex(" << object << ", " << newIndex << ")");
    MO__CHECK_SCENE

    bool res = scene_->setObjectIndex(object, newIndex);

    if (res)
    {
        emit objectChanged(object);
        emit sceneChanged(scene_);
    }

    return res;
}

bool ObjectEditor::moveObject(Object *object, Object *newParent, int newIndex)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::moveObject(" << object << ", " << newParent << ", " << newIndex << ")");
    MO__CHECK_SCENE

    QString error;
    if (newParent != object->parentObject() && !newParent->isSaveToAdd(object, error))
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
        {
            emit objectChanged(object);
            emit sceneChanged(scene_);
        }
        return res;
    }
    else
    {
        scene_->moveObject(object, newParent, newIndex);
        emit objectMoved(object, oldParent);
        emit sceneChanged(scene_);
    }

    return true;
}




void ObjectEditor::appendTextureProcessor(Object *object, Object *newObject, int insert_index)
{
    // find tex input to connect
    Parameter * texParam = 0;
    const auto params = newObject->params()->parameters();
    for (Parameter * p : params)
    if (p->isModulateable() && p->typeName() == "texture")
    {
        texParam = p;
        break;
    }

    // be smart about some settings
    bool doMasterOut = false;
    if (TextureObjectBase * to = qobject_cast<TextureObjectBase*>(newObject))
    {
        to->setEnableMasterOut(doMasterOut = true);
        to->setResolutionMode(TextureObjectBase::RM_INPUT);
    }
    // disable output in source module
    if (doMasterOut)
    {
        if (TextureObjectBase * to = qobject_cast<TextureObjectBase*>(object))
            to->setEnableMasterOut(false, true);
    }

    // find correct parent
    Object * parent = object->parentObject();
    if (!parent) // is scene already?
        parent = object;

    // find position
    if (!newObject->hasAttachedData(Object::DT_GRAPH_POS))
    {
        QPoint gridPos = object->getAttachedData(Object::DT_GRAPH_POS).toPoint();
        gridPos.rx() += 2;
        newObject->setAttachedData(gridPos, Object::DT_GRAPH_POS);
    }

    // add to scene
    addObject(parent, newObject, insert_index);

    // connect
    if (texParam)
    {
        addModulator(texParam, object->idName(), "");
    }

}




// ----------------------- params ---------------------------

namespace
{
    template <class P, typename V>
    void setParameterVal(ObjectEditor * edit, P p, V v)
    {
        {
            ScopedSceneLockWrite lock(edit->scene());

            // only apply if different
            if (p->baseValue() == v)
                return;

            p->setValue(v);
            p->object()->onParameterChanged(p);
        }
        p->object()->updateParameterVisibility();

        // signals
        emit edit->parameterChanged(p);
        if (auto iface = dynamic_cast<ValueFloatInterface*>(p->object()))
            emit edit->valueFloatChanged(iface);
        if (Sequence * seq = qobject_cast<Sequence*>(p->object()))
            emit edit->sequenceChanged(seq);

        // repaint
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

void ObjectEditor::setParameterVisibleInGraph(Parameter * p, bool enbale)
{
    if (enbale != p->isVisibleInGraph())
    {
        p->setVisibleGraph(enbale);
        emit parameterVisibilityChanged(p);
    }
}

void ObjectEditor::setParameterVisibleInterface(Parameter * p, bool enbale)
{
    if (enbale != p->isVisibleInterface())
    {
        p->setVisibleInterface(enbale);
        emit parameterVisibilityChanged(p);
    }
}

bool ObjectEditor::addModulator(Parameter *p, const QString &idName, const QString& outputId)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::addModulator(" << p << ", " << idName << "," << outputId << ")");
    MO__CHECK_SCENE

    /** @todo test sanity of connection! */

    Modulator * m;
    {
        ScopedSceneLockWrite lock(scene_);
        m = p->addModulator(idName, outputId);
        p->collectModulators();
        p->object()->onParameterChanged(p);
        p->object()->updateParameterVisibility();
    }
    emit modulatorAdded(m);
    emit parameterChanged(p);
    emit sceneChanged(scene_);
    scene_->render();
    return true;
}


bool ObjectEditor::removeModulator(Parameter *p, const QString &idName, const QString &outputId)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::removeModulator(" << p << ", " << idName << "," << outputId << ")");
    MO__CHECK_SCENE

    Modulator * m;
    {
        ScopedSceneLockWrite lock(scene_);
        m = p->findModulator(idName);
        p->removeModulator(idName, outputId);
        p->collectModulators();
        p->object()->onParameterChanged(p);
        p->object()->updateParameterVisibility();
    }
    emit modulatorDeleted(m);
    emit parameterChanged(p);
    emit sceneChanged(scene_);
    scene_->render();
    return true;
}

bool ObjectEditor::removeAllModulators(Parameter *p)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::removeAllModulators(" << p << ")");
    MO__CHECK_SCENE

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
    emit sceneChanged(scene_);
    scene_->render();
    return true;
}



// ---------------------------------- ui modulators ----------------------------------------

bool ObjectEditor::addUiModulator(Parameter *p, GUI::AbstractFrontItem * item)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::addUiModulator(" << p << ", " << item << ")");
    MO__CHECK_SCENE

    ModulatorObject * mo;
    Modulator * m;
    {
        ScopedSceneLockWrite lock(scene_);
        // create or reuse proxy object
        mo = scene_->createUiModulator(item->idName());
        // connect to parameter
        m = p->addModulator(mo->idName(), "");
        if (!m)
            return false;
        p->collectModulators();
        p->object()->onParameterChanged(p);
        p->object()->updateParameterVisibility();
    }
    emit modulatorAdded(m);

    emit parameterChanged(p);
    emit sceneChanged(scene_);
    scene_->render();
    return true;
}

bool ObjectEditor::removeUiModulator(const QString &uiId)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::removeUiModulator(" << uiId << ")");
    MO__CHECK_SCENE

    auto list = scene_->getUiModulatorObjects(QList<QString>() << uiId);
    QList<Object*> objects;
    for (auto o : list)
        objects << o;
    deleteObjects(objects);

    /** @todo should also emit parameterChanged() for every affected Parameter */
    return true;
}

bool ObjectEditor::removeUiModulators(const QList<QString> &uiIds)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::removeUiModulators(" << uiIds.size() << ")");
    MO__CHECK_SCENE

    auto list = scene_->getUiModulatorObjects(uiIds);
    QList<Object*> objects;
    for (auto o : list)
        objects << o;
    deleteObjects(objects);

    /** @todo should also emit parameterChanged() for every affected Parameter */
    return true;
}

bool ObjectEditor::setUiValue(const QString& uiId, Double timeStamp, Float value)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::setUiValue(" << uiId << ", " << timeStamp << ", " << value);
    MO__CHECK_SCENE

    scene_->setUiValue(uiId, timeStamp, value);

    return true;
}



// ---------------------------------- audio cons -------------------------------------------

bool ObjectEditor::connectAudioObjects(AudioObject *from, AudioObject *to,
                                       uint outChannel, uint inChannel,
                                       uint numChannels)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::connectAudioObjects(" << from << ", " << to << ", "
                        << outChannel << ", " << inChannel << ", " << numChannels << ")");
    MO__CHECK_SCENE

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
    emit sceneChanged(scene_);

    return true;
}

bool ObjectEditor::disconnectAudioObjects(const AudioObjectConnection & c)
{
    return disconnectAudioObjects(c.from(), c.to(), c.outputChannel(), c.inputChannel(), c.numChannels());
}

bool ObjectEditor::disconnectAudioObjects(AudioObject *from, AudioObject *to,
                                       uint outChannel, uint inChannel,
                                       uint numChannels)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::disconnectAudioObjects(" << from << ", " << to << ", "
                        << outChannel << ", " << inChannel << ", " << numChannels << ")");
    MO__CHECK_SCENE

    if (scene_->audioConnections()->disconnect(from, to, outChannel, inChannel, numChannels))
    {
        emit audioConnectionsChanged();
        emit sceneChanged(scene_);
    }

    return true;
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

Object * ObjectEditor::findAModulatorParent(Parameter * param)
{
    Object * obj = param->object();
    MO_ASSERT(obj, "missing object for parameter '" << param->idName() << "'");

    if (obj->parentObject())
        obj = obj->parentObject();

    return obj;
}


TrackFloat * ObjectEditor::createFloatTrack(Parameter * param)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::createFloatTrack('" << param->idName() << "')");

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
    addModulator(param, track->idName(), "");

    return (TrackFloat*)track;
}

TrackFloat * ObjectEditor::wrapIntoTrack(SequenceFloat *seq)
{
    Object * parent = seq->parentObject();

    if (!parent || seq->findParentObject(Object::TG_TRACK))
        return 0;

    TrackFloat * track = ObjectFactory::createTrackFloat(seq->name());
    addObject(parent, track);

    const QPoint pos = seq->getAttachedData(Object::DT_GRAPH_POS).toPoint();
    seq->setAttachedData(QPoint(1,1), Object::DT_GRAPH_POS);
    track->setAttachedData(pos, Object::DT_GRAPH_POS);

    moveObject(seq, track);

    return track;
}


Object * ObjectEditor::createInClip(Parameter *p, const QString& className, Clip * parent)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::createInClip('" << p->name() << ", " << className << ", " << parent << "')");

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
        // create clip
        parent = static_cast<Clip*>(ObjectFactory::createObject("Clip"));
        if (!parent)
            MO_ERROR("Could not create Clip");

        Object * clipparent = findAModulatorParent(p);
        addObject(clipparent, parent);
    }

    if (!parent->canHaveChildren(obj->type()))
    {
        delete obj;
        MO_ERROR("Can't add '" << obj->name() << "' to " << parent->name());
    }

    // add to clip
    addObject(parent, obj, -1);

    return obj;
}

SequenceFloat * ObjectEditor::createFloatSequenceFor(MO::Parameter * param)
{
    MO_DEBUG_OBJ_EDITOR("ObjectEditor::createFloatSequenceFor('" << param->idName() << "')");

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
