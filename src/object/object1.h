/** @file object1.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.10.2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT1_H
#define MOSRC_OBJECT_OBJECT1_H

#include <vector>

#include <QByteArray>
#include <QObject>
#include <QList>
#include <QSet>

#include "graph/tree.h"
#include "types/int.h"
#include "types/vector.h"
#include "object_fwd.h"
#include "io/filetypes.h"


namespace MO {
namespace IO { class DataStream; }
namespace MATH { class Timeline1D; }

class Object1;
namespace Private { void setObjectNode(Object1*, TreeNode<Object1*> *); }




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
class Object1
{
    // to set idName
    friend class ObjectFactory;
    // to set the node
    friend void Private::setObjectNode(Object1*, TreeNode<Object1*> *);
    friend struct TreeNodeTraits<Object1*>;
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
        T_AUDIO_UNIT        = 1<<13,
        T_MODULATOR_OBJECT_FLOAT = 1<<14,
        T_MICROPHONE_GROUP  = 1<<15,
        T_CLIP              = 1<<16,
        T_CLIP_CONTAINER    = 1<<17,
        T_OSCILLATOR        = 1<<18
    };
    enum TypeGroups
    {
        /** Objects that have a definite position */
        TG_REAL_OBJECT      = T_OBJECT | T_GROUP | T_MICROPHONE | T_SOUNDSOURCE
                                | T_CAMERA | T_LIGHTSOURCE | T_MICROPHONE_GROUP,

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

    /** TreeNode holding objects together */
    typedef TreeNode<Object1*> Node;

    // -------------- ctor -------------------

    /** Constructs a new object.
        @note Never construct an object yourself, it will not suffice.
        Always use ObjectFactory::createObject().
        */
    explicit Object1();

    virtual ~Object1();

    /** Creates a new instance of the class.
        In derived classes this will be defined via the MO_OBJECT_CONSTRUCTOR() macro.
        @note Never call cloneClass() yourself, it will not suffice.
        Always use ObjectFactory::createObject(). */
    virtual Object1 * cloneClass() const = 0;

    // ----------------- io ---------------------
#if (0)
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
#endif
    /** Override to store custom data.
        @note Always call the ancestor's serialize() function
        before your derived code!
        @note Always provide the serialize()/deserialize() methods for your classes,
        even if you do not have stuff to store yet and use
        IO::DataStream::writeHeader() to write your specific object version.
        Adding the serialize function later will definitely break previously saved files! */
    virtual void serialize(IO::DataStream&) const;

    /** Override to restore custom data.
        @note See notes for serialize() function. */
    virtual void deserialize(IO::DataStream&);

    // --------------- getter -------------------

    TreeNode<Object1*> * node() const;

    /** Name of the object class, for creating objects at runtime.
        MUST NOT CHANGE for compatibility with saved files! */
    virtual const QString& className() const = 0;
    /** Tree-unique id of the object. */
    const QString& idName() const;
    /** User defined name of the object */
    const QString& name() const;
    /** Override to add some additional information.
        The base class returns name() */
    virtual QString infoName() const { return name(); }
#if 0
    /** Returns the id of the object before it might have been changed through makeUniqueId() */
    const QString& originalIdName() const { return orgIdName_; }
#endif
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

    /** The base class method returns whether any of the Parameters of
        the object are modulated. */
    virtual bool isModulated() const;

    /** Returns true when this object or any of it's childs or sub-childs
        contains microphones or soundsources. */
    bool isAudioRelevant() const;

    /** Returns true when the object can be deleted by the ObjectTreeView */
    bool canBeDeleted() const;

    /** Returns a priority for each object type */
    static int objectPriority(const Object1 *);

    // ---------- activity (scope) ----------------

    /** Returns the user-set activity scope for the object */
    ActivityScope activityScope() const;

    /** Returns the currently set scope for the tree */
    ActivityScope currentActivityScope() const;

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
    bool isSaveToAdd(Object1 * newChild, QString& error) const;

    /** Returns the root object of this hierarchy, which may
        be the object itself. */
    const Object1 * rootObject() const;
          Object1 * rootObject();

    /** Returns the Scene object (also for the scene itself), or NULL */
    const Scene * sceneObject() const;
          Scene * sceneObject();

    /** Returns the parent Object, or NULL */
    Object1 * parentObject() const;
