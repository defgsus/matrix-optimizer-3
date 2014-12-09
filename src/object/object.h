/** @file object.h

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT_H
#define MOSRC_OBJECT_OBJECT_H

#include <vector>

#include <QByteArray>
#include <QObject>
#include <QList>
#include <QSet>
#include <QMap>

#include "types/int.h"
#include "types/vector.h"
#include "object_fwd.h"
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
namespace MATH { class Timeline1D; }

#define MO_REGISTER_OBJECT(class__) \
    namespace { \
        static bool success_register_object_##class__ = \
            ::MO::registerObject_( new class__((QObject*)0) ); \
    }

#define MO_OBJECT_CONSTRUCTOR(Class__) \
    explicit Class__(QObject *parent = 0); \
    virtual Class__ * cloneClass() const Q_DECL_OVERRIDE { return new Class__(); } \
    virtual const QString& className() const Q_DECL_OVERRIDE { static QString s(#Class__); return s; } \
    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE; \
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

#define MO_ABSTRACT_OBJECT_CONSTRUCTOR(Class__) \
    explicit Class__(QObject *parent = 0); \
    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE; \
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

#define MO_ABSTRACT_OBJECT_CONSTRUCTOR_2(Class__, p1__, p2__) \
    explicit Class__(p1__, p2__, QObject *parent = 0); \
    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE; \
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

#define MO_ABSTRACT_OBJECT_CONSTRUCTOR_3(Class__, p1__, p2__, p3__) \
    explicit Class__(p1__, p2__, p3__, QObject *parent = 0); \
    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE; \
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

/** Abstract base of all Objects in MO

*/
class Object : public QObject
{
    Q_OBJECT

    // to set idName_
    friend class ObjectFactory;
    // to edit the tree
    friend class Scene;
public:

