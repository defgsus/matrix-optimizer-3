/** @file parameterao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include "parameterao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "math/constants.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(ParameterAO)

ParameterAO::ParameterAO(QObject *parent)
    : AudioObject   (parent),
      lastSample_   (0.f),
      lastSample2_  (0.f),
      samplesWaited_(0)
{
    setName("Parameter");
    setNumberAudioInputs(0);
    setNumberAudioOutputs(1);
}

void ParameterAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aoparam", 1);
}

void ParameterAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aoparam", 1);
}

void ParameterAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("in", tr("input"));

        paramValue_ = params()->createFloatParameter("v", tr("value"),
                                                   tr("The input value"),
                                                   0.0, 0.05);

        paramRate_ = params()->createFloatParameter("rate", tr("sampling rate"),
                                                   tr("Number of samples per second"),
                                                   30.0, 0.01, 9999999,
                                                   1,
                                                   true, false);

    params()->endParameterGroup();
}

void ParameterAO::processAudio(uint , SamplePos pos, uint thread)
{
    if (!audioOutputs(thread).isEmpty())
    if (auto out = audioOutputs(thread).first())
    {
        // simply copy when no modulation present
        if (!paramValue_->isModulated())
        {
            F32 val = paramValue_->value(sampleRateInv() * pos, thread);

            for (SamplePos i=0; i<out->blockSize(); ++i)
                out->write(i, val);
        }

        // or resample
        else
        {
            // number of samples to ignore the input
            const SamplePos rate =
                    1.0 / paramRate_->value(sampleRateInv() * pos, thread) * sampleRate();

            for (SamplePos i=0; i<out->blockSize(); ++i)
            {
                if (samplesWaited_++ >= rate)
                {
                    samplesWaited_ = 0;
                    lastSample2_ = lastSample_;
                    lastSample_ = paramValue_->value(
                                sampleRateInv() * (pos + i), thread);
                }

                F32 f = F32(samplesWaited_) / rate;
                out->write(i, lastSample2_ + f * (lastSample_ - lastSample2_));
            }
        }
    }
}

} // namespace MO
