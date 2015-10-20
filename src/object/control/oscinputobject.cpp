/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#include <QVector>
#include <QDebug>
#include <QByteArray>

#include "oscinputobject.h"
#include "object/param/parameters.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "tool/linearizerfloat.h"
#include "network/oscinput.h"
#include "network/oscinputs.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(OscInputObject)

struct OscInputObject::Private
{
    Private(OscInputObject * p)
        : p     (p)
        //, udp   (new UdpConnection)
        , osc   (0)
    {
        for (int i=0; i<10; ++i)
            linear << new LinearizerFloat;

        //connect(udp, &UdpConnection::dataReady, [=]() { readData(); });
    }

    ~Private()
    {
        for (auto l : linear)
            delete l;
        if (osc)
            osc->releaseRef();
    }

    void setParamVis();
    void setNumChan();
    void setPort();
    void readData();
    void updateInterpolationMode();

    OscInputObject * p;
    ParameterSelect * p_interpol;
    ParameterInt * p_numChan, * p_port;
    QVector<ParameterText*> p_ids;
    QVector<LinearizerFloat*> linear;

    OscInput * osc;
};


OscInputObject::OscInputObject(QObject *parent)
    : Object    (parent)
    , p_        (new Private(this))
{
    setName("OscInput");

    p_->linear[0]->insertValue(0, 0);
    p_->linear[0]->insertValue(1, 1);
    p_->linear[0]->insertValue(2, -2);
    p_->linear[0]->insertValue(3, 3);
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

        p_->p_port = params()->createIntParameter(
                    "port", tr("udp port"),
                    tr("The port number"),
                    8000, 1, 65535, 1, true, false);

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
                    1, 1, p_->linear.size(), 1, true, false);

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
    p_->setPort();
}

void OscInputObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);
    if (p == p_->p_interpol)
        p_->updateInterpolationMode();
    if (p == p_->p_numChan)
        p_->setNumChan();
    if (p == p_->p_port)
        p_->setPort();

    if (p_->p_ids.indexOf((ParameterText*)p) >= 0)
        emitConnectionsChanged();
}

void OscInputObject::updateParameterVisibility()
{
    Object::updateParameterVisibility();
    p_->setParamVis();
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

void OscInputObject::Private::setPort()
{
    if (osc)
        OscInputs::releaseListener(osc->port());
    const int num = p_port->baseValue();
    osc = OscInputs::getListener(num);
}

void OscInputObject::Private::updateInterpolationMode()
{
    for (auto l : linear)
        l->setInterpolationMode(
            (LinearizerFloat::InterpolationMode)p_interpol->baseValue());
}

void OscInputObject::Private::readData()
{

}

Double OscInputObject::valueFloat(uint chan, Double time, uint ) const
{
    return chan < (uint)p_->linear.size()
            ? p_->linear[chan]->getValue(time)
            : 0.;
}


} // namespace MO
