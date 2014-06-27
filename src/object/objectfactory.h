/** @file objectfactory.h

    @brief factory for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTFACTORY_H
#define MOSRC_OBJECT_OBJECTFACTORY_H

#include <QObject>

namespace MO {

class Object;
namespace IO { class DataStream; }

class ObjectFactory : public QObject
{
    Q_OBJECT
public:
    explicit ObjectFactory(QObject *parent = 0);

    /** Creates the desired object for className, or return NULL */
    Object * createObject(const QString& className);

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTFACTORY_H