    // -------------- types ------------------

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
                                // 13
        T_MODULATOR_OBJECT_FLOAT = 1<<14,
        T_MICROPHONE_GROUP  = 1<<15,
        T_CLIP              = 1<<16,
        T_CLIP_CONTAINER    = 1<<17,
        T_SOUND_OBJECT      = 1<<18,
        T_OSCILLATOR        = 1<<19,
        T_AUDIO_OBJECT      = 1<<20
    };
    enum TypeGroups
    {
        /** Objects that have a definite position */
        TG_REAL_OBJECT      = T_OBJECT | T_GROUP | T_MICROPHONE | T_SOUNDSOURCE
                                | T_CAMERA | T_LIGHTSOURCE | T_MICROPHONE_GROUP
                                | T_SOUND_OBJECT,

        TG_TRACK            = T_TRACK_FLOAT,
        TG_SEQUENCE         = T_SEQUENCE_FLOAT,

        TG_FLOAT            = T_TRACK_FLOAT | T_SEQUENCE_FLOAT,
        /** All explicit modulator objects */
        TG_MODULATOR_OBJECT = T_MODULATOR_OBJECT_FLOAT,
        /** All objects that can serve as a modulator source */
        TG_MODULATOR        = TG_MODULATOR_OBJECT
                                | TG_TRACK | TG_SEQUENCE | T_SEQUENCEGROUP
                                | T_OSCILLATOR,

        TG_TRANSFORMATION   = T_TRANSFORMATION | T_TRANSFORMATION_MIX,

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

    enum DataType
    {
        /** Position in ObjectGraphView */
        DT_GRAPH_POS,
        /** Expanded-flag in ObjectGraphView */
        DT_GRAPH_EXPANDED
    };

    // -------------- ctor -------------------

    /** Constructs a new object.
        If @p parent is also an Object, this object will be installed in the
        parent's child list via setParentObject() or addObject().
        @note The @p parent parameter follows more QObject's style and is not really
        necessary here.
        @note More important: Never construct an object yourself, it will not suffice.
        Always use ObjectFactory::createObject().
        */
    explicit Object(QObject *parent = 0);

    ~Object();

    /** Creates a new instance of the class.
        In derived classes this will be defined via the MO_OBJECT_CONSTRUCTOR() macro.
        @note Never call cloneClass() yourself, it will not suffice.
        Always use ObjectFactory::createObject(). */
    virtual Object * cloneClass() const = 0;

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

    // --------------- getter -------------------

    /** Name of the object class, for creating objects at runtime.
        MUST NOT CHANGE for compatibility with saved files! */
    virtual const QString& className() const = 0;
    /** Tree-unique id of the object. */
    const QString& idName() const { return idName_; }
    /** User defined name of the object */
    const QString& name() const { return name_; }
    /** Override to add some additional information. */
    virtual QString infoName() const { return name_; }

    /** Returns the id of the object before it might have been changed through makeUniqueId()
        XXX Will be changed to work with attachedData() !!! */
    const QString& originalIdName() const { return orgIdName_; }

    /** Return the path up to this object */
    QString namePath() const;

    /** Return the id path up to this object */
    QString idNamePath() const;

    virtual bool isValid() const { return true; }

    virtual Type type() const { return T_NONE; }
    virtual bool isScene() const { return false; }
    virtual bool isGl() const { return false; }
    virtual bool isTransformation() const { return false; }
    virtual bool isSoundSource() const { return false; }
    virtual bool isMicrophone() const { return false; }
    virtual bool isCamera() const { return false; }
    virtual bool isParameter() const { return false; }
    virtual bool isTrack() const { return false; }
    virtual bool isSequence() const { return false; }
    virtual bool isClip() const { return false; }
    virtual bool isClipContainer() const { return false; }
    virtual bool isLightSource() const { return false; }
    virtual bool isAudioUnit() const { return false; }
    virtual bool isModulatorObject() const { return false; }
    virtual bool isAudioObject() const { return false; }

    /** The base class method returns whether any of the Parameters of
        the object are modulated. */
    virtual bool isModulated() const;

    /** Returns true when this object or any of it's childs or sub-childs
        contains microphones or soundsources. */
    bool isAudioRelevant() const;

    /** True for audio generating objects */
    virtual bool hasAudioOutput() const { return false; }

    /** Returns true when there are transformation objects among the children. */
    bool hasTransformationObjects() const { return !transformationObjects_.isEmpty(); }

    /** Returns true when the object can be deleted by the ObjectTreeView */
    bool canBeDeleted() const { return canBeDeleted_; }

    /** Returns a name that is unique among the direct children of the object */
    QString makeUniqueName(const QString& name) const;

    /** Returns a priority for each object type */
    static int objectPriority(const Object *);

    // ----------- attached data ------------------

    /** Attaches data to the object.
        The data is saved with the object.
        A null QVariant removes the entry */
    void setAttachedData(const QVariant& value, DataType type, const QString& id = "");

    /** Returns the attached data, or a null QVariant */
    QVariant getAttachedData(DataType type, const QString& id = "") const;

    /** Returns true if there is a set entry */
    bool hasAttachedData(DataType, const QString& id = "") const;

#ifdef QT_DEBUG
    /** Uses qDebug() */
    void dumpAttachedData() const;
#endif

    // ---------- activity (scope) ----------------

    /** Returns the user-set activity scope for the object */
    ActivityScope activityScope() const;

    /** Returns the currently set scope for the tree */
    ActivityScope currentActivityScope() const { return currentActivityScope_; }

    /** Returns if the object is active at the given time */
    bool active(Double time, uint thread) const;

    /** Returns if the object fits the currently set activity scope */
    bool activeAtAll() const;

    // --------------- setter -------------------

    /** Set the user-name for the object */
    void setName(const QString&);

    /** Recursively sets the activity scope for the whole tree.
        This will tell the objects, which scope is active and
        will be reflected in their active() method. */
    void setCurrentActivityScope(ActivityScope scope);

    // ---------- tree getter --------------------

    /** Test if object @p newChild can be added to this object.
        This checks for type compatibility as well as for
        potential loops in the modulation path. */
    bool isSaveToAdd(Object * newChild, QString& error) const;

    /** Returns the root object of this hierarchy, which may
        be the object itself. */
    const Object * rootObject() const;
          Object * rootObject();

    /** Returns the Scene object (also for the scene itself), or NULL */
    const Scene * sceneObject() const;
          Scene * sceneObject();

    /** Returns the parent Object, or NULL */
    Object * parentObject() const { return parentObject_; }

    /** See if this object has a parent object @p o. */
    bool hasParentObject(Object * o) const;

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
    const QList<Object*> childObjects() const { return childObjects_; }

    /** Returns a set of all idNames */
    QSet<QString> getChildIds(bool recursive) const;

    /** Returns the children with the given id, or NULL.
        If @p ignore is not NULL, this object will be ignored by search. */
    Object * findChildObject(const QString& id, bool recursive = false, Object * ignore = 0) const;

    /** Returns a list of all objects that match one of the flags in @p typeFlags,
        where typeFlags is an or-combination of Object::Type enums. */
    QList<Object*> findChildObjects(int typeFlags, bool recursive = false) const;

    /** Returns the children of type @p T with the given id, or NULL.
        if @p id is empty, all objects of type T will be returned.
        If @p ignore is not NULL, this object will be ignored by search
        (but it's child objects will be considered as well). */
    template <class T>
    QList<T*> findChildObjects(
            const QString& id = QString(), bool recursive = false, Object * ignore = 0) const;

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

    // ------------- tree stuff -----------------

    /** Returns a string that is unique among the @p existingNames.
        If a match is found, a counter is added to the idName.
        Also, any whitespace is relpaced with underscores.
        If @p existed != NULL, it will be set to true, if
        the id needed to be changed. */
    static QString getUniqueId(QString id, const QSet<QString> &existingNames, bool *existed = 0);

    /** Needed for ObjectGl. Base implementation calls propagteRenderMode() for
        all children. */
    virtual void propagateRenderMode(ObjectGl * parent);

//protected:

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

public:

    /** Returns the number of threads, this object is assigned for */
    uint numberThreads() const { return numberThreads_; }

    /** Returns true if number of threads is matching @p num.
        This checks for all contained stuff like AudioSources as well.
        @note Call ancestor's implementation before your derived code! */
    virtual bool verifyNumberThreads(uint num);

    /** Sets the number of threads that will run on this object.
        Any mutable values of the object must be present @p num times!
        @note Call ancestor's implementation before your derived code! */
    virtual void setNumberThreads(uint num);
#if (0)
    /** Sets the thread and dsp-block storage for the object.
        Override this to change the number of mutable values in your object.
        Always call the ancestor implementation in your derived function! */
    virtual void setThreadStorage(int threads, int blockSize);
#endif
    // ---------- only callable by scene -----------------
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

    virtual void collectModulators() { };

    /** Returns all modulators of all parameters of this object */
    QList<Modulator*> getModulators() const;

    /** Returns a list of objects that modulate this object. */
    virtual QList<Object*> getModulatingObjects() const;

    /** Returns the list of all Parameters that are modulated.
        Each entry is a pair of the Parameter and the modulating object.
        Multiple modulations on the same Parameter have multiply entries in the list. */
    virtual QList<QPair<Parameter*, Object*>> getModulationPairs() const;

    /** Returns a list of objects that will modulate this object
        when it gets added to the scene. */
    virtual QList<Object*> getFutureModulatingObjects(const Scene * scene) const;

    // --------------- outputs ---------------------

    /** Call this to get createOutputs() to be called. */
    void requestCreateOutputs();

    /** Create your ModulatorObject classes here.
        Use the createOutput...() functions! */
    virtual void createOutputs() { };

protected:

    /** Creates a new (or reuses an existing) ModulatorObjectFloat as child
        of this Object.
        The @p id will be adjusted, so don't rely on it.
        @note This function returns NULL, if another object with the same id
        is found which is not of ModulatorObjectFloat class! */
    ModulatorObjectFloat * createOutputFloat(const QString& id, const QString& name);

    // --------------- parameter -------------------
public:

    /** Returns the list of parameters for this object */
    const Parameters * params() const { return parameters_; }
    Parameters * params() { return parameters_; }

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


    // ------------------- audio ------------------
public:
    /** Returns the set audio block size for each thread. */
    uint bufferSize(uint thread) const { return bufferSize_[thread]; }

    /** Returns the set sample rate in samples per second. */
    uint sampleRate() const { return sampleRate_; }

    /** Returns the reciprocal of the set sample rate, e.g. 1.0 / sampleRate() */
    Double sampleRateInv() const { return sampleRateInv_; }


    /** Returns true if the buffersize of the thread is matching @p bufferSize.
        This checks for all contained stuff like AudioSources as well.
        @note Call ancestor's implementation before your derived code! */
    virtual bool verifyBufferSize(uint thread, uint bufferSize);

    /** Sets the audio block size for this object and the given thread.
        This function is only called <b>after</b> setNumberThreads().
        Override to make your per-thread-storage of dsp blocks.
        @note Be sure to call the ancestor class implementation in your derived method!
    */
    virtual void setBufferSize(uint bufferSize, uint thread);

    /** Sets the samplerate for the object.
        Override to initialize coefficients or stuff that depends on the samplerate.
        @note Be sure to call the ancestor class implementation in your derived method!
        */
    virtual void setSampleRate(uint samplerate);

    // ------------------ spatial audio -------------------

    uint numberSoundSources() const { return p_numberSoundSources_; }

    /** Override to create all microphones for your object.
        @note Be sure to call the ancestor class implementation before your derived code! */
    uint numberMicrophones() const { return p_numberMicrophones_; }

protected:

    void setNumberMicrophones(uint num);
    void setNumberSoundSources(uint num);

public:

    /** Override to update the transformations of the soundsources.
        The base implementation simply copies the object transformation. */
    virtual void calculateSoundSourceTransformation(
                                        const TransformationBuffer * objectTransformation,
                                        const QList<AUDIO::SpatialSoundSource*>,
                                        uint bufferSize, SamplePos pos, uint thread);

    /** Override to fill the audio buffers of the sound sources.
        The base implementation does nothing. */
    virtual void calculateSoundSourceBuffer(const QList<AUDIO::SpatialSoundSource*>,
                                            uint bufferSize, SamplePos pos, uint thread)
    { Q_UNUSED(bufferSize); Q_UNUSED(pos); Q_UNUSED(thread); }

    /** Override to update the transformations of each microphone.
        The base implementation simply copies the object transformation. */
    virtual void calculateMicrophoneTransformation(
                                        const TransformationBuffer * objectTransformation,
                                        const QList<AUDIO::SpatialMicrophone*>,
                                        uint bufferSize, SamplePos pos, uint thread);

public:
    // --------------- 3d --------------------------

    /** Initialize transformation matrix */
    void clearTransformation(uint thread, uint sample);

    /** Returns the transformation matrix of this object */
    const Mat4& transformation(uint thread, uint sample) const
        { return transformation_[thread][sample]; }

    /** Returns a pointer to bufferSize(thread) number of matrices */
    const Mat4* transformations(uint thread) const
        { return &transformation_[thread][0]; }

    /** Returns the position of this object */
    Vec3 position(uint thread, uint sample) const
        { return Vec3(transformation_[thread][sample][3][0],
                      transformation_[thread][sample][3][1],
                      transformation_[thread][sample][3][2]); }

    void setTransformation(int thread, int sample, const Mat4& mat)
        { transformation_[thread][sample] = mat; }

    /** Apply all transformations of this object to the given matrix. */
    void calculateTransformation(Mat4& matrix, Double time, uint thread) const;

    /** List of all direct transformation childs */
    const QList<Transformation*> transformationObjects() const { return transformationObjects_; }


    // ------------------ files ----------------------

    /** Should return the list of files, this object needs, by appending to the list.
        The method is called, regardless of the activity scope of the object.
        Derived classes should return any potentially needed files.
        More is better than not enough, in this case.
        Always call the ancestor's method before your derived code. */
    virtual void getNeededFiles(IO::FileList & files) { Q_UNUSED(files); }

signals:

public slots:

    // _____________ PRIVATE AREA __________________

private:

    // disable copy
    Object(const Object&);
    void operator=(const Object&);

    /** Implementation of deserializeTree() */
    static Object * deserializeTree_(IO::DataStream&);

    /** Removes the child from the child list, nothing else. */
    bool takeChild_(Object * child);

    /** Adds the object to child list, nothing else */
    Object * addChildObjectHelper_(Object * object, int insert_index = -1);

    /** Makes all idNames in the tree unique regarding the tree of @p root.
        The tree in @p root can be an actual parent of the object or not. */
    void makeUniqueIds_(Object * root);

    /** Makes all idNames in the tree unique regarding the @p existingNames.
        @p existingNames will be modified with the changed idNames. */
    void makeUniqueIds_(QSet<QString>& existingNames);

    /** Called on changes to the child list */
    void childrenChanged_();

    /** Fills the transformationChilds() array */
    void collectTransformationObjects_();

    //void setNumberThreadsRecursive_(int threads);

    // ---------- parameter s-----------------

    Parameters * parameters_;

    void passDownActivityScope_(ActivityScope parent_scope);

    // ------------ properties ---------------

    QString idName_, name_, orgIdName_;

    bool canBeDeleted_;

    QMap<QString, QMap<qint64, QVariant>> attachedData_;

    // ----------- tree ----------------------

    Object * parentObject_;
    QList<Object*> childObjects_;
    QList<Transformation*> transformationObjects_;
    bool childrenHaveChanged_;

    // ---------- threads and blocksize ------

    uint numberThreads_;
    std::vector<uint> bufferSize_;

    // --------- default parameters ----------

    ParameterSelect * paramActiveScope_;

    // ------------ audio --------------------

    uint p_numberSoundSources_,
         p_numberMicrophones_;

    uint sampleRate_;
    Double sampleRateInv_;

    // ------------ runtime ------------------

    ActivityScope
    /** activity scope passed down from parents */
        parentActivityScope_,
    /** current requested activity scope */
        currentActivityScope_;

    // ----------- position ------------------

    std::vector<std::vector<Mat4>> transformation_;

};

