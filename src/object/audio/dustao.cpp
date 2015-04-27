/** @file dustao.cpp

    @brief A random impulse generator

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include "dustao.h"

#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "audio/tool/audiobuffer.h"
#include "io/datastream.h"

#include <cmath>

namespace MO {

MO_REGISTER_OBJECT(DustAO)

class DustAO::Private
{
    public:

    enum Mode {
        M_DUST,
        M_DUST2
    };

    Private(DustAO *ao) : ao(ao) { }

    Mode mode() const { return (Mode)paramMode->baseValue(); }

    void processAudio(uint size, SamplePos pos, uint thread );

    DustAO * ao;
    std::vector<Double>
            density0,
            rand,
            scale,
            threshold;
    ParameterFloat
            * paramAmp,
            * paramDensity;
    ParameterSelect
            * paramMode;
};

DustAO::DustAO(QObject * parent)
    : AudioObject(parent)
    , p_         (new Private(this))
{
    setName("Dust");
    setNumberAudioInputs(2);
    setNumberAudioOutputs(1);
    srand(time(NULL));
}

DustAO::~DustAO()
{
    delete p_;
}

void DustAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aodust",1);
}

void DustAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aodust",1);
}

void DustAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("dust", tr("random impulse generator"));
    initParameterGroupExpanded("dust");

        p_->paramAmp     = params()->createFloatParameter("dust_amp", tr("amplitude"),
                                                          tr("The amplitude of the impulses"),
                                                          1.0, 0.05);
        p_->paramDensity = params()->createFloatParameter("dust_dens", tr("density"),
                                                          tr("The density of the impulses"),
                                                          0.5, 0.05);
        p_->paramMode    = params()->createSelectParameter("dust_mode", tr("generator mode"),
                           tr("Selects the type of random generator"),
                           {"dust", "dust2"},
                           {tr("dust"), tr("dust2")},
                           {tr("Creates impulses from 0 to 1"), tr("Creates impulses from -1 to 1")},
                           {Private::M_DUST, Private::M_DUST2},
                           Private::M_DUST);
    params()->endParameterGroup();
}

void DustAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();
}

void DustAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);
}

void DustAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();
}

void DustAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
    p_->density0.resize(num);
    p_->rand.resize(num);
    p_->scale.resize(num);
    p_->threshold.resize(num);
    for(uint i=0; i<num; ++i) {
        p_->density0[i]  = 0.0;
        p_->rand[i]      = rand();
        p_->scale[i]     = 0.0;
        p_->threshold[i] = 0.0;
    }
}

QString DustAO::getAudioInputName(uint channel) const
{
    switch(channel)
    {
    case 0: return tr("amplitude");
    case 1: return tr("density");
    }
    return AudioObject::getAudioInputName(channel);
}

static const Double dv2_31 = 4.656612873077392578125e-10;

void DustAO::processDust(uint, SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*> &
            inputs  = audioInputs(thread),
            outputs = audioOutputs(thread);

    AUDIO::AudioBuffer
            * out    = outputs.isEmpty() ? 0 : outputs[0],
            * inAmp  = inputs.size() < 2 ? 0 : inputs[1],
            * inDens = inputs.size() < 3 ? 0 : inputs[2];

    if(!out) return;

    Double  time   = sampleRateInv() * pos;

    Double
            thresh, scale,
            dens0  = p_->density0[thread],
            dens   = p_->paramDensity->value(time,thread);

    if(inDens)
        dens += inDens->read(0);

    if(dens != dens0) {
        thresh = p_->threshold[thread] = dens * sampleRateInv();
        scale  = p_->scale[thread] = (thresh > 0.0 ? 1.0/thresh : 0.0);
        p_->density0[thread] = dens;
    } else {
        thresh = p_->threshold[thread];
        scale  = p_->scale[thread];
    }

    for(uint i=0; i<out->blockSize(); ++i) {
        Double  time = sampleRateInv() * (pos + i);
        Double  amp  = p_->paramAmp->value(time,thread);

        if(inAmp)
            amp += inAmp->read(i);

        p_->rand[thread] = rand();
        Double r = p_->rand[thread] * dv2_31;

        out->write(i, amp * ((r < thresh) ? r * scale : 0.0));
    }
}

void DustAO::processDust2(uint, SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*> &
            inputs  = audioInputs(thread),
            outputs = audioOutputs(thread);

    AUDIO::AudioBuffer
            * out    = outputs.isEmpty() ? 0 : outputs[0],
            * inAmp  = inputs.size() < 2 ? 0 : inputs[1],
            * inDens = inputs.size() < 3 ? 0 : inputs[2];

    if(!out) return;

    Double  time   = sampleRateInv() * pos;

    Double
            thresh, scale,
            dens0  = p_->density0[thread],
            dens   = p_->paramDensity->value(time,thread);

    if(inDens)
        dens += inDens->read(0);

    if(dens != dens0) {
        thresh = p_->threshold[thread] = dens * sampleRateInv();
        scale  = p_->scale[thread] = (thresh > 0.0 ? 2.0/thresh : 0.0);
        p_->density0[thread] = dens;
    } else {
        thresh = p_->threshold[thread];
        scale  = p_->scale[thread];
    }

    for(uint i=0; i<out->blockSize(); ++i) {
        Double  time = sampleRateInv() * (pos + i);
        Double  amp  = p_->paramAmp->value(time,thread);

        if(inAmp)
            amp += inAmp->read(i);

        p_->rand[thread] = rand();
        Double r = p_->rand[thread] * dv2_31;

        out->write(i, amp * ((r < thresh) ? r * scale -1.0 : 0.0));
    }

}

void DustAO::processAudio(uint i, SamplePos pos, uint thread)
{
    switch(p_->mode()) {
    case Private::M_DUST2:
        processDust2(i, pos, thread);
        break;
    case Private::M_DUST:
    default:
    processDust(i, pos, thread);
    }
}



}
