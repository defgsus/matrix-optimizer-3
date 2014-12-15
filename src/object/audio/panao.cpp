/** @file impulseao.cpp

    @brief a simple stereo panner

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include "panao.h"

#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "audio/tool/audiobuffer.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(PanAO)

class PanAO::Private
{
    public:

    Private(PanAO *ao) : ao(ao) {}

    void processAudio(uint size, SamplePos pos, uint thread);

    PanAO * ao;
    ParameterFloat
            * paramPan;
};

PanAO::PanAO(QObject *parent)
    : AudioObject (parent)
    , p_ (new Private(this))
{
    setName("Pan");
    setNumberAudioInputs(2);
    setNumberAudioOutputs(2);
}

PanAO::~PanAO()
{
    delete p_;
}

void PanAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aopan", 1);
}

void PanAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aopan", 1);
}

void PanAO::createParameters()
{
    AudioObject::createParameters();
    params()->beginParameterGroup("pan", tr("panner"));
    p_->paramPan = params()->createFloatParameter("pan_pan", tr("panning"),
                                                  tr("The panning position in the range [-1,1], 0 is center."),
                                                  0.0, 0.1);
    params()->endParameterGroup();
}


void PanAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();
}

void PanAO::onParameterChanged(Parameter *p)
{
    AudioObject::onParameterChanged(p);
}

void PanAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();
}

QString PanAO::getAudioInputName(uint channel) const
{
    switch(channel)
    {
    case 0: return tr("in");
    case 1: return tr("pan");
    }
    return AudioObject::getAudioInputName(channel);
}

void PanAO::processAudio(uint, SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*> &
            inputs  = audioInputs(thread),
            outputs = audioOutputs(thread);
    AUDIO::AudioBuffer
            * in    = inputs.isEmpty()  ? 0 : inputs[0],
            * inPan = inputs.size() < 2 ? 0 : inputs[1],
            * left  = outputs.isEmpty() ? 0 : outputs[0],
            * right = outputs.isEmpty() ? 0 : outputs[1];

    uint blockSize = 0;
    if(left) blockSize = left->blockSize();
    else if(right) blockSize = right->blockSize();

    if(inputs.isEmpty()) {
        for(uint i=0;i<blockSize; ++i) {

            if(left)  left->write(i, 0.0);
            if(right) right->write(i, 0.0);
        }
    } else {
        for(uint i=0;i<blockSize; ++i) {
            Double time = sampleRateInv() * (pos + i);

            Double pan = p_->paramPan->value(time,thread);
            if(inPan)
                pan += inPan->read(i);
            if(pan < -1.0) pan = -1.0;
            else if(pan > 1.0) pan = 1.0;

            if(left)  left->write(i, (1.0-(pan+1.0)/2.0)*in->read(i));
            if(right) right->write(i, (    (pan+1.0)/2.0)*in->read(i));
        }
    }

}

}
