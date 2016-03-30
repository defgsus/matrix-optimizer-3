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
#ifndef MO_DISABLE_FRONT
namespace GUI { class AbstractFrontItem; }
#endif

/** An edit-function-wrapper for creating and modifying Object trees.
    It issues signals for all edit actions.
    It contains some nice wrapper functions to create modulations and such. */
class ObjectEditor : public QObject
{
    Q_OBJECT
public:
    explicit ObjectEditor(QObject *parent = 0);

    /** Sets the scene to edit */
    void setScene(Scene * s) { scene_ = s; }
    Scene * scene() const { return scene_; }

    /** Returns a name for the object modulating the parameter.
        @p longName expands the parameter with it's object's name(s) */
    static QString modulatorName(Parameter * p, bool longName = false);

    // --------------- ids & helper --------------

    /** Returns a string that is unique among the @p existingNames.
        If a match is found, a counter is added to the idName.
        Also, any whitespace is relpaced with underscores.
        If @p existed != NULL, it will be set to true, if
        the id needed to be changed. */
    static QString getUniqueId(QString id, const QSet<QString> &existingNames, bool * existed = 0);

    /** Make all ids in newBranch unique among itself and the ids in root.
        Also cares for modulator ids and all that (calls Object::idNamesChanged()). */
    static void makeUniqueIds(const Object * root, Object * newBranch);

    /** Make all ids in all branches unique among themselves and the ids in root.
        Also cares for modulator ids and all that Object::idNamesChanged(). */
    static void makeUniqueIds(const Object* root, const QList<Object*> newBranches);

signals:

    /** Very broard signal.
        Emitted for everything except
            objectNameChanged() and parameterChanged(). */
    void sceneChanged(MO::Scene*);

    /** Emitted after a change to an object name */
    void objectNameChanged(MO::Object *);

    /** Emitted after settings in an object have changed.
        This includes number of audio channels, and the modulator outputs. */
    void objectChanged(MO::Object *);

    /** Emitted when the DT_HUE value has been changed */
    void objectColorChanged(MO::Object *);

    /** Emitted when the given object was added to the scene. */
    void objectAdded(MO::Object *);

    /** Emitted after a bunch of objects have been added */
    void objectsAdded(const QList<MO::Object*>&);

    /** Emitted when the @p object moved from the children list of @p oldParent
        to it's new parent */
    void objectMoved(MO::Object * object, MO::Object * oldParent);

    /** Emitted when the object was deleted.
        The object pointer will not point to a valid object anymore. */
    void objectDeleted(const MO::Object *);

    /** Emitted after all the objects where deleted.
        The object pointers will not point to valid objects. */
    void objectsDeleted(const QList<MO::Object*>&);

    /** A parameter has been changed with setParameterValue() */
    void parameterChanged(MO::Parameter*);

    /** A parameters visibility has changed (with isVisible() or isVisibleInGraph()) */
    void parameterVisibilityChanged(MO::Parameter*);

    /** All parameter widgets should be recreated */
    void parametersChanged(MO::Object*);

    /** A sequence has been changed somehow */
    void sequenceChanged(MO::Sequence*);

    /** A general ValueFloatInterface object has been changed */
    void valueFloatChanged(MO::ValueFloatInterface*);

    /** Emitted when a Modulator has been added */
    void modulatorAdded(MO::Modulator*);

    /** Emitted when a Modulator has been deleted. */
    void modulatorDeleted(const MO::Modulator*);

    /** A bunch of modulators has been deleted */
    void modulatorsDeleted(const QList<MO::Modulator*>& mods);

    /** Emitted in response to connectAudioObjects() or disconnectAudioObjects() */
    void audioConnectionsChanged();

public slots:

    // ----------- signal wrapper --------------

    /** Emits the appropriate signals */
    void emitObjectChanged(Object * );

    /** Emits the appropriate signals */
    void emitAudioChannelsChanged(Object * );

    // ------------ tree editing ---------------

    /** Adds the object @p newChild to the @p parent.
        @p newChild is completely given away. If it can't be added to parent, it will be deleted
        and a message is displayed to the user. */
    bool addObject(Object * parent, Object * newChild, int insert_index = -1);

    /** Adds the list of objects to @p parent.
        The items in @p newObjects are completely given away. If they can't be added to parent,
        they will be deleted and a message is displayed to the user. */
    bool addObjects(Object * parent, const QList<Object*> newObjects, int insert_index = -1);

    bool deleteObject(Object * object);

    bool deleteObjects(const QList<Object*>& list);

