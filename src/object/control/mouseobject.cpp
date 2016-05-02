/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/26/2015</p>
*/

#include "mouseobject.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "io/mousestate.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(MouseObject)

struct MouseObject::Private
{
    Private(MouseObject * p)
        : p     (p)
    {
    }

    MouseObject * p;
    ParameterSelect
            * p_range, * p_dragPos;
    ParameterFloat
            * p_onValue, * p_offValue;
};


MouseObject::MouseObject()
    : Object    ()
    , p_        (new Private(this))
{
    setName("Mouse");
    setNumberOutputs(ST_FLOAT, 5);
}

MouseObject::~MouseObject()
{
    delete p_;
}

void MouseObject::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("mouse", 1);
}

void MouseObject::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("mouse", 1);
}

void MouseObject::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("mouse", tr("Mouse input"));
    initParameterGroupExpanded("mouse");

        p_->p_range = params()->createSelectParameter(
                "range", tr("position range"),
                tr("Selects the range of the mouse coordinate output"),
                { "pixel", "norm", "norm_signed" },
                { tr("pixels"), tr("normalized [0,1]"), tr("normalized signed [-1,1]") },
                { tr("The coordinates are output as true pixel value"),
                  tr("The coordiantes are output in the range [0,1]"),
                  tr("The coordinates are output in the range [-1,1]")},
                { 0, 1, 2},
                2, true, false);

        p_->p_dragPos = params()->createSelectParameter(
                "pos_update", tr("position update"),
                tr("Selects when the mouse position is updated"),
                { "always", "on_drag" },
                { tr("always"), tr("when dragging") },
                { tr("The coordinates are always updated"),
                  tr("The coordiante are only updated when a mouse-key is pressed") },
                { 0, 1 },
                1, true, false);

        p_->p_onValue = params()->createFloatParameter(
                    "on_value", tr("on value"),
                    tr("The value that is output for a pressed key"),
                    1., true, true);
        p_->p_offValue = params()->createFloatParameter(
                    "off_value", tr("off value"),
                    tr("The value that is output for an unpressed key"),
                    0., true, true);

    params()->endParameterGroup();
}

void MouseObject::onParametersLoaded()
{
    Object::onParametersLoaded();
}

void MouseObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);
}

void MouseObject::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}

QString MouseObject::getOutputName(SignalType st, uint channel) const
{
    if (st != ST_FLOAT)
        return Object::getOutputName(st, channel);

    switch (channel)
    {
        case 0: return "x";
        case 1: return "y";
        case 2: return "left";
        case 3: return "mid";
        case 4: return "right";
        case 5: return "wheel-up";
        case 6: return "wheel-down";
        default: return "-";
    }
}


Double MouseObject::valueFloat(uint chan, const RenderTime& time) const
{
    switch (chan)
    {
        case 0: return p_->p_dragPos->baseValue() == 0
                    ? (p_->p_range->baseValue() == 0
                       ? MouseState::globalInstance().pos().x()
                       : p_->p_range->baseValue() == 1
                         ? MouseState::globalInstance().posNorm().x()
                         : MouseState::globalInstance().posNormSigned().x())
                    : (p_->p_range->baseValue() == 0
                       ? MouseState::globalInstance().dragPos().x()
                       : p_->p_range->baseValue() == 1
                         ? MouseState::globalInstance().dragPosNorm().x()
                         : MouseState::globalInstance().dragPosNormSigned().x());

        case 1: return p_->p_dragPos->baseValue() == 0
                    ? (p_->p_range->baseValue() == 0
                       ? MouseState::globalInstance().pos().y()
                       : p_->p_range->baseValue() == 1
                         ? MouseState::globalInstance().posNorm().y()
                         : MouseState::globalInstance().posNormSigned().y())
                    : (p_->p_range->baseValue() == 0
                       ? MouseState::globalInstance().dragPos().y()
                       : p_->p_range->baseValue() == 1
                         ? MouseState::globalInstance().dragPosNorm().y()
                         : MouseState::globalInstance().dragPosNormSigned().y());

        case 2: return MouseState::globalInstance().isDown(Qt::LeftButton)
                                        ? p_->p_onValue->value(time)
                                        : p_->p_offValue->value(time);
        case 3: return MouseState::globalInstance().isDown(Qt::MidButton)
                                        ? p_->p_onValue->value(time)
                                        : p_->p_offValue->value(time);
        case 4: return MouseState::globalInstance().isDown(Qt::RightButton)
                                        ? p_->p_onValue->value(time)
                                        : p_->p_offValue->value(time);
        /*case 5: return MouseState::globalInstance().isDown(Qt::UpArrow)
                                        ? p_->p_onValue->value(time)
                                        : p_->p_offValue->value(time);
        case 6: return MouseState::globalInstance().isDown(Qt::DownArrow)
                                        ? p_->p_onValue->value(time)
                                        : p_->p_offValue->value(time);
        */
        default: return 0.;
    }
}


} // namespace MO