#if 0
    /** See if this object has a parent object @p o. */
    bool hasParentObject(Object1 * o) const;

    /** Returns the first parent object matching given Type mask, or NULL */
    Object1 * findParentObject(int typeFlags) const;

    /** Returns this object or the first parent that matches
        the Object::Type mask in @p typeFlags.
        If none matches, returns NULL */
    Object1 * findContainingObject(int typeFlags);
    /** Returns this object or the first parent that matches
        the Object::Type mask in @p typeFlags.
        If none matches, returns NULL */
    const Object1 * findContainingObject(int typeFlags) const;

    /** Returns true if this object or any of it's children match
        the Object::Type mask in @p typeFlags */
    bool containsTypes(int typeFlags) const;

    /** Returns true if this object or any of it's childs are
        equal to @p o */
    bool containsObject(Object1 * o) const;

    /** Returns number of direct childs or number of all sub-childs. */
    int numChildren(bool recursive = false) const;
#endif

    /** Returns true if an Object of @p type can be a children of this Object. */
    virtual bool canHaveChildren(Type type) const;
#if 0
    /** Read-access to the list of childs */
    const QList<Object1*> childObjects() const { return childObjects_; }

    /** Returns a set of all idNames */
    QSet<QString> getChildIds(bool recursive) const;

    /** Returns the children with the given id, or NULL.
        If @p ignore is not NULL, this object will be ignored by search. */
    Object1 * findChildObject(const QString& id, bool recursive = false, Object1 * ignore = 0) const;

    /** Returns a list of all objects that match one of the flags in @p typeFlags,
        where typeFlags is an or-combination of Object::Type enums. */
    QList<Object1*> findChildObjects(int typeFlags, bool recursive = false) const;

    /** Returns the children of type @p T with the given id, or NULL.
        if @p id is empty, all objects of type T will be returned.
        If @p ignore is not NULL, this object will be ignored by search
        (but it's child objects will be considered as well). */
    template <class T>
    QList<T*> findChildObjects(
            const QString& id = QString(), bool recursive = false, Object1 * ignore = 0) const;

    /** Returns the children of type @p T with the given id, or NULL.
        if @p id is empty, all objects of type T will be returned.
        @p stopAt and all it's children will be ignored by the search. */
    template <class T>
    QList<T*> findChildObjectsStopAt(
            const QString& id, bool recursive, Object1 * stopAt) const;

    /** Returns the index of the last child object of type @p T */
    template <class T>
    int indexOfLastChild(int last = -1) const;

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
#endif
protected:

    /** Called when the children list has changed */
    virtual void childrenChanged() { }

    /** Called when an object and it's subtree have been deleted.
        The objects in @p list are still existing!
        @note Call ancestor's implementation before your derived code!
        Object's base implementation removes all modulators from parameters
        that point to deleted objects. */
    virtual void onObjectsAboutToDelete(const QList<Object1 *> &list);

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
    uint numberThreads() const;

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

#if 0
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
#endif
public:
    // --------------- modulators ------------------

    virtual void collectModulators() { };

    /** Returns a list of objects that modulate this object. */
    virtual QList<Object1*> getModulatingObjects() const;

    /** Returns the list of all Parameters that are modulated.
        Each entry is a pair of the Parameter and the modulating object.
        Multiple modulations on the same Parameter have multiply entries in the list. */
    virtual QList<QPair<Parameter*, Object1*>> getModulationPairs() const;

    /** Returns a list of objects that will modulate this object
        when it gets added to the scene. */
    virtual QList<Object1*> getFutureModulatingObjects(const Scene * scene) const;

    // --------------- outputs ---------------------

    /** Call this to get createOutputs() to be called. */
    void requestCreateOutputs();

    /** Create your ModulatorObject classes here.
        Use the createOutput...() functions! */
    virtual void createOutputs() { };

protected:
#if 0
    /** Creates a new (or reuses an existing) ModulatorObjectFloat as child
        of this Object.
        The @p id will be adjusted, so don't rely on it.
        @note This function returns NULL, if another object with the same id
        is found which is not of ModulatorObjectFloat class! */
    ModulatorObjectFloat * createOutputFloat(const QString& id, const QString& name);
#endif

#if 0
    // --------------- parameter -------------------
public:

    /** Returns the list of parameters for this object */
    const QList<Parameter*>& parameters() const { return parameters_; }

    /** Returns the parameter with the given id, or NULL. */
    Parameter * findParameter(const QString& id);

    /** Override to create all parameters for your object.
        Always call the ancestor classes createParameters() in your derived function! */
    virtual void createParameters();

