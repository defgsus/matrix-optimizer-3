/** @file object.h

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT_H
#define MOSRC_OBJECT_OBJECT_H

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
class ObjectGl;
class Model3d;
class Transformation;

// PERSISTENT class names
#ifndef MO_OBJECTCLASSNAMES_DEFINED
    #define MO_OBJECTCLASSNAMES_DEFINED
    #define MO_OBJECTCLASSNAME_DUMMY "_dummy"
    #define MO_OBJECTCLASSNAME_SCENE "_scene"
    #define MO_OBJECTCLASSNAME_CAMERA "_camera"
    #define MO_OBJECTCLASSNAME_MICROPHONE "_microphone"
    #define MO_OBJECTCLASSNAME_SOUNDSOURCE "_soundsource"
    #define MO_OBJECTCLASSNAME_MODEL3D "_model3d"
    #define MO_OBJECTCLASSNAME_PARAMETER "_parameter"
    #define MO_OBJECTCLASSNAME_TRANSFORMATION "_transformation"
#endif

#define MO_REGISTER_OBJECT(class__) \
    namespace { \
        bool success_register_object_##class__ = \
            ::MO::registerObject_(new class__); \
    }

#define MO_OBJECT_CLONE(class__) \
    virtual class__ * cloneClass() const { return new class__(); }


class Object : public QObject
{
    Q_OBJECT

    // to set idName_
    friend class ObjectFactory;

public:

    /** Constructs a new object.
        If @p parent is also an Object, this object will be installed in the
        parent's child list via setParentObject() or addObject() */
    explicit Object(QObject *parent = 0);

    /** Creates a new instance of the class.
        In derived classes this can be defined via the MO_OBJECT_CLONE() macro. */
    virtual Object * cloneClass() const = 0;

    // ----------------- io ---------------------

    /** Serializes the whole tree including this object. */
    void serializeTree(IO::DataStream&) const;

    /** Creates a parent-less object containing the whole tree as
        previously created by serializeTree.
        On data or io errors, an IO::IoException will be thrown.
        Unknown objects will be replaced by the Dummy object. */
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

    virtual bool isValid() const { return true; }

    virtual bool isScene() const { return false; }
    virtual bool isGl() const { return false; }
    virtual bool isTransformation() const { return false; }
    virtual bool isSoundSource() const { return false; }
    virtual bool isMicrophone() const { return false; }
    virtual bool isCamera() const { return false; }
    virtual bool isParameter() const { return false; }

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

    /** Deletes the child from the list of children, if found. */
    void deleteObject(Object * child);


    // --------------- 3d --------------------------

    /** Returns the transformation matrix of this object */
    const Mat4& transformation() const { return transformation_; }

    /** Returns the position of this object */
    Vec3 position() const
        { return Vec3(transformation_[3][0], transformation_[3][1], transformation_[3][2]); }



signals:

public slots:

    // _____________ PRIVATE AREA __________________

private:

    // disable copy
    Object(const Object&);
    void operator=(const Object&);


    /** Removes the child from the child list, nothing else. */
    bool takeChild_(Object * child);

    /** Adds the object to child list, nothing else */
    Object * addChildObject_(Object * object, int insert_index = -1);

    /** Makes all id's in the tree unique regarding the tree of @p root.
        The tree in @p root can be an actual parent of the object or not. */
    void makeUniqueIds_(Object * root);

    // ------------ properties ---------------

    QString idName_, name_;

    // ----------- tree ----------------------

    Object * parentObject_;
    QList<Object*> childObjects_;

    // ----------- position ------------------

    Mat4 transformation_;
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
