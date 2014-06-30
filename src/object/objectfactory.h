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

namespace MO {

class Object;
class Scene;

namespace IO { class DataStream; }

class ObjectFactory : public QObject
{
    Q_OBJECT

    ObjectFactory();
public:

    static ObjectFactory& instance();

    static bool registerObject(Object *);

    /** Creates the desired object for className, or returns NULL */
    static Object * createObject(const QString& className, bool createParameters = true);
    /** Returns a new scene object, or NULL */
    static Scene * createSceneObject();

    static Object * createDummy();

signals:

public slots:

private:

    static ObjectFactory * instance_;

    std::map<QString, std::shared_ptr<Object>> objectMap_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTFACTORY_H
