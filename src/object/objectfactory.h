/** @file objectfactory.h

    @brief factory for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTFACTORY_H
#define MOSRC_OBJECT_OBJECTFACTORY_H

#include <map>
#include <memory>

#include <QObject>
#include <QSize>

#include "object/object_fwd.h"

class QIcon;

namespace MO {

namespace IO { class DataStream; }

/** Singleton class to create all objects */
class ObjectFactory : public QObject
{
    Q_OBJECT
    /** Private constructor */
    ObjectFactory();
public:

    /** Access to singleton instance */
    static ObjectFactory& instance();

    // ----------- object creation ----------------

    /** Registers the object to make it available by createObject() */
    static bool registerObject(Object *);

    /** Creates the desired object for className, or returns NULL */
    static Object * createObject(const QString& className, bool createParametersAndObjects = true);

    /** Returns a new Scene object */
    static Scene * createSceneObject();

    /** Returns a new float sequence */
    static SequenceFloat * createSequenceFloat(const QString& name = QString());

    /** Returns a new float track */
    static TrackFloat * createTrackFloat(const QString& name = QString());

    /** Returns a new float modulator object*/
    static ModulatorObjectFloat * createModulatorObjectFloat(const QString& name = QString());

    /** Creates a dummy object (for skipping unknown objects in deserializer) */
    static Object * createDummy();

    // ----------- object infos -------------------

    /** Returns read access to an instantiation of the given object,
        or NULL if @p className is unknown. */
    static const Object * getObject(const QString& className);

    /** Translated name of the object group, or empty string for unknown */
    static QString objectPriorityName(int priority);

    /** Returns the type for a given classname, or -1 */
    static int typeForClass(const QString& className);

    /** Hue value for object type (Object::Type).
        Returns hue for a defined object group, or -1 if undefined! */
    static int hueForObject(int type);

    static QColor colorForObject(const Object *, bool darkSet = false);
#if 0
    /** Returns an icon for the object type */
    static const QIcon& iconForObject(const Object *);

    /** Returns an icon for the object type (Object::Type) */
    static const QIcon& iconForObject(int objectType);

    /** Returns an icon for the object with given color. */
    static QIcon iconForObject(const Object *, QColor color, const QSize& size = QSize());
#endif
    /** Returns a list of objects, possible to add to given object @p parent */
    static QList<const Object*> possibleChildObjects(const Object * parent);

    /** Returns true if the object can have children at all. */
    static bool canHaveChildObjects(const Object * parent);

    /** Returns a list of objects matching the Object::Type flags */
    static QList<const Object*> objects(int objectTypeFlags);

    /** Returns the correct index to insert @p newChild into parent */
    static int getBestInsertIndex(Object * parent, Object * newChild, int desired_index);

    /* Returns the html documentation of all parameters of the given object,
        or empty QString if @p className is not known. */
    //static QString getParameterDoc(const QString& className);

    // -------------- byte io ---------------------

    /** Stores the complete Scene object.
        On IO errors, an IoException will be thrown. */
    static void saveScene(IO::DataStream& io, const Scene*);

    /** Tries to load a scene and returns a the object tree.
        If the saved object was no Scene, NULL is returned.
        On other IO errors an IoException will be thrown. */
    static Scene * loadScene(IO::DataStream& io);

    /** Stores the complete Object and it's subtree object.
        On IO errors, an IoException will be thrown. */
    static void saveObject(IO::DataStream& io, const Object*);

    /** Tries to load an object and returns a the object tree.
        On IO errors an IoException will be thrown. */
    static Object * loadObject(IO::DataStream& io);

    // ------------- file io ----------------------

    /** Stores the complete Scene object to disk.
        On IO errors, an IoException will be thrown. */
    static void saveScene(const QString& filename, const Scene*);

    /** Tries to load a scene from disk and returns a the object tree.
        If the saved object was no Scene, NULL is returned.
        On other IO errors an IoException will be thrown. */
    static Scene * loadScene(const QString& filename);

    /** Stores the object and it's subtree to disk.
        On IO errors, an IoException will be thrown. */
    static void saveObject(const QString& filename, const Object*);

    /** Tries to load an object from disk and returns a the object tree.
        On any IO errors an IoException will be thrown. */
    static Object * loadObject(const QString& filename);

    /** Opens filedialog, saves the object,
        no exceptions. */
    static void storeObjectTemplate(Object * obj);

    /** Opens filedialog, returns an object or NULL,
        no exceptions. */
    static Object * loadObjectTemplate();

signals:

public slots:

private:

    const static bool useCompression_ = true;

    static ObjectFactory * instance_;

    std::map<QString, std::shared_ptr<Object>> objectMap_;
};


/** Creates an object of specific type, or NULL if something goes wrong (which it shouldn't) */
template <class OBJ>
OBJ * create_object(const QString& name = "")
{
    Object * o = ObjectFactory::createObject(OBJ::staticClassName());
    if (OBJ * obj = qobject_cast<OBJ*>(o))
    {
        if (!name.isEmpty())
            obj->setName(name);
        return obj;
    }
    return 0;
}



} // namespace MO

#endif // MOSRC_OBJECT_OBJECTFACTORY_H
