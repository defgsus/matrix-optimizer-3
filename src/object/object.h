/** @file object.h

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT_H
#define MOSRC_OBJECT_OBJECT_H

#include <QObject>
#include <QList>

namespace MO {
namespace IO { class DataStream; }

class Camera;
class Microphone;
class SoundSource;
class Parameter;


class Object : public QObject
{
    Q_OBJECT

    friend class ObjectFactory;

protected:

    /** Constructs a new object.
        If @p parent is also an Object, this object will be installed in the
        parent's child list via setParentObject().
        @note Only ObjectFactory can create Objects. */
    explicit Object(QObject *parent = 0);

public:

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

    /** Name of the object class, for creating objects at runtime. MUST NOT CHANGE! */
    virtual const QString& className() const = 0;
    /** Tree-unique id of the object. */
    const QString& idName() const { return idName_; }
    /** User defined name of the object */
    const QString& name() const { return name_; }

    virtual bool isValid() const { return true; }

    virtual bool isScene() const { return false; }
    virtual bool is3d() const { return false; }
    virtual bool isGl() const { return false; }
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

    /** Deletes the child from the list of children, if found. */
    void deleteObject(Object * child);

signals:

public slots:

    // _____________ PRIVATE AREA __________________

private:

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
};



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




} // namespace MO

#endif // MOSRC_OBJECT_OBJECT_H
