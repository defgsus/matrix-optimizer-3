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
    explicit Object(const QString& className, QObject *parent = 0);

    // ----------------- io ---------------------

    void serializeTree(IO::DataStream&);
    static Object * deserializeTree(IO::DataStream&);

    virtual void serialize(IO::DataStream&) { }
    virtual void deserialize(IO::DataStream&) { }

    // --------------- getter -------------------

    /** Name of the object class, for creating objects at runtime. MUST NOT CHANGE! */
    const QString& className() const { return className_; }
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

    /** Installs the object in the parent object's childlist.
        If @p insert_index is >= 0, the object will be
        inserted before the indexed object (e.g. 0 = start, 1 = before second).*/
    void setParentObject(Object * parent, int insert_index = -1);

    /** Returns a string that is unique among the childs of this object */
    QString getUniqueId(QString id);

    /** Read-access to the list of childs */
    const QList<Object*> childObjects() const { return childObjects_; }

    /** Returns the children with the given id, or NULL */
    Object * getChildObject(const QString& id, bool recursive = false);

    /** Adds the object to the list of childs.
        The object is (re-)parented to this object.
        If @p insert_index is >= 0, the object will be
        inserted before the indexed object (e.g. 0 = start, 1 = before second ..).
        @returns The added object. */
    Object * addObject(Object * object, int insert_index = -1);

signals:

public slots:

    // _____________ PRIVATE AREA __________________

private:

    /** Removes the child from the child list, nothing else. */
    void takeChild_(Object * child);

    /** Only adds the object to child list. */
    Object * addChildObject_(Object * object, int insert_index = -1);

    // ------------ properties ---------------

    QString className_, idName_, name_;

    // ----------- tree ----------------------

    Object * parentObject_;
    QList<Object*> childObjects_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECT_H
