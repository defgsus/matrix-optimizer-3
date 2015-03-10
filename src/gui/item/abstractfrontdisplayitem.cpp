/** @file abstractfrontdisplayitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.03.2015</p>
*/

#include "abstractfrontdisplayitem.h"
#include "gui/util/frontpreset.h"
#include "gui/util/frontscene.h"
#include "types/properties.h"
#include "object/scene.h"
#include "object/util/objecteditor.h"

namespace MO {
namespace GUI {

AbstractFrontDisplayItem::AbstractFrontDisplayItem(QGraphicsItem * parent)
    : AbstractFrontItem (parent)
    , p_d_object_       (0)
{
    //setCanHaveChildren(true);

    initProperty("object-id", QString("/"));
    initProperty("object-output", 0);
}


void AbstractFrontDisplayItem::onPropertiesChanged()
{
    assignObject();
}

bool AbstractFrontDisplayItem::assignObject()
{
    p_d_object_ = 0;

    // get pointer to MO::Scene
    auto s = frontScene();
    if (!s)
        return false;
    auto e = s->objectEditor();
    if (!e)
        return false;

    // find object pointer
    p_d_object_ = e->scene()->findChildObject(
                properties().get("object-id").toString(),
                true);

    return p_d_object_ != 0;
}

uint AbstractFrontDisplayItem::assignedChannel() const
{
    int c = properties().get("object-output").toInt();
    return c < 0 ? 0 : c;
}

/*
void AbstractFrontDisplayItemItem::onEditModeChanged()
{
    // disable mouse events for controls in edit mode
    Qt::MouseButtons mb = editMode() ? Qt::NoButton : Qt::AllButtons;

    if (p_->fader)
        p_->fader->setAcceptedMouseButtons(mb);
}
*/

} // namespace GUI
} // namespace MO