protected:

    /** Called when a parameter has changed it's value (from the gui).
        Be sure to call the ancestor class implementation before your derived code! */
    virtual void onParameterChanged(Parameter * p);

    /** Called after all parameters of the object have been deserialized.
        createParameters() has alreay been called here.
        Be sure to call the ancestor class implementation before your derived code!
        XXX Right now it's a bit unclear, what is possible here except from lazy requests. */
    virtual void onParametersLoaded() { }

public:
    /** Starts a new group which will contain all Parameters created afterwards.
        @p id is the PERSITANT name, to keep the gui-settings between sessions. */
    void beginParameterGroup(const QString& id, const QString& name);
    /** Ends the current Parameter group */
    void endParameterGroup();


    /** Creates the desired parameter,
        or returns an already created parameter object.
        When the Parameter was present before, all it's settings are still overwritten.
        If @p statusTip is empty, a default string will be set in the edit views. */
    ParameterFloat * createFloatParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Double defaultValue, Double minValue, Double maxValue, Double smallStep,
                bool editable = true, bool modulateable = true);

    ParameterFloat * createFloatParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Double defaultValue, bool editable = true, bool modulateable = true);

    ParameterFloat * createFloatParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Double defaultValue, Double smallStep,
                bool editable = true, bool modulateable = true);


    ParameterInt * createIntParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Int defaultValue, Int minValue, Int maxValue, Int smallStep,
                bool editable, bool modulateable);

    ParameterInt * createIntParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Int defaultValue, Int smallStep,
                bool editable, bool modulateable);

    ParameterInt * createIntParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Int defaultValue, bool editable, bool modulateable);

    ParameterSelect * createBooleanParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const QString& offStatusTip, const QString& onStatusTip,
                bool defaultValue, bool editable = true, bool modulateable = true);

    ParameterSelect * createSelectParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const QStringList& valueIds, const QStringList& valueNames,
                const QList<int>& valueList,
                int defaultValue, bool editable = true, bool modulateable = true);

    ParameterSelect * createSelectParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const QStringList& valueIds, const QStringList& valueNames,
                const QStringList& statusTips,
                const QList<int>& valueList,
                int defaultValue, bool editable = true, bool modulateable = true);

    ParameterText * createTextParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const QString& defaultValue,
                bool editable = true, bool modulateable = true);

    ParameterText * createTextParameter(
                const QString& id, const QString& name, const QString& statusTip,
                TextType textType,
                const QString& defaultValue,
                bool editable = true, bool modulateable = true);

    ParameterFilename * createFilenameParameter(
                const QString& id, const QString& name, const QString& statusTip,
                IO::FileType fileType,
                const QString& defaultValue = QString(), bool editable = true);


    /** Creates a timeline parameter.
        Ownership of @p defaultValue stays with caller. */
    ParameterTimeline1D * createTimeline1DParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const MATH::Timeline1D * defaultValue = 0, bool editable = true);

    ParameterTimeline1D * createTimeline1DParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const MATH::Timeline1D * defaultValue,
                Double minTime, Double maxTime, bool editable = true);

    ParameterTimeline1D * createTimeline1DParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const MATH::Timeline1D * defaultValue,
                Double minTime, Double maxTime, Double minValue, Double maxValue,
                bool editable = true);
#endif

#if 0 // should go into own audio tree?
    // ------------------- audio ------------------
public:
    /** Returns the set audio block size for each thread. */
    uint bufferSize(uint thread) const { return bufferSize_[thread]; }

    /** Returns the set sample rate in samples per second. */
    uint sampleRate() const { return sampleRate_; }

    /** Returns the reciprocal of the set sample rate, e.g. 1.0 / sampleRate() */
    Double sampleRateInv() const { return sampleRateInv_; }

protected:

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

    /** Override to create all audio sources for your object.
        @note Be sure to call the ancestor class implementation before your derived code! */
    virtual void createAudioSources() { };

    /** Override to create all microphones for your object.
        @note Be sure to call the ancestor class implementation before your derived code! */
    virtual void createMicrophones() { };

public:
    /** Returns the audio sources of this object. */
    const QList<AUDIO::AudioSource*>& audioSources() const { return objAudioSources_; }

    /** Returns the microphones of this object. */
    const QList<AUDIO::AudioMicrophone*>& microphones() const { return objMicrophones_; }

