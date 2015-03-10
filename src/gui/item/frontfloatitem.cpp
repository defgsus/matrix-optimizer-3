/** @file frontfloatitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.02.2015</p>
*/

#include <QPainter>

#include "frontfloatitem.h"
#include "faderitem.h"
#include "knobitem.h"
#include "gui/util/frontscene.h"
#include "object/object.h"
#include "types/properties.h"


namespace MO {
namespace GUI {

MO_REGISTER_FRONT_ITEM(FrontFloatItem);

struct FrontFloatItem::Private
{
    Private(FrontFloatItem * i)
        : item      (i)
        , fader     (0)
        , knob      (0)
    { }

    FrontFloatItem * item;
    QRectF labelRect;

    ControlType controlType;
    FaderItem * fader;
    KnobItem * knob;

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

    initProperty("control-type", 0);
    initProperty("fader-vertical", true);

    initProperty("value-min", 0.0);
    initProperty("value-max", 1.0);
    initProperty("value-default", 0.0);

    initProperty("value-label-visible", true);
    initProperty("value-label-outside", true);
    initProperty("value-label-align", Properties::Alignment(Properties::A_BOTTOM | Properties::A_HCENTER));
    initProperty("value-label-margin", 0);
}

FrontFloatItem::~FrontFloatItem()
{
    delete p_;
}

int FrontFloatItem::modulatorType() const
{
    return Object::T_MODULATOR_OBJECT_FLOAT;
}

void FrontFloatItem::onPropertiesChanged()
{
    p_->controlType = (ControlType)properties().get("control-type").toInt();

    if (p_->controlType == CT_FADER)
    {
        delete p_->knob;
        p_->knob = 0;

        if (!p_->fader)
        {
            p_->fader = new FaderItem(this);
            p_->fader->setCallback([=](Float)
            {
                sendValue();
                update();
            });
            onEditModeChanged();
        }

        p_->fader->setRect(innerRect());
        p_->fader->setOnColor(properties().get("value-on-color").value<QColor>());
        p_->fader->setOffColor(properties().get("value-off-color").value<QColor>());
        p_->fader->setVertical(properties().get("fader-vertical").toBool());
        p_->fader->setDefaultValue(defaultValue());
        p_->fader->setRange(properties().get("value-min").toFloat(),
                            properties().get("value-max").toFloat());
    }

    if (p_->controlType == CT_KNOB)
    {
        delete p_->fader;
        p_->fader = 0;

        if (!p_->knob)
        {
            p_->knob = new KnobItem(this);
            p_->knob->setCallback([=](Float)
            {
                sendValue();
                update();
            });
            onEditModeChanged();
        }

        p_->knob->setRect(innerRect());
        p_->knob->setOnColor(properties().get("value-on-color").value<QColor>());
        p_->knob->setOffColor(properties().get("value-off-color").value<QColor>());
        p_->knob->setDefaultValue(defaultValue());
        p_->knob->setRange(properties().get("value-min").toFloat(),
                            properties().get("value-max").toFloat());
    }
}

Float FrontFloatItem::value() const
{
    if (p_->fader)
        return p_->fader->value();
    if (p_->knob)
        return p_->knob->value();

    return 0;
}

Float FrontFloatItem::defaultValue() const
{
    return properties().get("value-default").toFloat();
}

void FrontFloatItem::setValue(Float v)
{
    if (!p_->fader && !p_->knob)
        onPropertiesChanged();

    if (p_->fader)
        p_->fader->setValue(v);
    if (p_->knob)
        p_->knob->setValue(v);
}

bool FrontFloatItem::sendValue()
{
    auto s = frontScene();
    if (!s)
        return false;
    s->sendValue(idName(), value());
    return true;
}

void FrontFloatItem::setValueVariant(const QVariant& v)
{
    setValue(v.toFloat());
}

void FrontFloatItem::onEditModeChanged()
{
    // disable mouse events for controls in edit mode
    Qt::MouseButtons mb = isEditMode() ? Qt::NoButton : Qt::AllButtons;

    if (p_->fader)
        p_->fader->setAcceptedMouseButtons(mb);
    if (p_->knob)
        p_->knob->setAcceptedMouseButtons(mb);
}

//void FrontFloatItem::o

QRectF FrontFloatItem::boundingRect() const
{
    return AbstractFrontItem::boundingRect()
            | p_->labelRect;
}

void FrontFloatItem::paint(QPainter *p, const QStyleOptionGraphicsItem * style, QWidget * widget)
{
    AbstractFrontItem::paint(p, style, widget);

    // -- value label --
    if (properties().get("value-label-visible").toBool())
    {
        const QString text = QString::number(value());

        QFontMetrics metrics = p->fontMetrics();

        // alignment
        p_->labelRect = metrics.boundingRect(text);
        p_->labelRect = Properties::align(p_->labelRect, rect(),
                                         properties().get("value-label-align").value<Properties::Alignment>(),
                                         properties().get("value-label-margin").toInt(),
                                         properties().get("value-label-outside").toBool());
        //rec.moveTop(rec.top() + metrics.height());

        // draw
        p->setPen(QPen(borderColor()));
        p->drawText(p_->labelRect, 0, text);
    }
}

} // namespace GUI
} // namespace MO
