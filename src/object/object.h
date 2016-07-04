/** @file object.h

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT_H
#define MOSRC_OBJECT_OBJECT_H

#include <vector>
#include <functional>

#include <QCoreApplication> // for Q_DECLARE_TR_FUNCTIONS()
#include <QByteArray>
#include <QList>
#include <QSet>
#include <QMap>

#include "object_fwd.h"
#include "types/refcounted.h"
#include "interface/valuetransformationinterface.h"
#include "interface/evolutioneditinterface.h"
#include "types/int.h"
#include "types/vector.h"
#include "types/time.h"
#include "io/filetypes.h"

/** Maximum time in seconds (for widgets mainly) */
#define MO_MAX_TIME (60 * 60 * 1000) // 1000 hours

/** The thread for gui-elements, e.g. drawing sequence content and such. */
#define MO_GUI_THREAD (0)
/** The one gfx thread. In theory, there are many more possible.
    Most of the framework supports that already. */
#define MO_GFX_THREAD (1)
/** The one audio thread. Again, many more are possible, probably for splitting the
    tree and doing calculations in parallel. */
#define MO_AUDIO_THREAD (2)

namespace MO {
namespace IO { class DataStream; }
namespace MATH { class Timeline1d; }

/** "Private" access to certain functionality of objects,
    that is not part of the interface */
class ObjectPrivate
{
public:
    /** Installs the object in ObjectFactory.
        @note Use MO_REGISTER_OBJECT to do the job */
    static bool registerObject(Object *);
    /** hidden id access */
    static void setObjectId(Object * o, const QString& id);
    /** Call to Object::addObject_() */
    static void addObject(Object * parent, Object * newChild, int index = -1);
    /** Call to parent's Object::deleteObject_() */
    static void deleteObject(Object * o);
    static void deleteChildren(Object * parent);
};

#define MO_REGISTER_OBJECT(class__) \
    namespace { \
        static bool success_register_object_##class__ = \
            ::MO::ObjectPrivate::registerObject( new class__() ); \
    }

#define MO_OBJECT_CONSTRUCTOR(Class__) \
    protected: ~Class__(); \
    public: explicit Class__(); \
    virtual Class__ * cloneClass() const Q_DECL_OVERRIDE { return new Class__(); } \
    static const QString& staticClassName() { static QString s(#Class__); return s; } \
    virtual const QString& className() const Q_DECL_OVERRIDE { return staticClassName(); } \
    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE; \
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

#define MO_ABSTRACT_OBJECT_CONSTRUCTOR(Class__) \
    protected: ~Class__(); \
    public: explicit Class__(); \
    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE; \
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

#define MO_ABSTRACT_OBJECT_CONSTRUCTOR_2(Class__, p1__, p2__) \
    protected: ~Class__(); \
    public: explicit Class__(p1__, p2__); \
    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE; \
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

#define MO_ABSTRACT_OBJECT_CONSTRUCTOR_3(Class__, p1__, p2__, p3__) \
    protected: ~Class__(); \
    public: explicit Class__(p1__, p2__, p3__); \
    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE; \
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

/** Abstract base of all Objects in MO

*/
class Object : public RefCounted
             , public ValueTransformationInterface
             , public EvolutionEditInterface
{
    Q_DECLARE_TR_FUNCTIONS(Object)

    friend class ObjectPrivate;
    // to edit the tree
    friend class Scene;
public:

    // -------------- types ------------------

    /** must be bits!
        order can change between runs. */
    enum Type
    {
        T_NONE              = 0,
        T_OBJECT            = 1<<0,
        T_GROUP             = 1<<1,
        T_TRANSFORMATION    = 1<<2,
        T_TRANSFORMATION_MIX= 1<<3,
        T_SCENE             = 1<<4,
        T_MICROPHONE        = 1<<5,
        T_CAMERA            = 1<<6,
        T_SOUNDSOURCE       = 1<<7,
        T_SEQUENCEGROUP     = 1<<8,
        T_SEQUENCE_FLOAT    = 1<<9,
        T_TRACK_FLOAT       = 1<<10,
        T_DUMMY             = 1<<11,
        T_LIGHTSOURCE       = 1<<12,
        T_GEOMETRY          = 1<<13,
        T_MODULATOR_OBJECT_FLOAT = 1<<14,
        T_MICROPHONE_GROUP  = 1<<15,
        T_CLIP              = 1<<16,
        T_CLIP_CONTROLLER    = 1<<17,
        T_SOUND_OBJECT      = 1<<18,
        T_OSCILLATOR        = 1<<19,
        T_AUDIO_OBJECT      = 1<<20,
        T_ANGELSCRIPT       = 1<<21,
        T_PYTHON            = 1<<22,
        T_SHADER            = 1<<23,
        T_TEXTURE           = 1<<24,
        T_TEXT              = 1<<25, //! a text source of some kind
        T_CONTROL           = 1<<26 //! generator of control signals, not specified otherwise
    };
    enum TypeGroups
    {
        /** Objects that have a definite position */
        TG_REAL_OBJECT      = T_OBJECT | T_GROUP | T_MICROPHONE | T_SOUNDSOURCE
                                | T_CAMERA | T_LIGHTSOURCE | T_MICROPHONE_GROUP
                                | T_SOUND_OBJECT | T_SHADER,

        TG_TRACK            = T_TRACK_FLOAT,
        TG_SEQUENCE         = T_SEQUENCE_FLOAT,

        TG_FLOAT            = T_TRACK_FLOAT | T_SEQUENCE_FLOAT,
        /** All explicit modulator objects */
        TG_MODULATOR_OBJECT = T_MODULATOR_OBJECT_FLOAT,
        /** All objects that can serve as a modulator source */
        TG_MODULATOR        = TG_MODULATOR_OBJECT
                                | TG_TRACK | TG_SEQUENCE | T_SEQUENCEGROUP
                                | T_OSCILLATOR | T_CONTROL,

        TG_TRANSFORMATION   = T_TRANSFORMATION | T_TRANSFORMATION_MIX,

        TG_SCRIPT           = T_ANGELSCRIPT | T_PYTHON,

        TG_ALL = 0xffffffff
    };

    enum ActivityScope
    {
        AS_OFF          = 0,
        AS_PREVIEW_1    = 1<<0,
        AS_PREVIEW_2    = 1<<1,
        AS_PREVIEW_3    = 1<<2,
        AS_RENDER       = 1<<3,
        AS_CLIENT_ONLY  = 1<<4,
        AS_PREVIEW      = AS_PREVIEW_1 | AS_PREVIEW_2 | AS_PREVIEW_3,
        AS_ON           = AS_PREVIEW | AS_RENDER,
    };

    /** ORDER MUST NOT CHANGE! */
    enum DataType
    {
        /** Position in ObjectGraphView (QPoint) */
        DT_GRAPH_POS,
        /** Expanded-flag in ObjectGraphView (bool) */
        DT_GRAPH_EXPANDED,
        /** Object hue (int) */
        DT_HUE,
        /** @deprecated */
        DT_CLIP_COLUMN,
        /** @deprecated */
        DT_CLIP_ROW,
        /** Parameter-group-id expanded (bool) */
        DT_PARAM_GROUP_EXPANDED,
        DT_SELECTED_PARAM,
        /** Height of a track in TrackView (int) */
        DT_TRACK_HEIGHT,
        /** Value of vertical scrollbar in ParmaterView (int) */
        DT_PARAM_VIEW_Y,
        /** Expanded or collapsed in ObjectTreeView (bool) */
        DT_TREE_VIEW_EXPANDED
    };

    // -------------- ctor -------------------
protected:
    /** Constructs a new object.
        @warning Never construct an object yourself, it will not suffice.
        Always use ObjectFactory::createObject().
        */
    explicit Object();

    ~Object();

public:

    /** Creates a new instance of the class.
        In derived classes this will be defined via the MO_OBJECT_CONSTRUCTOR() macro.
        @note Never call cloneClass() yourself, it will not suffice.
        Always use ObjectFactory::createObject(). */
    virtual Object * cloneClass() const = 0;

    /** Copies parameters and data from @p other.
        Override to copy additional data. */
    virtual void copyFrom(const Object* other);

    // ---------------- info --------------------

    void dumpTreeIds(std::ostream& out, const std::string &prefix = "") const;

    // ----------------- io ---------------------

    /** Serializes the whole tree including this object. */
    void serializeTree(IO::DataStream&) const;

    /** Creates a parent-less object containing the whole tree as
        previously created by serializeTree.
        On data or io errors, an IO::IoException will be thrown.
        Unknown objects will be replaced by the Dummy object.
        The constructed object can be added to a parent afterwards. */
    static Object * deserializeTree(IO::DataStream&);

    /** Serializes the whole tree including this object and
        returns the zipped data. */
    QByteArray serializeTreeCompressed() const;

    /** Same as deserializeTree() but for zipped data. */
    static Object * deserializeTreeCompressed(const QByteArray&);

    /** Override to store custom data.
        @note Always call the ancestor's serialize() function
        before your derived code!
        @note Always provide the serialize()/deserialize() methods for your classes,
        even if you do not have stuff to store yet and use
        IO::DataStream::writeHeader() to write your specific object version.
        Adding the serialize function later will definitely break previously saved files! */
    virtual void serialize(IO::DataStream&) const;
    /** Override to store custom data after the child object tree has been written.
        If this function returns false, deserializeAfterChilds() will not be called
        when reading the object - So never return false when you have written something.
        Base implementation returns false. */
    virtual bool serializeAfterChilds(IO::DataStream&) const { return false; }

    /** Override to restore custom data.
        @note See notes for serialize() function. */
    virtual void deserialize(IO::DataStream&);
    /** Override to restore custom data after all object childs have been deserialized. */
    virtual void deserializeAfterChilds(IO::DataStream&) { }

    QString getIoLoadErrors() const;

    // --------------- getter -------------------

    /** Name of the object class, for creating objects at runtime.
        MUST NOT CHANGE for compatibility with saved files! */
    virtual const QString& className() const = 0;
    /** Tree-unique id of the object. */
    const QString& idName() const;
    /** User defined name of the object */
    const QString& name() const;
    /** Override to add some additional information, Base returns name() */
    virtual QString infoName() const;
    /** Override to return potential multiline info about
        the object's state */
    virtual QString infoString() const { return QString(); }

    /** Return the path up to this object */
    QString namePath() const;

    /** Return the id path up to this object */
    QString idNamePath() const;

    virtual bool isValid() const { return true; }
    /** Returns whether the Object should be displayed to the user. */
    virtual bool isVisible() const;

    virtual Type type() const { return T_NONE; }
    virtual bool isScene() const { return false; }
    virtual bool isGl() const { return false; }
    virtual bool isGeometry() const { return false; }
    virtual bool isTransformation() const { return false; }
    virtual bool isSoundSource() const { return false; }
    virtual bool isMicrophone() const { return false; }
    virtual bool isCamera() const { return false; }
    virtual bool isParameter() const { return false; }
    virtual bool isTrack() const { return false; }
    virtual bool isSequence() const { return false; }
    virtual bool isClip() const { return false; }
    virtual bool isClipController() const { return false; }
    virtual bool isLightSource() const { return false; }
    virtual bool isAudioUnit() const { return false; }
    virtual bool isModulatorObject() const { return false; }
    virtual bool isAudioObject() const { return false; }
    virtual bool isScript() const { return false; }
    virtual bool isShader() const { return false; }
    virtual bool isTexture() const { return false; }
    virtual bool isText() const { return false; }

    /** The base class method returns whether any of the Parameters of
        the object are modulated. */
    virtual bool isModulated() const;

    /** Returns true when this object or any of it's childs or sub-childs
        contains microphones or soundsources. */
    bool isAudioRelevant() const;

    /** True for audio generating objects */
    virtual bool hasAudioOutput() const { return false; }

    /** Returns true when there are transformation objects among the children. */
    bool hasTransformationObjects() const
        { return !transformationObjects().isEmpty(); }

    /** Returns true when the object can be deleted by the ObjectTreeView */
    bool canBeDeleted() const;

    /** Returns a name that is unique among the direct children of the object */
    QString makeUniqueName(const QString& name) const;

    // ----------- attached data ------------------

    /** Attaches data to the object.
        The data is saved with the object.
        A null QVariant removes the entry */
    void setAttachedData(
            const QVariant& value, DataType type, const QString& id = "");

    /** Returns the attached data, or a null QVariant */
    QVariant getAttachedData(DataType type, const QString& id = "") const;

    /** Returns true if there is a set entry */
    bool hasAttachedData(DataType, const QString& id = "") const;

#ifdef QT_DEBUG
    /** Uses qDebug() */
    void dumpAttachedData() const;
#endif

    /** Returns the default or adjusted color of the object */
    QColor color() const;

    // ---------- activity (scope) ----------------

    /** Returns the user-set activity scope for the object */
    ActivityScope activityScope() const;

    /** Returns the currently set scope for the tree */
    ActivityScope currentActivityScope() const;

    /** Changes the activity scope for the object */
    void setActivityScope(ActivityScope, bool sendGui = false);

    /** Returns if the object is active at the given time */
    bool active(const RenderTime& time) const;

    /** Returns if the object fits the currently set activity scope */
    bool activeAtAll() const;

    // --------------- setter -------------------

    /** Set the user-name for the object */
    void setName(const QString&);

    /** Recursively sets the activity scope for the whole tree.
        This will tell the objects, which scope is active and
        will be reflected in their active() method. */
    void setCurrentActivityScope(ActivityScope scope);

    /** Sets the visibility flag, nothing else.
        This flag should be set before the object is exposed to any views. */
    void setVisible(bool v);

    // ---------- tree getter --------------------

    /** Test if object @p newChild can be added to this object.
        This checks for type compatibility as well as for
        potential loops in the modulation path. */
    bool isSaveToAdd(Object * newChild, QString& error) const;

    /** Returns the root object of this hierarchy, which may
        be the object itself. */
    Object * rootObject() const;

    /** Returns the Scene object (also for the scene itself), or NULL */
    Scene * sceneObject() const;

    /** Returns the editor attached to the tree, or NULL */
    ObjectEditor * editor() const;

    /** Returns the parent Object, or NULL */
    Object * parentObject() const;

    /** See if this object has a parent object @p o. */
    bool hasParentObject(const Object * o) const;

    /** Returns the first parent object matching given Type mask, or NULL */
    Object * findParentObject(int typeFlags) const;

    /** Returns the common parent for this and @p other,
        or NULL if at least one of them is root */
    Object * findCommonParentObject(Object * other) const;

    /** Returns this object or the first parent that matches
        the Object::Type mask in @p typeFlags.
        If none matches, returns NULL */
    Object * findContainingObject(int typeFlags);
    /** Returns this object or the first parent that matches
        the Object::Type mask in @p typeFlags.
        If none matches, returns NULL */
    const Object * findContainingObject(int typeFlags) const;

    /** Returns true if this object or any of it's children match
        the Object::Type mask in @p typeFlags */
    bool containsTypes(int typeFlags) const;

    /** Returns true if this object or any of it's childs are
        equal to @p o */
    bool containsObject(Object * o) const;

    /** Returns number of direct childs or number of all sub-childs. */
    int numChildren(bool recursive = false) const;

    /** Returns true if an Object of @p type can be a children of this Object. */
    virtual bool canHaveChildren(Type type) const;

    /** Read-access to the list of childs */
    const QList<Object*>& childObjects() const;

    /** Returns a set of all idNames */
    QSet<QString> getChildIds(bool recursive) const;

    /** Returns the object matching the path in @p namePath.
        @p namePath can be of the form '/name1/name2' or 'name1/name2'.
        In the first form, name1 is expected to be a direct child of this object,
        in the latter form, name1 can be anywhere down the branches.
        Returns NULL, if the object was not found. */
    Object * findObjectByNamePath(const QString& namePath) const;

    /** Helper function for the other findObjectByIdPath() method. */
    Object * findObjectByNamePath(const QStringList& names, int offset = 0, bool firstIsRoot = false) const;

    /** Returns the first object, including self, for which @p selector returns true,
        or NULL. */
    Object * findChildObject(std::function<bool(Object*)> selector, bool recursive = true);

    /** Returns the children with the given id, or NULL.
        If @p ignore is not NULL, this object will be ignored by search. */
    Object * findChildObject(const QString& id, bool recursive = false, Object * ignore = 0) const;

    /** Returns a list of all objects that match one of the flags in @p typeFlags,
        where typeFlags is an or-combination of Object::Type enums. */
    QList<Object*> findChildObjects(int typeFlags, bool recursive = false) const;

    /** Returns the children of type @p T with the given id.
        if @p id is empty, all objects of type T will be returned.
        If @p ignore is not NULL, this object will be ignored by search
        (but it's child objects will be considered as well). */
    template <class T>
    QList<T*> findChildObjects(
            const QString& id = QString(), bool recursive = false, Object * ignore = 0) const;

    /** Returns the children of type @p T for which @p selector returns true.
        If @p ignore is not NULL, this object will be ignored by search
        (but it's child objects will be considered as well). */
    template <class T>
    QList<T*> findChildObjects(std::function<bool(T*)> selector,
                               bool recursive = false,
                               Object * ignore = 0) const;

    /** Returns the children of type @p T with the given id, or NULL.
        if @p id is empty, all objects of type T will be returned.
        @p stopAt and all it's children will be ignored by the search. */
    template <class T>
    QList<T*> findChildObjectsStopAt(
            const QString& id, bool recursive, Object * stopAt) const;

    /** Returns the index of the last child object of type @p T */
    template <class T>
    int indexOfLastChild(int last = -1) const;

    /** Adds the tree to the map */
    void getIdMap(QMap<QString, Object*>& idMap) const;

    bool haveChildrenChanged() const;

    // ------------- tree stuff -----------------

    /** Needed for ObjectGl. Base implementation calls propagteRenderMode() for
        all children. */
    virtual void propagateRenderMode(ObjectGl * parent);

//protected:

    /** Tells the whole branch including the object itself about changed ids.
        The map maps from old id to new id.
        The idNames() of the objects in this branch will already be changed,
        and each object gets onIdNamesChanged called. */
    void idNamesChanged(const QMap<QString,QString>&);

    /** Called when the idNames of objects have changed.
        This happens if a new object/branch is inserted into an existing branch.
        The map maps from old id to new id.
        Call ancestor's code in your derived function!
        Base implementation does nothing. */
    virtual void onIdNamesChanged(const QMap<QString,QString>&) { }

    /** Called when the children list has changed */
    virtual void childrenChanged() { }

    /** Called when an object and it's subtree have been deleted.
        The objects in @p list are still existing!
        @note Call ancestor's implementation before your derived code!
        Object's base implementation removes all modulators from parameters
        that point to deleted objects. */
    virtual void onObjectsAboutToDelete(const QList<Object*>& list);

    /** Called when the parent of this object has changed.
        This happens when the object has been added to or moved in tree.
        @note Call ancestor's implementation before your derived code! */
    virtual void onParentChanged() { }

    /** Override to set the visibility of Parameters.
        This function is called after parameters are loaded
        and after a parameter change.
        @note Call ancestor's implementation before your derived code! */
    virtual void updateParameterVisibility() { }

    /** This is called when Scene::installDependency() was called,
        and the requested object made an action.
        XXX Will be refined later, currently only used for TextObject */
    virtual void onDependency(Object * ) { }

public:

    /** Returns the number of threads, this object is assigned for */
    uint numberThreads() const;

    /** Returns true if number of threads is matching @p num.
        This checks for all contained stuff like AudioSources as well.
        @note Call ancestor's implementation before your derived code! */
    virtual bool verifyNumberThreads(uint num);

    /** Sets the number of threads that will run on this object.
        Any mutable values of the object must be present @p num times!
        @note Call ancestor's implementation before your derived code! */
    virtual void setNumberThreads(uint num);

    // ---------- only callable by scene or ObjectPrivate -----------------
private:

    /** Adds the object to the list of childs.
        The object is (re-)parented to this object.
        If @p insert_index is >= 0, the object will be
        inserted before the indexed object (e.g. 0 = start, 1 = before second ..).
        The object will be removed from the previous parent's child list.
        The idName() will be adjusted if needed.
        @returns The added object. */
    Object * addObject_(Object * object, int insert_index = -1);

    /** Installs the object in the parent object's childlist.
        If @p insert_index is >= 0, the object will be
        inserted before the indexed object (e.g. 0 = start, 1 = before second).
        This call is equivalent to calling parent->addObject(this).
        This object will be removed from the previous parent's child list.
        The idName() will be adjusted if needed. */
    void setParentObject_(Object * parent, int insert_index = -1);

    /** Exchange the two child object given by the indices. */
    void swapChildren_(int from, int to);

    /** Deletes the child from the list of children, if found,
        and destroys the object itself if @p destroy is true.
        @p child will have it's QObject parent set to NULL.
        Also childrenChanged_ is set to true for this class. */
    void deleteObject_(Object * child, bool destroy);

    /** Sets a new index for the children object */
    bool setChildrenObjectIndex_(Object * child, int newIndex);

public:
    // --------------- modulators ------------------

    /** Collect objects that you need here.
        XXX Only used by TrackFloat a.t.m. */
    virtual void collectModulators() { };

    /** Removes all modulator ids from all parameters for which
        no modulator object has been found. */
    void removeNullModulators(bool recursive);

    /** Removes all modulators from all parameters, who's ids
        are not in the current tree.
        @note low-level function, not really part of interface. */
    void removeOutsideModulators(bool recursive);

    /** Removes all modulators with the given ids.
        @note low-level function, not really part of interface. */
    void removeModulators(const QList<QString>& modulatorIds, bool recursive);

    /** Returns all modulators of all parameters of this object.
        If @p recursive is true, all childs will add their Modulators too. */
    QList<Modulator*> getModulators(bool recursive = false) const;

    /** Adds all connections of objects that modulate this object to the graph.
        If @p recursive is true, all connections from other modulators to the
        modulators of this object are traced recursively.
        Essentially call Parameter::getModulatingObjects() for each parameter. */
    virtual void getModulatingObjects(ObjectConnectionGraph&, bool recursive) const;

    /** Calls getModulatingObjects() and returns the ObjectConnectionGraph::makeLinear()
        list without this object itself. */
    QList<Object*> getModulatingObjectsList(bool recursive) const;

    /** Returns the list of all Parameters that are modulated.
        Each entry is a pair of the Parameter and the modulating object.
        Multiple modulations on the same Parameter have multiply entries in the list. */
    virtual QList<QPair<Parameter*, Object*>> getModulationPairs() const;

    /** Adds all connections from all objects + the objects in @p scene that would
        modulate any of this object's parameters, if the object was added to the scene. */
    virtual void getFutureModulatingObjects(
            ObjectConnectionGraph& graph, const Scene * scene, bool recursive) const;

    /** Returns a list of objects that will modulate this object
        when it gets added to the scene. */
    virtual QList<Object*> getFutureModulatingObjectsList(
            const Scene * scene, bool recursive) const;

    /** Returns the source for the include url, or an empty string.
        If @p object is non NULL the pointed-to pointer will be set to
        the found object pointer. */
    QString getGlslInclude(const QString& url, bool include_system_defaults,
                           Object** object = 0) const;

    // ------------ modulator outputs ----------------

    /** Returns the number of desired outputs for the specific signal type */
    uint getNumberOutputs(SignalType ) const;

    /** Returns a map with the number of outputs per signal type */
    const QMap<SignalType, uint>& getNumberOutputs() const;

    /** Returns the name of the specific output.
        Override to change names. */
    virtual QString getOutputName(SignalType, uint channel) const;

    /** Returns the name of the signal type.
        XXX Found no better place just yet */
    static QString getSignalName(SignalType);

    /** Sets the desired number of outputs per signal type.
        If @p emitSignal is true, the ObjectEditor, if attached, will be notified. */
    void setNumberOutputs(SignalType t, uint num, bool emitSignal = true);

    /** Signals a change of connections through the ObjectEditor, if attached. */
    void emitConnectionsChanged();

    // --------------- parameter -------------------
public:

    /** Returns the list of parameters for this object */
    const Parameters * params() const;
    Parameters * params();

    /** Override to create all parameters for your object.
        Always call the ancestor classes createParameters() in your derived function! */
    virtual void createParameters();

    /** Called when a parameter has changed it's value (from the gui).
        Be sure to call the ancestor class implementation before your derived code! */
    virtual void onParameterChanged(Parameter * p);

    /** Called after all parameters of the object have been deserialized.
        createParameters() has alreay been called here.
        Be sure to call the ancestor class implementation before your derived code!
        XXX Right now it's a bit unclear, what is possible here except from lazy requests. */
    virtual void onParametersLoaded() { }

protected:

    /** Sets the given parameter group's expansion flag default value.
        The value is unchanged if it had already been changed.
        This can be used in createParameters() to set particular goups to expanded
        by default. */
    void initParameterGroupExpanded(const QString& groupId, bool expanded = true);

    // ------------------- audio ------------------
public:
    /** Returns the set sample rate in samples per second. */
    uint sampleRate() const;

    /** Returns the reciprocal of the set sample rate, e.g. 1.0 / sampleRate() */
    Double sampleRateInv() const;

    /** Sets the samplerate for the object.
        Override to initialize coefficients or stuff that depends on the samplerate.
        @note Be sure to call the ancestor class implementation in your derived method!
        */
    virtual void setSampleRate(uint samplerate);

    /** When the Object is loaded from a template via ObjectFactory::loadObject(),
        it will have it's internal tree audio connections here, so the scene
        can incorporate them. */
    AudioObjectConnections * getAssignedAudioConnections() const;

    /** Assigns audio connections to the object. This just copies the pointer.
        Only used for saving templates via ObjectFactory::saveObject()
        The ownership is taken. Assign NULL to clear existing. */
    void assignAudioConnections(AudioObjectConnections *);

    // ------------------ spatial audio -------------------

    uint numberSoundSources() const;

    /** Override to create all microphones for your object.
        @note Be sure to call the ancestor class implementation before your derived code! */
    uint numberMicrophones() const;

protected:

    void setNumberMicrophones(uint num);
    void setNumberSoundSources(uint num);

public:

    /** Override to update the transformations of the soundsources.
        The base implementation simply copies the object transformation. */
    virtual void calculateSoundSourceTransformation(
                                        const TransformationBuffer * objectTransformation,
                                        const QList<AUDIO::SpatialSoundSource*>&,
                                        const RenderTime& time);

    /** Override to fill the audio buffers of the sound sources.
        The base implementation does nothing. */
    virtual void calculateSoundSourceBuffer(const QList<AUDIO::SpatialSoundSource*> sources,
                                            const RenderTime& time)
    { Q_UNUSED(sources); Q_UNUSED(time); }

    /** Override to update the transformations of each microphone.
        The base implementation simply copies the object transformation. */
    virtual void calculateMicrophoneTransformation(
                                        const TransformationBuffer * objectTransformation,
                                        const QList<AUDIO::SpatialMicrophone*>&,
                                        const RenderTime& time);

    /** Override to sample or change the current dsp-block of each virtual microphone. */
    virtual void processMicrophoneBuffers(
            const QList<AUDIO::SpatialMicrophone*>& microphones, const RenderTime& time)
        { Q_UNUSED(microphones); Q_UNUSED(time); }
public:
    // --------------- 3d --------------------------

    // XXX transformations-per-object are just temporarily
    //     before a generic render class wraps this

    /** Initialize transformation matrix */
    void clearTransformation();
    /** Sets the transformation matrix */
    void setTransformation(const Mat4& mat);

    /** Returns the transformation matrix of this object */
    const Mat4& transformation() const;

    /** ValueTransformationInterface */
    virtual Mat4 valueTransformation(
            uint /*channel*/, const RenderTime& /*time*/) const Q_DECL_OVERRIDE;

    /** Returns the position of this object */
    Vec3 position() const;

    /** Base implementation applies all transformation objects inside this object to the given matrix.
        XXX Made virtual to override Camera's matrix... */
    virtual void calculateTransformation(Mat4& matrix, const RenderTime& time) const;

    /** List of all direct transformation childs */
    const QList<Transformation*>& transformationObjects() const;

    // ----------------- errors ----------------------

    bool hasError() const;
    const QString& errorString() const;
    void clearError();
    /** Call this during initialization to signal an error to the gui/user.
        Passing an empty string does nothing. Otherise, error strings are
        accumulated (with newline).
        This function is const to be called from anywhere but, of course,
        changes the errorString() */
    void setErrorMessage(const QString& errorString) const;

    // ------------------ files ----------------------

    /** Should return the list of files, this object needs, by appending to the list.
        The method is called, regardless of the activity scope of the object.
        Derived classes should return any potentially needed files.
        More is better than not enough, in this case.
        Always call the ancestor's method before your derived code. */
    virtual void getNeededFiles(IO::FileList & files) { Q_UNUSED(files); }

    // -------------- EvolutionEditInterface ---------

    virtual const EvolutionBase* getEvolution(const QString& key) const override;
    virtual void setEvolution(const QString& key, const EvolutionBase*) override;

    // _____________ PRIVATE AREA __________________

private:

    // disable copy
    Object(const Object&) = delete;
    void operator=(const Object&) = delete;

    /** Called on changes to the child list */
    void p_childrenChanged_();

    /** Removes the child from the child list, nothing else. */
    bool p_takeChild_(Object * child);

    struct PrivateObj;
    PrivateObj* pobj_;
};






// ---------------------- template impl -------------------


template <class T>
QList<T*> Object::findChildObjects(const QString& id, bool recursive, Object * ignore) const
{
    QList<T*> list;

    for (auto o : childObjects())
    {
        if (o != ignore
            && (dynamic_cast<T*>(o))
            && (id.isEmpty() || o->idName() == id))
                list.append(static_cast<T*>(o));

        if (recursive)
            list.append(o->findChildObjects<T>(id, recursive, ignore));
    }

    return list;
}

template <class T>
QList<T*> Object::findChildObjects(std::function<bool(T*)> selector,
                                   bool recursive,
                                   Object * ignore) const
{
    QList<T*> list;

    for (auto o : childObjects())
    {
        if (o != ignore
            && (dynamic_cast<T*>(o))
            && (selector(static_cast<T*>(o))))
                list.append(static_cast<T*>(o));

        if (recursive)
            list.append(o->findChildObjects<T>(selector, recursive, ignore));
    }

    return list;
}

template <class T>
QList<T*> Object::findChildObjectsStopAt(
        const QString& id, bool recursive, Object * stopAt) const
{
    QList<T*> list;

    for (auto o : childObjects())
    {
        if (o != stopAt
            && (dynamic_cast<T*>(o))
            && (id.isEmpty() || o->idName() == id))
                list.append(static_cast<T*>(o));

        if (recursive && o != stopAt)
            list.append(o->findChildObjectsStopAt<T>(id, recursive, stopAt));
    }

    return list;
}


template <class T>
int Object::indexOfLastChild(int last) const
{
    if (childObjects().empty())
        return -1;

    if (last < 0 || last >= childObjects().size())
        last = childObjects().size() - 1;

    for (int i = last; i>=0; --i)
    {
        if (dynamic_cast<T*>(childObjects()[i]))
            return i;
    }

    return -1;
}


} // namespace MO

Q_DECLARE_METATYPE(MO::Object*)

#endif // MOSRC_OBJECT_OBJECT_H
