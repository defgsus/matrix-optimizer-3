/** @file microphone.cpp

    @brief basic microphone object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MO_DISABLE_SPATIAL

#include <math.h>

#include "Microphone.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "audio/spatial/SpatialMicrophone.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(Microphone)

Microphone::Microphone()
    : Object()
{
    setName("Microphone");
    setNumberOutputs(ST_TRANSFORMATION, 1);
    setNumberMicrophones(1);
}

Microphone::~Microphone() { }

void Microphone::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("mic", 1);
}

void Microphone::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("mic", 1);
}

void Microphone::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("mic", tr("microphone"));
    initParameterGroupExpanded("mic",true);

        paramAmp_ = params()->createFloatParameter(
                    "amp", tr("amplitude"),
                    tr("Amplitude of the microphone signal"),
                    1., 0.1);

        paramDistFade_ = params()->createFloatParameter(
                    "dist_fade", tr("distance fall-off"),
                    tr("Amplitude decimation by distance factor"),
                    1., 0.1);
        paramDistFade_->setMinValue(0.);

        paramDirExp_ = params()->createFloatParameter(
                    "dir_exp", tr("directional exponent"),
                    tr("The exponent of the directional term defining the "
                       "opening angle of the microphone, the higher - the narrower"),
                    3., 0.1);
        paramDirExp_->setMinValue(0.);


        paramUseDist_ = params()->createBooleanParameter(
                    "use_dist", tr("distance sound"),
                    tr("Enable mixing with distance-sound if sound sources provide it"),
                    tr("Off"), tr("On"),
                    true, true, false);

        paramDistMin_ = params()->createFloatParameter(
                    "min_dist", tr("minimum distance"),
                tr("The minimum distance where mixing-in the distance-sound starts"),
                    3., .1, true, true);
        paramDistMin_->setMinValue(0.);

        paramDistMax_ = params()->createFloatParameter(
                    "max_dist", tr("maximum distance"),
                tr("The distance where the distance-sound is fully mixed-in"),
                    20., .1, true, true);
        paramDistMax_->setMinValue(0.);

    params()->endParameterGroup();
}

void Microphone::updateParameterVisibility()
{
    Object::updateParameterVisibility();

    const bool useDist = paramUseDist_->baseValue();
    paramDistMin_->setVisible(useDist);
    paramDistMax_->setVisible(useDist);
}

void Microphone::processMicrophoneBuffers(
        const QList<AUDIO::SpatialMicrophone*>& mics,
        const RenderTime& time)
{
    const F32
            amp = paramAmp_->value(time),
            distFade = paramDistFade_->value(time),
            dirExp = std::max(0.00001, paramDirExp_->value(time));
    const bool useDist = paramUseDist_->baseValue();
    F32 minDist=0., maxDist=0.;
    if (useDist)
        minDist = paramDistMin_->value(time),
        maxDist = paramDistMax_->value(time);

    for (auto m : mics)
    {
        m->setAmplitude(amp);
        m->setDistanceFadeout(distFade);
        m->setDirectionExponent(dirExp);
        m->setDistanceSound(useDist, minDist, maxDist);
    }
}

} // namespace MO

#endif // #ifndef MO_DISABLE_SPATIAL
