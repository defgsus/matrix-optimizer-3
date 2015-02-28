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

struct FrontFloatItem::Private
{
    Private(FrontFloatItem * i)
        : item      (i)
        , fader     (0)
    { }

    FrontFloatItem * item;
    FaderItem * fader;
};

FrontFloatItem::FrontFloatItem(Parameter * p, QGraphicsItem * parent)
    : AbstractFrontItem (p, parent)
    , p_                (new Private(this))
{
    initProperty("size", QSize(16, 64));
    initProperty("value-on-color", QColor(0xa0, 0xb0, 0xa0));
    initProperty("value-off-color", QColor(0x20, 0x30, 0x20));
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
