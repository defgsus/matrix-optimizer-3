/** @file frontdisplayfloatitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.03.2015</p>
*/

#include <functional>

#include <QPainter>

#include "frontdisplayfloatitem.h"
#include "faderitem.h"
#include "gui/util/frontscene.h"
#include "object/object.h"
#include "object/audioobject.h"
#include "io/currenttime.h"
#include "types/properties.h"


namespace MO {
namespace GUI {

MO_REGISTER_FRONT_ITEM(FrontDisplayFloatItem);

struct FrontDisplayFloatItem::Private
{
    Private(FrontDisplayFloatItem * i)
        : item      (i)
        , fader     (0)
    { }

    void getValueFunction();
    void createWidgets();
    void updateWidgetValue();

    FrontDisplayFloatItem * item;
    QRectF labelRect;

    DisplayType displayType;
    FaderItem * fader;

    std::function<Float(Double)> valueFunc;
};

FrontDisplayFloatItem::FrontDisplayFloatItem(QGraphicsItem * parent)
    : AbstractFrontDisplayItem (parent)
    , p_                (new Private(this))
{
    initProperty("size", QSize(16, 64));
    initProperty("label-text", QString("X"));
    initProperty("padding", 2);
    initProperty("label-outside", true);
    initProperty("background-color", QColor(0x30, 0x50, 0x30));
    initProperty("value-on-color", QColor(0xff,0xff,0xff,0x60));
    initProperty("value-off-color", QColor(0,0,0,0x60));

    initProperty("display-type", 0);
    initProperty("fader-vertical", true);

    initProperty("value-min", 0.0);
    initProperty("value-max", 1.0);

    initProperty("value-label-visible", true);
    initProperty("value-label-outside", true);
    initProperty("value-label-align", Properties::Alignment(Properties::A_BOTTOM | Properties::A_HCENTER));
    initProperty("value-label-margin", 0);
}

FrontDisplayFloatItem::~FrontDisplayFloatItem()
{
    delete p_;
}

void FrontDisplayFloatItem::onPropertiesChanged()
{
    // assign object
    AbstractFrontDisplayItem::onPropertiesChanged();

    p_->getValueFunction();
    p_->createWidgets();
}

void FrontDisplayFloatItem::Private::getValueFunction()
{
    valueFunc = 0;

    if (!item->assignedObject())
        return;

    // output channel
    // XXX For audio objects, that's the actual channel number
    //     Modulator/Float outputs only have one channel currently
    int channel = item->assignedChannel();

    if (AudioObject * ao = dynamic_cast<AudioObject*>(item->assignedObject()))
    {
        valueFunc = [=](Double time)
        {
            return ao->getAudioOutputAsFloat(channel, time, MO_AUDIO_THREAD);
        };
    }
}

void FrontDisplayFloatItem::Private::createWidgets()
{
    // ----- update widgets -------

    displayType = (DisplayType)item->properties().get("display-type").toInt();

    if (displayType == DT_FADER)
    {
//        delete knob;
//        knob = 0;

        if (!fader)
        {
            fader = new FaderItem(item);
            item->onEditModeChanged();
        }

        fader->setRect(item->innerRect());
        fader->setOnColor(item->properties().get("value-on-color").value<QColor>());
        fader->setOffColor(item->properties().get("value-off-color").value<QColor>());
        fader->setVertical(item->properties().get("fader-vertical").toBool());
        fader->setRange(item->properties().get("value-min").toFloat(),
                        item->properties().get("value-max").toFloat());
    }

/*    if (controlType == CT_KNOB)
    {
        delete fader;
        fader = 0;

        if (!knob)
        {
            knob = new KnobItem(this);
            knob->setCallback([=](Float)
            {
                sendValue();
                update();
            });
            onEditModeChanged();
        }

        knob->setRect(innerRect());
        knob->setOnColor(properties().get("value-on-color").value<QColor>());
        knob->setOffColor(properties().get("value-off-color").value<QColor>());
        knob->setDefaultValue(defaultValue());
        knob->setRange(properties().get("value-min").toFloat(),
                            properties().get("value-max").toFloat());
    }
*/
}


void FrontDisplayFloatItem::onEditModeChanged()
{
    // disable mouse events for controls in edit mode
    Qt::MouseButtons mb = isEditMode() ? Qt::NoButton : Qt::AllButtons;

    if (p_->fader)
        p_->fader->setAcceptedMouseButtons(mb);
//    if (p_->knob)
//        p_->knob->setAcceptedMouseButtons(mb);
}

//void FrontDisplayFloatItem::o

QRectF FrontDisplayFloatItem::boundingRect() const
{
    return AbstractFrontItem::boundingRect()
            | p_->labelRect;
}

Float FrontDisplayFloatItem::value() const
{
    return p_->valueFunc
            ? p_->valueFunc(CurrentTime::time())
            : 0;
}

void FrontDisplayFloatItem::Private::updateWidgetValue()
{
    const Float v = item->value();

    if (fader)
        fader->setValue(v);
}

void FrontDisplayFloatItem::paint(QPainter *p, const QStyleOptionGraphicsItem * style, QWidget * widget)
{
    p_->updateWidgetValue();

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
