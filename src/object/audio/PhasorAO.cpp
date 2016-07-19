/** @file phasorao.cpp

    @brief A normalized moving phase value

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include "PhasorAO.h"

#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterText.h"
#include "audio/tool/AudioBuffer.h"
#include "io/DataStream.h"

namespace MO {

MO_REGISTER_OBJECT(PhasorAO)

class PhasorAO::Private
{
    public:

    Private(PhasorAO *ao) : ao(ao) { }

    PhasorAO * ao;
    std::vector<Double> phase;
    ParameterFloat
            * paramFreq,
            * paramOffset;
};

PhasorAO::PhasorAO()
    : AudioObject   ()
    , p_          (new Private(this))
{
    setName("Phasor");
    setNumberAudioInputs(2);
    setNumberAudioOutputs(1);
}

PhasorAO::~PhasorAO()
{
    delete p_;
}

void PhasorAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aophasor", 1);
}

void PhasorAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aophasor", 1);
}

void PhasorAO::createParameters()
{
    AudioObject::createParameters();
    params()->beginParameterGroup("phasor", tr("phasor"));
    initParameterGroupExpanded("phasor");

    p_->paramFreq   = params()->createFloatParameter("phsr_freq", tr("frequency"),
                                                     tr("The frequency of the phase change"),
                                                     100.0, 1.0);
    p_->paramOffset = params()->createFloatParameter("phsr_offs", tr("offset"),
                                                     tr("The initial phase, a value in [0,1)"),
                                                     0.0, 0.05);
    params()->endParameterGroup();
}

void PhasorAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
    p_->phase.resize(num);
    for (auto & p : p_->phase)
        p = 0.;
}

QString PhasorAO::getAudioInputName(uint channel) const
{
    switch (channel)
    {
        default: return tr("*unknown*");
        case 0: return tr("freq");
        case 1: return tr("offset");
    }
}

void PhasorAO::processAudio(const RenderTime& time)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs  = audioInputs(time.thread()),
            outputs = audioOutputs(time.thread());

    AUDIO::AudioBuffer
            * inFreq = inputs.size() < 1 ? 0 : inputs[0],
            * inOffs = inputs.size() < 2 ? 0 : inputs[1],
            * out    = outputs.isEmpty() ? 0 : outputs[0];

    if(!out) return;

    Double onedsr   = sampleRateInv();
    Double phase    = p_->phase[time.thread()];
    RenderTime ti(time);
    for(uint i=0;i<out->blockSize();++i) {
        Double
                freq = p_->paramFreq->value(ti),
                offs = p_->paramOffset->value(ti);
        if(inFreq)
            freq += inFreq->read(i);
        if(inOffs)
            offs += inOffs->read(i);

        Double increment = freq * onedsr;
        out->write(i,phase);
        phase += increment;
        if(phase > 1.0) phase -= 1.0;
        else if(phase < 0.0) phase += 1.0;
        ti += SamplePos(1);
    }
    p_->phase[time.thread()] = phase;
}

}