    bool deleteChildren(Object * object);

    /** Changes the position of an object among it's siblings */
    bool setObjectIndex(Object * object, int newIndex);

    /** Moves the @p object to a new position under @p newParent.
        If the object's current parent and @p newParent are the same,
        this call simplifies to setObjectIndex() */
    bool moveObject(Object * object, Object * newParent, int newIndex = -1);

    //bool groupObjects(const QList<Object*>& list);

    // -------- object specific funcs --------

    /** Works the same as addObject().
        If @p object is a ValueTextureInterface and @p newObject has a texture input,
        they will be connected. */
    void appendTextureProcessor(Object * object, Object * newObject, int insert_index = -1);

    /** Splits sequence @s at global time, creates new sequence.
        @returns the second half of the sequence, or NULL if globalTime is
        not within sequence time. */
    Sequence* splitSequence(Sequence* s, Double globalTime);

    // ----------- properties ------------------

    /** Changes the objects name, emits objectNameChanged() */
    void setObjectName(Object * object, const QString& name);

    /** Sets the hue for the object color, -1 for gray */
    void setObjectHue(Object * object, int hue);

    // ------------- parameter -----------------

    void setParameterValue(MO::ParameterInt *, Int value);
    void setParameterValue(MO::ParameterFloat *, Double value);
    void setParameterValue(MO::ParameterSelect *, int value);
    void setParameterValue(MO::ParameterFilename *, const QString& value);
    void setParameterValue(MO::ParameterText *, const QString& value);
    void setParameterValue(MO::ParameterTimeline1D *, const MATH::Timeline1d& value);
    void setParameterValue(MO::ParameterImageList *, const QStringList& value);
    void setParameterValue(MO::ParameterFont*, const QFont& value);

    void setParameterUserName(MO::Parameter*, const QString& name);

    void setParameterVisibleInGraph(MO::Parameter *, bool enbale);
    void setParameterVisibleInterface(MO::Parameter *, bool enbale);

    // ----------- audio cons ------------------

    /** Creates a connection between the Objects.
        Displays message and returns false when the connection would
        create a loop. */
    bool connectAudioObjects(MO::AudioObject * from, MO::AudioObject * to,
                             uint outChannel = 0, uint inChannel = 0,
                             uint numChannels = 1);

    bool disconnectAudioObjects(const AudioObjectConnection&);
    bool disconnectAudioObjects(MO::AudioObject * from, MO::AudioObject * to,
                                uint outChannel = 0, uint inChannel = 0,
                                uint numChannels = 1);

    // ------------ modulators -----------------

    bool addModulator(MO::Parameter *, const QString& idName, const QString &outputId);
    bool removeModulator(MO::Parameter *, const QString& idName, const QString& outputId);
    bool removeAllModulators(MO::Parameter *);

    // ---------- ui modulators ----------------
#ifndef MO_DISABLE_FRONT
    /** Creates a connection between an ui-item and a parameter,
        if types do match. */
    bool addUiModulator(MO::Parameter *, MO::GUI::AbstractFrontItem *);

    /** Removes the ModulatorObject previously created for the ui item with id @p uiId. */
    bool removeUiModulator(const QString& uiId);

    /** Removes the ModulatorObjects that belong to the ui items. */
    bool removeUiModulators(const QList<QString>& uiIds);

    /** Propagates a value from an ui-item to the appropriate ModulatorObjectFloat */
    bool setUiValue(const QString& uiId, Double timeStamp, Float value);
#endif

    // ------------ modulator objects ----------

    /** Returns an object for which to place a modulator for @p param in. */
    Object * findAModulatorParent(MO::Parameter * parm);

    /** Creates a float track for the given parameter.
        The track is placed at the next suitable level,
        searchin the parameter's object and parents. */
    TrackFloat * createFloatTrack(MO::Parameter * p);

    /** Wraps the sequence into a track.
        If the sequence is already part of a track, nothing happens
        and NULL is returned. */
    TrackFloat * wrapIntoTrack(MO::SequenceFloat * seq);

    /** Creates the object of @p className in the @p clip and links to parameter.
        If @p clip is NULL, a clip (and even a ClipContainer are automatically created).
        The clip is placed at the next suitable position to the parameter,
        @throws Exception if anything goes wrong. */
    Object * createInClip(MO::Parameter * p, const QString& className, MO::Clip * clip = 0);

    SequenceFloat * createFloatSequenceFor(MO::Parameter * p);

private:

    Scene * scene_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_OBJECTEDITOR_H
