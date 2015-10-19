/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#include <QList>

#include "oscinputobject.h"
#include "object/param/parameters.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "tool/linearizerfloat.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(OscInputObject)

struct OscInputObject::Private
{
    Private(OscInputObject * p)
        : p(p)
    {

    }

    void setParamVis();
    void setNumChan();
    void updateInterpolationMode();

    OscInputObject * p;
    ParameterSelect * p_interpol;
    ParameterInt * p_numChan;
    QList<ParameterText*> p_ids;
    LinearizerFloat linear;
};


OscInputObject::OscInputObject(QObject *parent)
    : Object    (parent)
    , p_        (new Private(this))
{
    setName("OscInput");

    p_->linear.insertValue(0, 0);
    p_->linear.insertValue(1, 1);
    p_->linear.insertValue(2, -2);
    p_->linear.insertValue(3, 3);
}

OscInputObject::~OscInputObject()
{
    delete p_;
}

void OscInputObject::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("osci", 1);
}

void OscInputObject::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("osci", 1);
}

void OscInputObject::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("osc", tr("OSC"));

        p_->p_interpol = params()->createSelectParameter(
            "interpolation", tr("interpolation"),
            tr("Selects the interpolation mode for smoothing input values"),
            { "none", "linear", "smooth", "smooth2" },
            { tr("none"), tr("linear"), tr("smooth"), tr("smooth steeper") },
            { tr("No interpolation"),
              tr("Linear interpolation"),
              tr("Smooth transition from one value to the next"),
              tr("Steep smooth transition from one value to the next") },
            { LinearizerFloat::IM_NONE, LinearizerFloat::IM_LINEAR,
              LinearizerFloat::IM_SMOOTH, LinearizerFloat::IM_SMOOTH2 },
            LinearizerFloat::IM_NONE,
            true, false);

        p_->p_numChan = params()->createIntParameter(
                    "num_channel", tr("number channels"),
                    tr("The number of different channels in this object"),
                    1, 1, 20, 1, true, false);

        for (int i=0; i<p_->p_numChan->maxValue(); ++i)
        {
            p_->p_ids << params()->createTextParameter(
                             QString("id_%1").arg(i),
                             tr("id %1").arg(i),
                             tr("The name of the osc value"),
                             TT_PLAIN_TEXT,
                             QString("value_%1").arg(i),
                             true, false);
        }

    params()->endParameterGroup();
}

void OscInputObject::onParametersLoaded()
{
    Object::onParametersLoaded();
    p_->updateInterpolationMode();
    p_->setNumChan();
}

void OscInputObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);
    if (p == p_->p_interpol)
        p_->updateInterpolationMode();
    if (p == p_->p_numChan)
        p_->setNumChan();

    if (p_->p_ids.indexOf((ParameterText*)p) >= 0)
        emitConnectionsChanged();
}

void OscInputObject::updateParameterVisibility()
{
    Object::updateParameterVisibility();
    p_->setParamVis();
}

void OscInputObject::Private::setParamVis()
{
    const int num = p_numChan->baseValue();
    for (int i=0; i<p_ids.size(); ++i)
    {
        p_ids[i]->setVisible(i < num);
    }
}

void OscInputObject::Private::setNumChan()
{
    const int num = p_numChan->baseValue();
    p->setNumberOutputs(ST_FLOAT, num);
}

QString OscInputObject::getOutputName(SignalType st, uint channel) const
{
    if (st != ST_FLOAT)
        return Object::getOutputName(st, channel);

    if ((int)channel >= p_->p_ids.size())
        return QString("id %").arg(channel);
    else
        return p_->p_ids[channel]->baseValue();
}

void OscInputObject::Private::updateInterpolationMode()
{
    linear.setInterpolationMode(
                (LinearizerFloat::InterpolationMode)p_interpol->baseValue());
}

Double OscInputObject::valueFloat(uint , Double time, uint ) const
{
    return p_->linear.getValue(time);
}


} // namespace MO
