/** @file shaperao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.12.2014</p>
*/

#include <memory>

#include "ShaperAO.h"
#include "audio/tool/AudioBuffer.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterTimeline1d.h"
#include "object/param/ParameterText.h"
#include "math/constants.h"
#include "math/functions.h"
#include "math/Timeline1d.h"
#include "math/funcparser/parser.h"
#include "io/DataStream.h"
#include "io/error.h"


namespace MO {

MO_REGISTER_OBJECT(ShaperAO)

class ShaperAO::Private
{
    public:

    Private(ShaperAO * s) : shaper(s) { }

    void updateEquations();

    struct EquationObject
    {
        EquationObject()
            : equ       (new PPP_NAMESPACE::Parser()),
              equPtr    (std::shared_ptr<PPP_NAMESPACE::Parser>(equ)),
              out       (0.0)

        {
            equ->variables().add("x", &input, ShaperAO::tr("The current audio sample").toStdString());
            equ->variables().add("x1", &input1, ShaperAO::tr("The previous audio sample").toStdString());
            equ->variables().add("x2", &input2, ShaperAO::tr("The one-before-previous audio sample").toStdString());
            equ->variables().add("out", &out, ShaperAO::tr("The last output sample").toStdString());
        }

        F32 operator()(F32 in)
        {
            input2 = input1;
            input1 = input;
            input = in;
            return out = equ->eval();
        }

        PPP_NAMESPACE::Parser * equ;
        std::shared_ptr<PPP_NAMESPACE::Parser> equPtr;
        PPP_NAMESPACE::Float input, input1, input2, out;
    };

    ShaperAO * shaper;

    ParameterFloat
        * paramInAmp,
        * paramOutAmp;
    ParameterSelect
        * paramType;
    ParameterTimeline1D
        * paramCurve;
    ParameterText
        * paramEquation;

    // per thread / per channel
    std::vector<std::vector<EquationObject>> equations;
};

namespace {

    enum ShaperTypes
    {
        ST_TANH,
        ST_TANH_CHEAP,
        ST_CURVE,
        ST_EQUATION
    };

    static const QStringList valueIds =
    {
        "tanh", "tanhcheap",
        "curve",
        "equ"
    };

    static const QStringList valueNames =
    {
        ShaperAO::tr("tangens hyperbolicus"),
        ShaperAO::tr("tangens hyperbolicus (fast)"),
        ShaperAO::tr("adjustable curve"),
        ShaperAO::tr("math equation")
    };

    static const QStringList valueStatusTips =
    {
        ShaperAO::tr("A saturating function"),
        ShaperAO::tr("A saturating function - tangens hyperbolicus is approximated with a cpu-friendly formula"),
        ShaperAO::tr("A shaping function defined by a graphical curve"),
        ShaperAO::tr("A mathematical function mapping input to output values")
    };

    static const QList<int> valueList =
    {
        ST_TANH,
        ST_TANH_CHEAP,
        ST_CURVE,
        ST_EQUATION
    };

} // namespace


ShaperAO::ShaperAO()
    : AudioObject   (),
      p_            (new Private(this))
{
    setName("Shaper");
    setNumberChannelsAdjustable(true);
}

ShaperAO::~ShaperAO()
{
    delete p_;
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
    AudioObject::createParameters();

    params()->beginParameterGroup("out", tr("output"));
    initParameterGroupExpanded("out");

        p_->paramType = params()->createSelectParameter("_shaper_type", tr("type"),
                                                        tr("The type of shaping function"),
                                                        valueIds,
                                                        valueNames,
                                                        valueStatusTips,
                                                        valueList,
                                                        ST_TANH_CHEAP,
                                                        true, false);

        // -- adjustable curve --

        auto deftl = new MATH::Timeline1d();
        ScopedRefCounted tldel(deftl, "ShaperAO defaultvalue");
        deftl->add(-1, -1, MATH::TimelinePoint::SYMMETRIC_USER);
        deftl->add(1, 1);
        p_->paramCurve = params()->createTimeline1DParameter(
                    "_shaper_curve", tr("input->output curve"),
                    tr("The curve mapping from input value to output value"),
                    deftl, true);
        // create timeline component now (instead of in audio-thread)
        p_->paramCurve->timeline();

        // -- equation text --

        p_->paramEquation = params()->createTextParameter("_shaper_equ", tr("equation"),
                  tr("An equation mapping input to output values"),
                  TT_EQUATION,
                  "x", true, false);
        Private::EquationObject tmpequ;
        p_->paramEquation->setVariableNames(tmpequ.equ->variables().variableNames());
        p_->paramEquation->setVariableDescriptions(tmpequ.equ->variables().variableDescriptions());


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
    AudioObject::updateParameterVisibility();

    auto type = (ShaperTypes)p_->paramType->baseValue();

    p_->paramCurve->setVisible(type == ST_CURVE);
    p_->paramEquation->setVisible(type == ST_EQUATION);
}

void ShaperAO::onParameterChanged(Parameter *p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->paramEquation)
        p_->updateEquations();
}

void ShaperAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);

    p_->equations.resize(num);
}


void ShaperAO::Private::updateEquations()
{
    const std::string text = paramEquation->baseValue().toStdString();
    for (auto & t : equations)
    {
        for (auto & e : t)
        {
            e.input = e.input1 = e.input2 = 0;
            if (!e.equ->parse(text))
                MO_WARNING("parsing failed for equation in ShaperAO '" << shaper->idName() << "'"
                           " (text = '" << text << "')");
        }
    }
}

void ShaperAO::setAudioBuffers(uint thread, uint ,
                               const QList<AUDIO::AudioBuffer *> &inputs,
                               const QList<AUDIO::AudioBuffer *> &outputs)
{
    // create EquationObject for each channel
    int num = std::max(inputs.size(), outputs.size());
    p_->equations[thread].resize(num);
    p_->updateEquations();
}

void ShaperAO::processAudio(const RenderTime& time)
{
#define MO__PROCESS(func__)                                                     \
    AUDIO::AudioBuffer::process(audioInputs(time.thread()),                     \
                                audioOutputs(time.thread()),                    \
    [=](uint channel, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)  \
    {                                                                           \
        Q_UNUSED(channel);                                                      \
        RenderTime ti(time);                                                    \
        for (SamplePos i=0; i<ti.bufferSize(); ++i)                             \
        {                                                                       \
            const F32 inamp = p_->paramInAmp->value(ti);                        \
            const F32 outamp = p_->paramOutAmp->value(ti);                      \
            out->write(i, outamp * (func__(inamp * in->read(i))));              \
            ti += SamplePos(1);                                                 \
        }                                                                       \
    });

    switch ((ShaperTypes)p_->paramType->baseValue())
    {
        case ST_TANH:       MO__PROCESS(tanh); break;
        case ST_TANH_CHEAP: MO__PROCESS(MATH::fast_tanh); break;
        case ST_CURVE:      MO__PROCESS(p_->paramCurve->timeline()->get); break;
        case ST_EQUATION:   MO__PROCESS(p_->equations[time.thread()][channel]); break;
    }

#undef MO__PROCESS
}

} // namespace MO
