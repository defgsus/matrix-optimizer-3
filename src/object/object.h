/** @file object.h

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT_H
#define MOSRC_OBJECT_OBJECT_H

#include <vector>

#include <QObject>

namespace MO {

class Object : public QObject
{
    Q_OBJECT
public:
    explicit Object(const QString& className, QObject *parent = 0);

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

    /** Installs the object in the parent object's childlist */
    void setParentObject(Object * parent);

    /** Return a string that is unique among the childs of this object */
    QString getUniqueId(QString id);

    /** Returns the children with the given id, or NULL */
    Object * getChild(const QString& id, bool recursive = false);

signals:

public slots:

    // _____________ PRIVATE AREA __________________

private:

    /** Removes the child from the child list, nothing else. */
    void takeChild_(Object * child);

    // ------------ properties ---------------

    QString className_, idName_, name_;

    // ----------- tree ----------------------

    Object * parentObject_;
    std::vector<Object*> childObjects_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECT_H
