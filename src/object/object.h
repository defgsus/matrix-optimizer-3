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

class Object : public QObject
{
    Q_OBJECT
public:
    /** Constructs a new object with idName() and name() set to @p idName.
        If @p parent is also an Object, this object will be installed in the
        parent's child list via setParentObject(). */
    explicit Object(const QString& idName, QObject *parent = 0);

    // ----------------- io ---------------------

    /** Serializes the whole tree including this object. */
    void serializeTree(IO::DataStream&);
    /** Creates a parent-less object containing the whole tree as
        previously created by serializeTree. */
    static Object * deserializeTree(IO::DataStream&);

    /** Override to store custom data */
    virtual void serialize(IO::DataStream&) { }
    /** Override to restore custom data */
    virtual void deserialize(IO::DataStream&) { }

    // --------------- getter -------------------

    /** Name of the object class, for creating objects at runtime. MUST NOT CHANGE! */
    virtual const char * className() const = 0;
    /** Unique id of the object within it's parent's childlist */
    const QString& idName() const { return idName_; }
    /** User defined name of the object */
    const QString& name() const { return name_; }

    // --------------- setter -------------------

    /** Set the user-name for the object */
    void setName(const QString&);


    // ------------- tree stuff -----------------

    /** Returns the parent Object, or NULL */
    Object * parentObject() const { return parentObject_; }

    /** Returns the root object of this hierarchy, which may
        be the object itself. */
    const Object * rootObject() const;
          Object * rootObject();

    /** Returns a string that is unique among the childs of this object */
    QString getUniqueId(QString id) const;

    /** Read-access to the list of childs */
    const QList<Object*> childObjects() const { return childObjects_; }

    /** Returns the children with the given id, or NULL */
    Object * getChildObject(const QString& id, bool recursive = false) const;

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

signals:

public slots:

    // _____________ PRIVATE AREA __________________

private:

    /** Removes the child from the child list, nothing else. */
    void takeChild_(Object * child);

    /** Adds the object to child list, nothing else */
    Object * addChildObject_(Object * object, int insert_index = -1);

    // ------------ properties ---------------

    QString idName_, name_;

    // ----------- tree ----------------------

    Object * parentObject_;
    QList<Object*> childObjects_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECT_H