protected:

    /** Call this if you want to have createAudioSources() be called again. */
    void requestCreateAudioSources();

    /** Call this if you want to have createMicrophones() be called again. */
    void requestCreateMicrophones();

    /** Creates and returns a new audio source installed to this object.
        The id is not really important, only for display purposes,
        except when using createOrDeleteAudiosources(). */
    AUDIO::AudioSource * createAudioSource(const QString& id = QString("audio"));

    /** Creates @p number audiosources, destroys all others.
        The id is appended with a digit and the audiosources are created and deleted as needed. */
    QList<AUDIO::AudioSource*> createOrDeleteAudioSources(const QString& id, uint number);

    /** Creates and returns a new microphone installed to this object.
        The id is not really important, only for display purposes,
        except when using createOrDeleteMicrophones(). */
    AUDIO::AudioMicrophone* createMicrophone(const QString& id = QString("micro"));

    /** Creates @p number microphones, destroys all others.
        The id is appended with a digit and the microphones are created and deleted as needed. */
    QList<AUDIO::AudioMicrophone*> createOrDeleteMicrophones(const QString& id, uint number);

    /** Override to update the transformations of the AudioSource and Microphone objects
        in the gfx thread.
        The object's transformation is calculated before the call of this function.
        @note Be sure to call the ancestor's method before in your derived method. */
    virtual void updateAudioTransformations(Double time, uint thread)
        { Q_UNUSED(time); Q_UNUSED(thread); };

    /** Override to update the transformations of the AudioSource and Microphone objects
        in the audio thread.
        The object's transformation is calculated for the whole blocksize
        before the call of this function.
        @note Be sure to call the ancestor's method before in your derived method. */
    virtual void updateAudioTransformations(Double time, uint blockSize, uint thread)
        { Q_UNUSED(time); Q_UNUSED(blockSize); Q_UNUSED(thread); };

    /** Perform all necessary audio calculations and fill the AUDIO::AudioSource class(es).
        The block is given by bufferSize(thread).
        The object transformation is calculated for the whole buffer size
        before the call of this function.
        */
    virtual void performAudioBlock(SamplePos pos, uint thread) { Q_UNUSED(pos); Q_UNUSED(thread); };
#endif


#if 0 // should go in own transformation tree ???
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
#endif

    // _____________ PRIVATE AREA __________________

private:

    // disable copy
    Object1(const Object1&);
    void operator=(const Object1&);

    void p_set_node_(Node *);
    void p_set_id_(const QString&);

#if 0
    /** Implementation of deserializeTree() */
    static Object1 * deserializeTree_(IO::DataStream&);

    /** Removes the child from the child list, nothing else. */
    bool takeChild_(Object1 * child);

    /** Adds the object to child list, nothing else */
    Object1 * addChildObjectHelper_(Object1 * object, int insert_index = -1);

    /** Makes all idNames in the tree unique regarding the tree of @p root.
        The tree in @p root can be an actual parent of the object or not. */
    void makeUniqueIds_(Object1 * root);

    /** Makes all idNames in the tree unique regarding the @p existingNames.
        @p existingNames will be modified with the changed idNames. */
    void makeUniqueIds_(QSet<QString>& existingNames);

    /** Called on changes to the child list */
    void childrenChanged_();

    /** Fills the transformationChilds() array */
    void collectTransformationObjects_();

    //void setNumberThreadsRecursive_(int threads);

    // ---------- parameter s-----------------

    /** Writes all parameters to the stream */
    static void serializeParameters_(IO::DataStream&, const Object1 *);
    /** Reads all parameters from stream.
        @note The parameters MUST be created before! */
    static void deserializeParameters_(IO::DataStream&, Object1*);

    void passDownActivityScope_(ActivityScope parent_scope);
#endif

    class PrivateObject;
    PrivateObject * p_o_;


    // ----------- tree ----------------------

#if 0
    Object1 * parentObject_;
    QList<Object1*> childObjects_;
    QList<Transformation*> transformationObjects_;
    bool childrenHaveChanged_;
#endif
    // ------------ audio --------------------
#if 0
    QList<AUDIO::AudioSource*> objAudioSources_;
    QList<AUDIO::AudioMicrophone*> objMicrophones_;

    uint sampleRate_;
    Double sampleRateInv_;
#endif
    // ------------ runtime ------------------


    // ----------- position ------------------
#if 0
    std::vector<std::vector<Mat4>> transformation_;
#endif
};

/** Installs the Object1 in ObjectFactory.
    @note Prefere to use MO_REGISTER_Object to do the job */
extern bool registerObject1_(Object1 *);



} // namespace MO

Q_DECLARE_METATYPE(MO::Object1*)

#endif // MOSRC_OBJECT_OBJECT1_H
