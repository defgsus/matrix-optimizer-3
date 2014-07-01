/** @file object.h

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT_H
#define MOSRC_OBJECT_OBJECT_H

#include <vector>

#include <QObject>
#include <QList>

#include "types/vector.h"

namespace MO {
namespace IO { class DataStream; }

class Scene;
class Camera;
class Microphone;
class SoundSource;
class Parameter;
class ParameterFloat;
class ObjectGl;
class Model3d;
class Transformation;
class Sequences;
class Sequence;
class SequenceFloat;

// PERSISTENT class names
#ifndef MO_OBJECTCLASSNAMES_DEFINED
    #define MO_OBJECTCLASSNAMES_DEFINED
    #define MO_OBJECTCLASSNAME_DUMMY            "_dummy"
    #define MO_OBJECTCLASSNAME_SCENE            "_scene"
    #define MO_OBJECTCLASSNAME_CAMERA           "_cam"
    #define MO_OBJECTCLASSNAME_MICROPHONE       "_mic"
    #define MO_OBJECTCLASSNAME_SOUNDSOURCE      "_sound"
    #define MO_OBJECTCLASSNAME_MODEL3D          "_model"
    #define MO_OBJECTCLASSNAME_PARAMETER_FLOAT  "_parf"
    #define MO_OBJECTCLASSNAME_AXISROTATION     "_arot"
    #define MO_OBJECTCLASSNAME_TRANSLATION      "_pos"
    #define MO_OBJECTCLASSNAME_SEQUENCEGROUP    "_seqgroup"
    #define MO_OBJECTCLASSNAME_SEQUENCE_FLOAT   "_seqf"
#endif

#define MO_REGISTER_OBJECT(class__) \
    namespace { \
        static bool success_register_object_##class__ = \
            ::MO::registerObject_(new class__); \
    }

#define MO_OBJECT_CLONE(class__) \
    virtual class__ * cloneClass() const { return new class__(); }

/** Abstract base of all Objects in MO.

*/
class Object : public QObject
{
    Q_OBJECT

    // to set idName_
    friend class ObjectFactory;

public:

    // -------------- types ------------------

    enum Type
    {
        T_NONE              = 0,
        T_OBJECT            = 1,
        T_PARAMETER_FLOAT   = 1<<2,
        T_TRANSFORMATION    = 1<<3,
        T_SCENE             = 1<<4,
        T_MICROPHONE        = 1<<5,
        T_CAMERA            = 1<<6,
        T_SOUNDSOURCE       = 1<<7,
        T_SEQUENCEGROUP     = 1<<8,
        T_SEQUENCE_FLOAT    = 1<<9,
    };
    enum TypeGroups
    {
        TG_PARAMETER        = T_PARAMETER_FLOAT,
        TG_REAL_OBJECT      = T_OBJECT | T_MICROPHONE | T_SOUNDSOURCE | T_CAMERA,
        TG_SEQUENCE         = T_SEQUENCE_FLOAT
    };


    // -------------- ctor -------------------

    /** Constructs a new object.
        If @p parent is also an Object, this object will be installed in the
        parent's child list via setParentObject() or addObject().
        @note The @p parent parameter follows more QObject's style and is not really
        used here.
        @note More important: Never construct an object yourself, it will not suffice.
        Always use ObjectFactory::createObject().
        */
    explicit Object(QObject *parent = 0);

    /** Creates a new instance of the class.
        In derived classes this can be defined via the MO_OBJECT_CLONE() macro.
        @note Never call cloneClass() yourself, it will not suffice.
        Always use ObjectFactory::createObject(). */
    virtual Object * cloneClass() const = 0;

    // ----------------- io ---------------------

    /** Serializes the whole tree including this object. */
    void serializeTree(IO::DataStream&) const;

    /** Creates a parent-less object containing the whole tree as
        previously created by serializeTree.
        On data or io errors, an IO::IoException will be thrown.
        Unknown objects will be replaced by the Dummy object.
        The constructed object can be added to a parent afterwards. */
    static Object * deserializeTree(IO::DataStream&);

    /** Override to store custom data */
    virtual void serialize(IO::DataStream&) const { }
    /** Override to restore custom data */
    virtual void deserialize(IO::DataStream&) { }

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

    /** Return the path up to this object */
    QString namePath() const;

    virtual bool isValid() const { return true; }

    virtual Type type() const { return T_NONE; }
    virtual bool isScene() const { return false; }
    virtual bool isGl() const { return false; }
    virtual bool isTransformation() const { return false; }
    virtual bool isSoundSource() const { return false; }
    virtual bool isMicrophone() const { return false; }
    virtual bool isCamera() const { return false; }
    virtual bool isParameter() const { return false; }
    virtual bool isSequence() const { return false; }

    // --------------- setter -------------------

    /** Set the user-name for the object */
    void setName(const QString&);


    // ------------- tree stuff -----------------

    /** Returns the parent Object, or NULL */
    Object * parentObject() const { return parentObject_; }

    /** See if this object has a parent object @p o. */
    bool hasParentObject(Object * o) const;

    /** Returns the root object of this hierarchy, which may
        be the object itself. */
    const Object * rootObject() const;
          Object * rootObject();

