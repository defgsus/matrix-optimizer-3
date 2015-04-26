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
#include "gui/painter/valuecurve.h"
#include "object/object.h"
#include "object/audioobject.h"
#include "object/control/sequencefloat.h"
#include "io/currenttime.h"
#include "types/properties.h"

//#include "object/util/objecteditor.h"
//#include "object/scene.h"
//#include "io/log.h"

namespace MO {
namespace GUI {

MO_REGISTER_FRONT_ITEM(FrontDisplayFloatItem);

struct FrontDisplayFloatItem::Private
{
    Private(FrontDisplayFloatItem * i)
        : item      (i)
        , fader     (0)
        , curvePainter(0)
        , lookupTime(0.0)
        , timeRange (1.0)
    { }

    void getValueFunction();
    void createWidgets();
    void updateWidgetValue();

    FrontDisplayFloatItem * item;
    QRectF labelRect;

    DisplayType displayType;
    FaderItem * fader;
    PAINTER::ValueCurve * curvePainter;

    std::function<Float(Double)> valueFunc;
    Double lookupTime, timeRange;
};

FrontDisplayFloatItem::FrontDisplayFloatItem(QGraphicsItem * parent)
    : AbstractFrontDisplayItem (parent)
    , p_                (new Private(this))
{
    initProperty("size", QSize(16, 64));
    initProperty("label-text", QString("X"));
    initProperty("padding", 2);
    initProperty("label-outside", true);
    initProperty("background-color", QColor(0x30, 0x30, 0x30));
    initProperty("value-on-color", QColor(0xff,0x60,0xf0,0x60));
    initProperty("value-off-color", QColor(0,0,0x60,0));

    initProperty("display-type", 0);
    initProperty("fader-vertical", true);
    initProperty("scope-time-range", 1.0);

    initProperty("value-min", 0.0);
    initProperty("value-max", 1.0);
}

FrontDisplayFloatItem::~FrontDisplayFloatItem()
{
    delete p_->curvePainter;
    delete p_;
}

void FrontDisplayFloatItem::onPropertiesChanged()
{
    // assign object
    AbstractFrontDisplayItem::onPropertiesChanged();

    p_->timeRange = properties().get("scope-time-range").toDouble();

    p_->getValueFunction();
    p_->createWidgets();
}

bool FrontDisplayFloatItem::acceptObject(Object * o) const
{
    return o->type() == Object::T_AUDIO_OBJECT
        || o->type() == Object::T_SEQUENCE_FLOAT;
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

    if (SequenceFloat * seq = dynamic_cast<SequenceFloat*>(item->assignedObject()))
    {
        valueFunc = [=](Double time)
        {
            return seq->value(time, MO_GUI_THREAD);
        };
    }
}

void FrontDisplayFloatItem::Private::createWidgets()
{
    // ----- update widgets -------

    displayType = (DisplayType)item->properties().get("display-type").toInt();

    if (displayType == DT_FADER)
    {
        delete curvePainter;
        curvePainter = 0;
//        delete knob;
//        knob = 0;

        if (!fader)
        {
            fader = new FaderItem(item);
            fader->setControllable(false);
            item->onEditModeChanged();
        }

        fader->setRect(item->innerRect());
        fader->setOnColor(item->properties().get("value-on-color").value<QColor>());
        fader->setOffColor(item->properties().get("value-off-color").value<QColor>());
        fader->setVertical(item->properties().get("fader-vertical").toBool());
        fader->setRange(item->properties().get("value-min").toFloat(),
                        item->properties().get("value-max").toFloat());
    }

    if (displayType == DT_SCOPE)
    {
        delete fader;
        fader = 0;
        //        delete knob;
        //        knob = 0;

        if (!curvePainter)
        {
            curvePainter = new PAINTER::ValueCurve;
            curvePainter->setOverpaint(1);
        }

        curvePainter->setPen(QPen(item->properties().get("value-on-color").value<QColor>()));
        curvePainter->setViewSpace(UTIL::ViewSpace(
                                       0.,
                                       item->properties().get("value-min").toFloat(),
                                       1.,
                                       item->properties().get("value-max").toFloat() - item->properties().get("value-min").toFloat()
                                    ));
        curvePainter->setCurveFunction([this](Double x)
        {
            if (!valueFunc)
                return Double(0.);
            Double t = lookupTime - (1. - x) * timeRange;
            return t >= 0. ? (Double)valueFunc(t) : 0.0;
        });
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
    auto r = AbstractFrontItem::boundingRect();
    if (properties().get("value-label-visible").toBool())
        r |= p_->labelRect;
    return r;
}

Float FrontDisplayFloatItem::value() const
{
    return p_->valueFunc
            ? p_->valueFunc(CurrentTime::time())
            : 0;
}

void FrontDisplayFloatItem::Private::updateWidgetValue()
{
    if (curvePainter)
    {
        lookupTime = CurrentTime::time();
        /*if (item->frontScene())
            if (auto e = item->frontScene()->objectEditor())
                if (auto s = e->scene())
                    lookupTime = s->sceneTime();*/

        item->update();
        return;
    }

    const Float v = item->value();

    if (fader)
        fader->setValue(v);
}

void FrontDisplayFloatItem::updateValue()
{
    p_->updateWidgetValue();
    if (properties().get("value-label-visible").toBool())
        update();
}

void FrontDisplayFloatItem::paint(QPainter *p, const QStyleOptionGraphicsItem * style, QWidget * widget)
{
    //p_->updateWidgetValue();

    AbstractFrontItem::paint(p, style, widget);

    if (p_->curvePainter)
    {
        QRect r = innerRect().toRect();
        p_->curvePainter->paint(*p, r, r);
    }

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
