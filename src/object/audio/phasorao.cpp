/** @file phasorao.cpp

    @brief A normalized moving phase value

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include "phasorao.h"

#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "audio/tool/audiobuffer.h"
#include "io/datastream.h"

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

PhasorAO::PhasorAO(QObject *parent)
    : AudioObject (parent)
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
    for(int i=0;i<num;++i) {
        p_->phase[i]     = 0.0;
    }
}

QString PhasorAO::getAudioInputName(uint channel) const
{
    switch (channel) {
    case 0: return tr("freq");
    case 1: return tr("offset");
    }

}

void PhasorAO::processAudio(uint, SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs  = audioInputs(thread),
            outputs = audioOutputs(thread);

    AUDIO::AudioBuffer
            * inFreq = inputs.size() < 1 ? 0 : inputs[0],
            * inOffs = inputs.size() < 2 ? 0 : inputs[1],
            * out    = outputs.isEmpty() ? 0 : outputs[0];

    if(!out) return;

    Double onedsr   = sampleRateInv();
    Double phase    = p_->phase[thread];
    for(uint i=0;i<out->blockSize();++i) {
        Double
                time = onedsr * (pos + i),
                freq = p_->paramFreq->value(time, thread),
                offs = p_->paramOffset->value(time,thread);
        if(inFreq)
            freq += inFreq->read(i);
        if(inOffs)
            offs += inOffs->read(i);

        Double increment = freq * onedsr;
        out->write(i,phase);
        phase += increment;
        if(phase > 1.0) phase -= 1.0;
        else if(phase < 0.0) phase += 1.0;
    }
    p_->phase[thread] = phase;
}

}
