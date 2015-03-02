/** @file frontfloatitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.02.2015</p>
*/

#include "frontfloatitem.h"
#include "faderitem.h"
#include "types/properties.h"


namespace MO {
namespace GUI {

MO_REGISTER_FRONT_ITEM(FrontFloatItem);

struct FrontFloatItem::Private
{
    Private(FrontFloatItem * i)
        : item      (i)
        , fader     (0)
    { }

    FrontFloatItem * item;
    FaderItem * fader;
};

FrontFloatItem::FrontFloatItem(QGraphicsItem * parent)
    : AbstractFrontItem (parent)
    , p_                (new Private(this))
{
    initProperty("size", QSize(16, 64));
    initProperty("label-text", QString("X"));
    initProperty("padding", 2);
    initProperty("label-outside", true);
    initProperty("background-color", QColor(0x30, 0x50, 0x30));
    initProperty("value-on-color", QColor(0xff,0xff,0xff,0x60));
    initProperty("value-off-color", QColor(0,0,0,0x60));
    initProperty("fader-vertical", true);
}

FrontFloatItem::~FrontFloatItem()
{
    delete p_;
}


void FrontFloatItem::onPropertiesChanged()
{
    if (!p_->fader)
    {
        p_->fader = new FaderItem(this);
        onEditModeChanged();
    }

    p_->fader->setRect(innerRect());
    p_->fader->setOnColor(properties().get("value-on-color").value<QColor>());
    p_->fader->setOffColor(properties().get("value-off-color").value<QColor>());
    p_->fader->setVertical(properties().get("fader-vertical").toBool());
}

void FrontFloatItem::onEditModeChanged()
{
    // disable mouse events for controls in edit mode
    Qt::MouseButtons mb = editMode() ? Qt::NoButton : Qt::AllButtons;

    if (p_->fader)
        p_->fader->setAcceptedMouseButtons(mb);
}

} // namespace GUI
} // namespace MO
