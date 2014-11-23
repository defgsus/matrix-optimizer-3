/** @file abstractobjectitem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include "abstractobjectitem.h"
#include "object/objectfactory.h"
#include "io/error.h"

namespace MO {
namespace GUI {

// ------------------------------ Private -------------------------------------

class AbstractObjectItem::PrivateOI
{
public:

    Object * object;
};



// -------------------------- AbstractObjectItem ------------------------------


AbstractObjectItem::AbstractObjectItem(Object *object, QGraphicsItem * parent)
    : QGraphicsItem     (parent),
      p_oi_             (new PrivateOI())
{
    MO_ASSERT(object, "no object given for AbstractObjectItem");

    p_oi_->object = object;
}

AbstractObjectItem::~AbstractObjectItem()
{
    delete p_oi_;
}

Object * AbstractObjectItem::object() const
{
    return p_oi_->object;
}

const QIcon& AbstractObjectItem::icon() const
{
    return ObjectFactory::iconForObject(p_oi_->object);
}



} // namespace GUI
} // namespace MO
