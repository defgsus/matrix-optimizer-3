/** @file keyto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.05.2015</p>
*/

#include <QColor>

#include "keyto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "math/functions.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(KeyTO)

struct KeyTO::Private
{
    Private(KeyTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

    KeyTO * to;

    ParameterSelect
            * p_hsvIn, * p_hsvComp;
    ParameterFloat
            * p_r, * p_g, * p_b,
            * p_h, * p_s, * p_v,
            * p_tolerance, *p_range,
            * p_mix_alpha, * p_mix_white;

    GL::Uniform
            * u_color,
            * u_range,
            * u_mix;
};


KeyTO::KeyTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Key");
    initMaximumTextureInputs(1);
}

KeyTO::~KeyTO()
{
    delete p_;
}

void KeyTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("tokey", 1);
}

void KeyTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("tokey", 1);
}

void KeyTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void KeyTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void KeyTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void KeyTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
}


void KeyTO::Private::createParameters()
{
    to->params()->beginParameterGroup("key", tr("key"));
    to->initParameterGroupExpanded("key");

        p_hsvIn = to->params()->createBooleanParameter("hsv", tr("hsv"),
                                    tr("Use hue/saturation/value space for key color"),
                                    tr("Red/green/blue color space is used"),
                                    tr("Hue/saturation/value color space is used"),
                                                        false, true, false);

        p_r = to->params()->createFloatParameter("red", tr("red"), tr("Red amount of key color"), 1.0,  0.,1.,  0.025);
        p_g = to->params()->createFloatParameter("green", tr("green"), tr("Green amount of key color"), 0.0,  0.,1.,  0.025);
        p_b = to->params()->createFloatParameter("blue", tr("blue"), tr("Blue amount of key color"), 0.0,  0.,1.,  0.025);

        p_h = to->params()->createFloatParameter("hue", tr("hue"), tr("Hue value of key color [0,1]"), 0.0, 0.025);
        p_s = to->params()->createFloatParameter("sat", tr("saturation"), tr("Saturation of key color"), 1.0,  0.,1.,  0.025);
        p_v = to->params()->createFloatParameter("val", tr("value"), tr("Brightness of key color"), 1.0, 0.,1.,  0.025);

        p_tolerance = to->params()->createFloatParameter("tolerance", tr("tolerance"),
                            tr("Selects the range around which to clear the alpha channel - "
                               "higher value means more tolerant"), 0.1,  0.,1.,  0.025);
        p_range = to->params()->createFloatParameter("range", tr("range"),
                            tr("A higher value creates a smoother transition between key and non-key color - "
                               "a negative value inverts the selection"), .1,  -1.,1.,  0.025);

        p_hsvComp = to->params()->createBooleanParameter("hsvcomp", tr("compare in hsv"),
                                    tr("Use hue/saturation/value space for comparison with key color"),
                                    tr("Red/green/blue color space is used for comparison"),
                                    tr("Hue/saturation/value color space is used for comparison"),
                                                        false, true, false);

        p_mix_alpha = to->params()->createFloatParameter("mix_alpha", tr("use source alpha"),
                tr("Mix between key-alpha and key-alpha multiplied with source-alpha channel"), 1.0,  0.,1.,  0.05);

        p_mix_white = to->params()->createFloatParameter("mix_white", tr("white mask"),
                                                         tr("Mix the output color to white"), 0.0,  0.,1.,  0.05);
    to->params()->endParameterGroup();
}

void KeyTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_hsvComp)
        requestReinitGl();

}

void KeyTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void KeyTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    bool useHsv = p_->p_hsvIn->baseValue() != 0;
    p_->p_r->setVisible(!useHsv);
    p_->p_g->setVisible(!useHsv);
    p_->p_b->setVisible(!useHsv);
    p_->p_h->setVisible(useHsv);
    p_->p_s->setVisible(useHsv);
    p_->p_v->setVisible(useHsv);
}




void KeyTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/key.frag");
        src.pasteDefaultIncludes();
        if (p_hsvComp->baseValue())
            src.addDefine("#define HSV_COMPARE");
    }
    catch (Exception& )
    {
        // XXX Signal to gui
        throw;
    }

    auto shader = to->createShaderQuad(
                src, { "u_tex" })->shader();

    // uniforms

    u_color = shader->getUniform("u_color", true);
    u_range = shader->getUniform("u_range", true);
    u_mix = shader->getUniform("u_mix", true);
}

void KeyTO::Private::releaseGl()
{

}

void KeyTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    // update uniforms

    if (p_hsvComp->baseValue() == 0)
    {
        // rgb -> rgb
        if (p_hsvIn->baseValue() == 0)
        {
            u_color->setFloats( p_r->value(time, thread),
                                p_g->value(time, thread),
                                p_b->value(time, thread));
        }
        // hsv -> rgb
        else
        {
            /** @todo use same hsv/rgb conversion in c++ as in inc/color.glsl */
            auto c = QColor::fromHsvF(
                                MATH::moduloSigned(p_h->value(time, thread), 1.),
                                p_s->value(time, thread),
                                p_v->value(time, thread));
            u_color->setFloats(c.redF(), c.greenF(), c.blueF());
        }
    }
    else
    {
        // rgb -> hsv
        if (p_hsvIn->baseValue() == 0)
        {
            auto c = QColor::fromRgbF(
                                p_r->value(time, thread),
                                p_g->value(time, thread),
                                p_b->value(time, thread));
            u_color->setFloats(c.hueF(), c.saturationF(), c.valueF());
        }
        // hsv -> hsv
        else
        {
            u_color->setFloats( p_h->value(time, thread),
                                p_s->value(time, thread),
                                p_v->value(time, thread));
        }
    }

    Float th = 1. - p_tolerance->value(time, thread),
          r = p_range->value(time, thread);
    u_range->setFloats(         std::max(0.f, std::min(1.f, th - r)),
                                std::max(0.f, std::min(1.f, th + r)));

    u_mix->setFloats(           p_mix_alpha->value(time, thread),
                                p_mix_white->value(time, thread));

    to->renderShaderQuad(time, thread);
}


} // namespace MO
