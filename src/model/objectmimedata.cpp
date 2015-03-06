/** @file objectmimedata.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.12.2014</p>
*/

#include <QDataStream>

#include "objectmimedata.h"

#include "object/object.h"
#include "io/datastream.h"
#include "io/application.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

const QString ObjectMimeData::mimeTypeString = "application/matrixoptimizer.3.object-description";


ObjectDescription::ObjectDescription(Object * o)
    : p_type_   (o == 0 ? Object::T_NONE : o->type()),
      p_class_  (o == 0 ? "" : o->className()),
      p_id_     (o == 0 ? "" : o->idName()),
      p_name_   (o == 0 ? "" : o->name()),
      p_ptr_    (o),
      p_app_    (application())
{ }

QByteArray ObjectDescription::toByteArray() const
{
    QByteArray a;
    QDataStream s(&a, QIODevice::WriteOnly);

    s << p_type_ << p_class_ << p_id_ << p_name_ << quint64(p_ptr_) << quint64(p_app_);

    return a;
}

ObjectDescription ObjectDescription::fromByteArray(const QByteArray & a)
{
    ObjectDescription d(0);
    QDataStream s(a);

    quint64 ptr, app;
    s >> d.p_type_ >> d.p_class_ >> d.p_id_ >> d.p_name_ >> ptr >> app;

    d.p_ptr_ = (void*)ptr;
    d.p_app_ = (void*)app;

    return d;
}

bool ObjectDescription::isFromSameApplicationInstance() const
{
    return p_app_ == application;
}



void ObjectMimeData::setObject(Object * o)
{
    ObjectDescription desc(o);
    setData(mimeTypeString, desc.toByteArray());
}

ObjectDescription ObjectMimeData::getDescription() const
{
    QByteArray a = data(mimeTypeString);
    if (a.size() > 10)
        return ObjectDescription::fromByteArray(a);
    else
        return ObjectDescription(0);
}


} // namespace MO
