/** @file shaperao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.12.2014</p>
*/

#include "shaperao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertimeline1d.h"
#include "math/constants.h"
#include "math/functions.h"
#include "math/timeline1d.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(ShaperAO)

class ShaperAO::Private
{
    public:

    ParameterFloat
        * paramInAmp,
        * paramOutAmp;
    ParameterSelect
        * paramType;
    ParameterTimeline1D
        * paramCurve;
};

namespace {

    enum ShaperTypes
    {
        ST_TANH,
        ST_TANH_CHEAP,
        ST_CURVE
    };

    static const QStringList valueIds =
    {
        "tanh", "tanhcheap",
        "curve"
    };

    static const QStringList valueNames =
    {
        ShaperAO::tr("tangens hyperbolicus"),
        ShaperAO::tr("tangens hyperbolicus (fast)"),
        ShaperAO::tr("adjustable curve")
    };

    static const QStringList valueStatusTips =
    {
        ShaperAO::tr("A saturating function"),
        ShaperAO::tr("A saturating function - tangens hyperbolicus is approximated with a cpu-friendly formula"),
        ShaperAO::tr("A shaping function defined by a graphical curve")
    };

    static const QList<int> valueList =
    {
        ST_TANH,
        ST_TANH_CHEAP,
        ST_CURVE
    };

} // namespace


ShaperAO::ShaperAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private)
{
    setName("Shaper");
}

void ShaperAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aoshaper", 1);
}

void ShaperAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aoshaper", 1);
}

void ShaperAO::createParameters()
{
    params()->beginParameterGroup("out", tr("output"));

        p_->paramType = params()->createSelectParameter("_shaper_type", tr("type"),
                                                        tr("The type of shaping function"),
                                                        valueIds,
                                                        valueNames,
                                                        valueStatusTips,
                                                        valueList,
                                                        ST_TANH_CHEAP,
                                                        true, false);

        MATH::Timeline1D deftl;
        deftl.add(-1, -1, MATH::Timeline1D::Point::SPLINE4_SYM);
        deftl.add(1, 1);
        p_->paramCurve = params()->createTimeline1DParameter("_shaper_curve", tr("input->output curve"),
                                                    tr("The curve mapping from input value to output value"),
                                                    &deftl, true);
        // create timeline component now (instead of in audio-thread)
        p_->paramCurve->timeline();

        p_->paramInAmp = params()->createFloatParameter("_shaper_inamp", tr("input amplitude"),
                                                   tr("The amplitude of the audio input"),
                                                   1.0, 0.05);
        p_->paramOutAmp = params()->createFloatParameter("_shaper_outamp", tr("output amplitude"),
                                                   tr("The amplitude of the audio output"),
                                                   1.0, 0.05);
    params()->endParameterGroup();
}

void ShaperAO::updateParameterVisibility()
{
    p_->paramCurve->setVisible(p_->paramType->baseValue() == ST_CURVE);
}

void ShaperAO::processAudio(const QList<AUDIO::AudioBuffer *> &inputs,
                              const QList<AUDIO::AudioBuffer *> &outputs,
                              uint bsize, SamplePos pos, uint thread)
{
#define MO__PROCESS(func__)                                             \
    AUDIO::AudioBuffer::process(inputs, outputs,                        \
    [=](const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)        \
    {                                                                   \
        for (SamplePos i=0; i<bsize; ++i)                               \
        {                                                               \
            const Double time = sampleRateInv() * (pos + i);            \
            const F32 inamp = p_->paramInAmp->value(time, thread);      \
            const F32 outamp = p_->paramOutAmp->value(time, thread);    \
            out->write(i, outamp * (func__(inamp * in->read(i))));      \
        }                                                               \
    });

    switch ((ShaperTypes)p_->paramType->baseValue())
    {
        case ST_TANH:       MO__PROCESS(tanh); break;
        case ST_TANH_CHEAP: MO__PROCESS(MATH::fast_tanh); break;
        case ST_CURVE:      MO__PROCESS(p_->paramCurve->timeline()->get); break;
    }

#undef MO__PROCESS
}

} // namespace MO