    /** Returns the Scene object, or NULL */
    const Scene * sceneObject() const;
          Scene * sceneObject();

    /** Returns a string that is unique among the whole tree hierarchy.
        If @p ignore is not NULL, this object will be ignored for comparsion. */
    QString getUniqueId(QString id, Object * ignore = 0) const;

    /** Returns number of direct childs or number of all sub-childs. */
    int numChildren(bool recursive = false) const;

    /** Read-access to the list of childs */
    const QList<Object*> childObjects() const { return childObjects_; }

    /** Returns the children with the given id, or NULL.
        If @p ignore is not NULL, this object will be ignored by search. */
    Object * findChildObject(const QString& id, bool recursive = false, Object * ignore = 0) const;

    /** Returns a list of all objects that match one of the flags in @p typeFlags,
        where typeFlags is an or-combination of Object::Type enums. */
    QList<Object*> findChildObjects(int typeFlags, bool recursive = false) const;

    /** Returns the children of type @p T with the given id, or NULL.
        if @p id is empty, all objects of type T will be returned.
        If @p ignore is not NULL, this object will be ignored by search. */
    template <class T>
    QList<T*> findChildObjects(const QString& id = QString(), bool recursive = false, Object * ignore = 0) const;

    /** Returns the index of the last child object of type @p T */
    template <class T>
    int indexOfLastChild(int last = -1) const;

    /** Installs the object in the parent object's childlist.
        If @p insert_index is >= 0, the object will be
        inserted before the indexed object (e.g. 0 = start, 1 = before second).
        This call is equivalent to calling parent->addObject(this).
        The object will be removed from the previous parent's child list.
        The idName() will be adjusted if needed. */
    void setParentObject(Object * parent, int insert_index = -1);

    /** Adds the object to the list of childs.
        The object is (re-)parented to this object.
        If @p insert_index is >= 0, the object will be
        inserted before the indexed object (e.g. 0 = start, 1 = before second ..).
        The object will be removed from the previous parent's child list.
        The idName() will be adjusted if needed.
        @returns The added object. */
    Object * addObject(Object * object, int insert_index = -1);

    /** Returns the correct index to insert as specific object type. */
    int getInsertIndex(Object * object, int insert_index = -1) const;

    /** Returns true if an Object of @p type can be a children of this Object. */
    virtual bool canHaveChildren(Type type) const;

    /** Deletes the child from the list of children, if found. */
    void deleteObject(Object * child);

    /** Called when the children list has changed */
    virtual void childrenChanged() { }

    /** Sets the number of threads that will run on this object and
        the whole tree. Any mutable values of the object must be present
        @p num times!
        Always call the ancestor implementation in your derived function! */
    virtual void setNumberThreads(int num);

    /** Returns the number of threads, this object is assigned to */
    int numberThreads() const { return transformation_.size(); }

    // --------------- modulators ------------------

    virtual void collectModulators() { };

    // --------------- parameter -------------------

    /** Override to create all parameters for your object */
    virtual void createParameters() { }

    /** Creates the desired parameter,
        or returns an already created parameter object. */
    ParameterFloat * createFloatParameter(
                const QString& id, const QString& name, Double defaultValue);

    // --------------- 3d --------------------------

    /** Initialize transformation matrix */
    void clearTransformation(int thread);

    /** Returns the transformation matrix of this object */
    const Mat4& transformation(int thread) const { return transformation_[thread]; }

    /** Returns the position of this object */
    Vec3 position(int thread) const
        { return Vec3(transformation_[thread][3][0], transformation_[thread][3][1],
                    transformation_[thread][3][2]); }

    void setTransformation(int thread, const Mat4& mat) { transformation_[thread] = mat; }

    /** Apply all transformation of this object to the given matrix. */
    void calculateTransformation(Mat4& matrix, Double time) const;

    /** List of all direct transformation childs */
    const QList<Transformation*> transformationObjects() const { return transformationObjects_; }

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
    Object * addChildObject_(Object * object, int insert_index = -1);

    /** Makes all id's in the tree unique regarding the tree of @p root.
        The tree in @p root can be an actual parent of the object or not. */
    void makeUniqueIds_(Object * root);

    /** Called on changes to the child list */
    void childrenChanged_();

    /** Fills the transformationChilds() array */
    void collectTransformationObjects_();

    //void setNumberThreadsRecursive_(int threads);

    // ------------ properties ---------------

    QString idName_, name_;

    // ----------- tree ----------------------

    Object * parentObject_;
    QList<Object*> childObjects_;
    QList<Transformation*> transformationObjects_;

    // ----------- position ------------------

    std::vector<Mat4> transformation_;
};

extern bool registerObject_(Object *);

// ---------------------- template impl -------------------

template <class T>
QList<T*> Object::findChildObjects(const QString& id, bool recursive, Object * ignore) const
{
    QList<T*> list;

    for (auto o : childObjects_)
        if (o != ignore
            && (qobject_cast<T*>(o))
            && (id.isEmpty() || o->idName() == id))
                list.append(static_cast<T*>(o));

    if (recursive)
        for (auto i : childObjects_)
            list.append(i->findChildObjects<T>(id, recursive, ignore));

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

#endif // MOSRC_OBJECT_OBJECT_H
