/** @file frontgroupitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 28.02.2015</p>
*/

#include "frontgroupitem.h"
//#include "types/properties.h"


namespace MO {
namespace GUI {

MO_REGISTER_FRONT_ITEM(FrontGroupItem);

FrontGroupItem::FrontGroupItem(QGraphicsItem * parent)
    : AbstractFrontItem (parent)
{
    setCanHaveChildren(true);

    initProperty("rounded-size", QSize(6, 6));

    initProperty("label-text", QObject::tr("group"));

    initProperty("padding", 8);
    initProperty("border", 3.);
}


FrontGroupItem::~FrontGroupItem()
{
}

/*
void FrontGroupItem::onPropertiesChanged()
{
    if (!p_->fader)
    {
        p_->fader = new FaderItem(this);
        onEditModeChanged();
    }

    p_->fader->setRect(innerRect());
}

void FrontGroupItem::onEditModeChanged()
{
    // disable mouse events for controls in edit mode
    Qt::MouseButtons mb = editMode() ? Qt::NoButton : Qt::AllButtons;

    if (p_->fader)
        p_->fader->setAcceptedMouseButtons(mb);
}
*/

} // namespace GUI
} // namespace MO