/** Installs the object in ObjectFactory.
    @note Prefere to use MO_REGISTER_OBJECT to do the job */
extern bool registerObject_(Object *);





// ---------------------- template impl -------------------

template <class T>
QList<T*> Object::findChildObjects(const QString& id, bool recursive, Object * ignore) const
{
    QList<T*> list;

    for (auto o : childObjects_)
    {
        if (o != ignore
            && (qobject_cast<T*>(o))
            && (id.isEmpty() || o->idName() == id))
                list.append(static_cast<T*>(o));

        if (recursive)
            list.append(o->findChildObjects<T>(id, recursive, ignore));
    }

    return list;
}

template <class T>
QList<T*> Object::findChildObjectsStopAt(
        const QString& id, bool recursive, Object * stopAt) const
{
    QList<T*> list;

    for (auto o : childObjects_)
    {
        if (o != stopAt
            && (qobject_cast<T*>(o))
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
    if (childObjects_.empty())
        return -1;

    if (last < 0 || last >= childObjects_.size())
        last = childObjects_.size() - 1;

    for (int i = last; i>=0; --i)
    {
        if (qobject_cast<T*>(childObjects_[i]))
            return i;
    }

    return -1;
}


} // namespace MO

Q_DECLARE_METATYPE(MO::Object*)

#endif // MOSRC_OBJECT_OBJECT_H
