/** @file objecteditor.cpp

    @brief Class for editing objects and signalling other widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.11.2014</p>
*/

#include "objecteditor.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/scenelock_p.h"
#include "object/sequence.h"
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
        return; \
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

void ObjectEditor::addObject(Object *parent, Object *newChild, int insert_index)
{
    MO_DEBUG_TREE("ObjectEditor::addObject(" << parent << ", " << newChild << ", " << insert_index << ")");
    MO__CHECK_SCENE

    scene_->addObject(parent, newChild, insert_index);

    emit objectAdded(newChild);
}

void ObjectEditor::deleteObject(Object *object)
{
    MO_DEBUG_TREE("ObjectEditor::deleteObject(" << object << ")");
    MO__CHECK_SCENE

    scene_->deleteObject(object);

    emit objectDeleted(object);
}

void ObjectEditor::swapChildren(Object *parent, int from, int to)
{
    MO_DEBUG_TREE("ObjectEditor::swapChildren(" << parent << ", " << from << ", " << to << ")");
    MO__CHECK_SCENE

    scene_->swapChildren(parent, from, to);

    emit childrenSwapped(parent, from, to);
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
    {
        ScopedSceneLockWrite lock(scene_);
        p->addModulator(idName);
        p->collectModulators();
        p->object()->onParameterChanged(p);
        p->object()->updateParameterVisibility();
    }
    emit parameterChanged(p);
    scene_->render();
}

void ObjectEditor::removeModulator(Parameter *p, const QString &idName)
{
    {
        ScopedSceneLockWrite lock(scene_);
        p->removeModulator(idName);
        p->collectModulators();
        p->object()->onParameterChanged(p);
        p->object()->updateParameterVisibility();
    }
    emit parameterChanged(p);
    scene_->render();
}

void ObjectEditor::removeAllModulators(Parameter *p)
{
    {
        ScopedSceneLockWrite lock(scene_);
        p->removeAllModulators();
        p->collectModulators();
        p->object()->onParameterChanged(p);
        p->object()->updateParameterVisibility();
    }
    emit parameterChanged(p);
    scene_->render();
}


} // namespace MO
