/** @file objecteditor.h

    @brief Class for editing objects and signalling other widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.11.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_OBJECTEDITOR_H
#define MOSRC_OBJECT_UTIL_OBJECTEDITOR_H

#include <QObject>

#include "object/object_fwd.h"
#include "types/int.h"
#include "types/float.h"

namespace MO {

class ObjectEditor : public QObject
{
    Q_OBJECT
public:
    explicit ObjectEditor(QObject *parent = 0);

    /** Sets the scene to edit */
    void setScene(Scene * s) { scene_ = s; }
    Scene * scene() const { return scene_; }

signals:

    /** Emitted after a change to an object name */
    void objectNameChanged(MO::Object *);

    /** Emitted after settings in an object have changed. */
    void objectChanged(MO::Object *);

    /** Emitted when the given object was added to the scene. */
    void objectAdded(MO::Object *);

    /** Emitted when the object was deleted.
        The object pointer will not point to a valid object anymore. */
    void objectDeleted(const MO::Object *);

    /** Emitted after swapChildren() */
    void childrenSwapped(MO::Object *, int from, int to);

    /** A parameter has been changed with setParameterValue() */
    void parameterChanged(MO::Parameter*);

    /** A sequence has been changed somehow */
    void sequenceChanged(MO::Sequence*);

public slots:

    // ------------ tree editing ---------------

    void addObject(Object * parent, Object * newChild, int insert_index = -1);

    void deleteObject(Object * object);

    void swapChildren(Object * parent, int from, int to);

    /** Changes the objects name, emits objectNameChanged() */
    void setObjectName(Object * object, const QString& name);

    // ------------- parameter -----------------

    void setParameterValue(MO::ParameterInt *, Int value);
    void setParameterValue(MO::ParameterFloat *, Double value);
    void setParameterValue(MO::ParameterSelect *, int value);
    void setParameterValue(MO::ParameterFilename *, const QString& value);
    void setParameterValue(MO::ParameterText *, const QString& value);
    void setParameterValue(MO::ParameterTimeline1D *, const MATH::Timeline1D& value);

    // ------------ modulators -----------------

    void addModulator(MO::Parameter *, const QString& idName);
    void removeModulator(MO::Parameter *, const QString& idName);
    void removeAllModulators(MO::Parameter *);

private:

    Scene * scene_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_OBJECTEDITOR_H
