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
    static Object * createObject(const QString& className, bool createParameters = true);

    /** Returns a new Scene object */
    static Scene * createSceneObject();

    /** Returns a new float sequence */
    static SequenceFloat * createSequenceFloat(const QString& name = QString());

    /** Returns a new float track */
    static TrackFloat * createTrackFloat(const QString& name = QString());

    /** Creates a dummy object (for skipping unknown objects in deserializer) */
    static Object * createDummy();

    // ----------- object infos -------------------

    /** Returns an icon for the object type */
    static const QIcon& iconForObject(const Object *);

    /** Returns a list of objects, possible to add to given object @p parent */
    static QList<const Object*> possibleChildObjects(const Object * parent);

    /** Returns true of the object can have children at all. */
    static bool canHaveChildObjects(const Object * parent);

    // ------------- file io ----------------------

    /** Stores the complete Scene object to disk.
        On IO errors, an IoException will be thrown. */
    static void saveScene(const QString& filename, const Scene*);

    /** Tries to load a scene from disk and returns a the object tree.
        If the saved object was no Scene, NULL is returned.
        On other IO errors an IoException will be thrown. */
    static Scene * loadScene(const QString& filename);

signals:

public slots:

private:

    const static bool useCompression_ = true;

    static ObjectFactory * instance_;

    std::map<QString, std::shared_ptr<Object>> objectMap_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTFACTORY_H
