/** @file noiseao.cpp

    @brief A noise generator

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include "NoiseAO.h"

#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterText.h"
#include "audio/tool/AudioBuffer.h"
#include "io/ApplicationTime.h"
#include "io/DataStream.h"

#include <cmath>

namespace MO {

MO_REGISTER_OBJECT(NoiseAO)

class NoiseAO::Private
{
    public:

    enum Mode {
        M_WHITE,
        M_PINK
    };

    Private(NoiseAO *ao)
        : ao(ao)
        , seed(applicationTime() * 7654321)
    {}

    Mode mode() const { return (Mode)paramMode->baseValue(); }

    int myRand31(int *seedVal) const
    {
        uint64_t tmp1, tmp2;

        tmp1 = (uint64_t) ((int32_t) (*seedVal) * (int64_t) 742938285);
        tmp2 = (uint32_t) tmp1 & (uint32_t) 0x7FFFFFFF;
        tmp2 += (uint32_t) (tmp1 >> 31);
        tmp2 = (tmp2 & (uint32_t) 0x7FFFFFFF) + (tmp2 >> 31);
        (*seedVal) = (int) tmp2;
        return (int) tmp2;
    }

    int rand31(void) {
        return myRand31(&seed) - 1;
    }

    NoiseAO * ao;
    int seed;
    std::vector<Double> last, lastbeta, coeff, ampmod, ampinc;
    std::vector<std::vector<Double>> b_coeffs;
    ParameterFloat
            * paramAmp,
            * paramBeta;
    ParameterSelect
            * paramMode;
};

NoiseAO::NoiseAO()
    : AudioObject   ()
    , p_          (new Private(this))
{
    setName("Noise");
    setNumberAudioInputs(0);
    setNumberAudioOutputs(1);
}

NoiseAO::~NoiseAO()
{
    delete p_;
}

void NoiseAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aonoise", 1);
}

void NoiseAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aonoise", 1);
}

void NoiseAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
    p_->ampmod.resize(num);
    p_->coeff.resize(num);
    p_->last.resize(num);
    p_->lastbeta.resize(num);
    p_->b_coeffs.resize(num);
    for(uint i=0;i<num;++i) {
        p_->last[i]     = 0.0;
        p_->lastbeta[i] = p_->paramBeta->baseValue();
        p_->coeff[i]    = sqrt(1.0 - p_->last[i]*p_->lastbeta[i]);
        p_->ampmod[i]   = 0.785/(1.0+p_->lastbeta[i]);
        p_->b_coeffs[i].resize(7);
        for(auto & f : p_->b_coeffs[i]) f = 0.0;
    }
}


void NoiseAO::createParameters()
{
    AudioObject::createParameters();
    params()->beginParameterGroup("noise", tr("noise generator"));
    initParameterGroupExpanded("noise");

    p_->paramAmp  = params()->createFloatParameter("noise_amp", tr("amplitude"),
                                                   tr("The amplitude of the signal"),
                                                   1.0, 0.05);
    p_->paramBeta = params()->createFloatParameter("noise_beta", tr("beta"),
                                                   tr("The beta for the lowpass in the range (-1,1)"),
                                                   0.0, 0.05);
    p_->paramMode = params()->createSelectParameter("noise_mode", tr("noise mode"),
                                                    tr("Selects the type of noise generated"),
                                                    {"white", "pink"},
                                                    {tr("White"), tr("Pink")},
                                                    {tr("White noise"), tr("Almost pink noise")},
                                                    {Private::M_WHITE, Private::M_PINK},
                                                    Private::M_WHITE);
    params()->endParameterGroup();
}

QString NoiseAO::getAudioInputName(uint channel) const
{
//    switch(channel) {
//    case 0: return "amp";
//    case 1: return "beta";
//    }

    return AudioObject::getAudioInputName(channel);
}

void NoiseAO::processWhiteNoise(const RenderTime& rtime)
{
    const QList<AUDIO::AudioBuffer*>&
//            inputs  = audioInputs(rtime.thread()),
            outputs = audioOutputs(rtime.thread());

    AUDIO::AudioBuffer
            * out    = outputs.isEmpty() ? 0 : outputs[0];

    if(!out) return;

    RenderTime time(rtime);
    for(uint i=0;i<out->blockSize();++i) {
        Double
                beta   = p_->paramBeta->value(time);
        p_->lastbeta[time.thread()] = beta;

        Double
                coeff  = p_->coeff[time.thread()]    = sqrt(1.0 - p_->lastbeta[time.thread()] * p_->lastbeta[time.thread()]),
                ampmod = p_->ampmod[time.thread()]   = 0.785/(1.0 + p_->lastbeta[time.thread()]),
                lastx  = p_->last[time.thread()],
                amp    = p_->paramAmp->value(time);

        Double
                rnd = 2.0 * Double(p_->rand31()) / Double(2147483645) - 1.0;

        lastx = lastx * beta + coeff * rnd;

        out->write(i, lastx * ampmod * amp);
        time += SamplePos(1);
    }
}

void NoiseAO::processPinkNoise(const RenderTime& rtime)
{
    const QList<AUDIO::AudioBuffer*>&
//            inputs  = audioInputs(rthread),
            outputs = audioOutputs(rtime.thread());

    AUDIO::AudioBuffer
            * out    = outputs.isEmpty() ? 0 : outputs[0];

    std::vector<Double> c_coeffs(7);
    Double next_in, next_out;
    std::copy(p_->b_coeffs[rtime.thread()].begin(),
              p_->b_coeffs[rtime.thread()].end(),
              c_coeffs.begin());
    RenderTime time(rtime);
    for(uint i=0;i<out->blockSize();++i) {
        Double
                amp  = p_->paramAmp->value(time),
                beta   = p_->paramBeta->value(time);
        p_->lastbeta[time.thread()] = beta;

        Double
                coeff  = p_->coeff[time.thread()]    = sqrt(1.0 - p_->lastbeta[time.thread()] * p_->lastbeta[time.thread()]),
                ampmod = p_->ampmod[time.thread()]   = 0.785/(1.0 + p_->lastbeta[time.thread()]),
                lastx  = p_->last[time.thread()];

        Double
                rnd = 2.0 * Double(p_->rand31()) / Double(2147483645) - 1.0;

        lastx = lastx * beta + coeff * rnd;

        next_in = lastx * ampmod * amp;

        // Paul Kellets "refined Pink Filter"
        c_coeffs[0] = c_coeffs[0] * 0.99886 + next_in * 0.0555179;
        c_coeffs[1] = c_coeffs[1] * 0.99332 + next_in * 0.0750759;
        c_coeffs[2] = c_coeffs[2] * 0.96900 + next_in * 0.1538520;
        c_coeffs[3] = c_coeffs[3] * 0.86650 + next_in * 0.3104856;
        c_coeffs[4] = c_coeffs[4] * 0.55000 + next_in * 0.5329522;
        c_coeffs[5] = c_coeffs[5] * -0.7616 - next_in * 0.0168980;
        Double sum = 0;
        for (auto & c : c_coeffs)
            sum += c;
        next_out = sum + next_in * 0.5362;
//        next_out = c_coeffs[0]
//                + c_coeffs[1]
//                + c_coeffs[2]
//                + c_coeffs[3]
//                + c_coeffs[4]
//                + c_coeffs[5]
//                + c_coeffs[6] + next_in * 0.5362;
        out->write(i, next_out * 0.11 * amp);
        c_coeffs[6] = next_in * 0.115926;
        time += SamplePos(1);
    }
    std::copy(c_coeffs.begin(),
              c_coeffs.end(),
              p_->b_coeffs[time.thread()].begin());
}

void NoiseAO::processAudio(const RenderTime& time)
{
    switch(p_->mode()) {
    case Private::M_PINK:
        processPinkNoise(time);
        break;
    case Private::M_WHITE:
    default:
        processWhiteNoise(time);
    }
}

}
