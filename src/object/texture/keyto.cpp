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
            * p_mode, * p_hsvComp, * p_invert;
    ParameterFloat
            * p_r, * p_g, * p_b,
            * p_h, * p_s, * p_v,
            * p_tolerance, *p_range,
            * p_near, * p_far, * p_dist, * p_dist_range,
            * p_mix_alpha, * p_mix_white, * p_mix_black;

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

        p_mode = to->params()->createSelectParameter("mode", tr("color selection"),
                    tr("Selects the mode to describe the color and range"),
        { "rgb", "hsv", "depth" },
        { tr("RGB"), tr("HSV"), tr("depth buffer") },
        { tr("Red/Green/Blue color space is used"),
          tr("Hue/Saturation/Value color space is used"),
          tr("A focal distance can be selected when the input is a depth buffer") },
        { 0, 1, 2 },
        0, true, false);

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

        p_near = to->params()->createFloatParameter("depth_near", tr("near plane"),
                            tr("The near-plane as defined in the camera that rendered the depth map"),
                               0.05,  0.01);
        p_near->setMinValue(0.00001);

        p_far = to->params()->createFloatParameter("depth_far", tr("far plane"),
                            tr("The far-plane as defined in the camera that rendered the depth map"),
                               1000.,  1.0);
        p_far->setMinValue(0.00002);

        p_dist = to->params()->createFloatParameter("depth", tr("focal depth"),
                            tr("The distance to the desired depth plane"),
                               10.,  .1);

        p_dist_range = to->params()->createFloatParameter("depth_range", tr("tolerance"),
                            tr("The tolerance in distance around the desired depth plane"),
                               5.,  .1);
        p_dist_range->setMinValue(0.0001);

        p_hsvComp = to->params()->createBooleanParameter("hsvcomp", tr("compare in hsv"),
                                    tr("Use hue/saturation/value space for comparison with key color"),
                                    tr("Red/green/blue color space is used for comparison"),
                                    tr("Hue/saturation/value color space is used for comparison"),
                                                        false, true, false);

        p_invert = to->params()->createBooleanParameter("invert", tr("invert"),
                                    tr("Invert the alpha selection"),
                                    tr("Off"),
                                    tr("The selection is completely reversed"),
                                    false, true, true);

        p_mix_alpha = to->params()->createFloatParameter("mix_alpha", tr("use source alpha"),
                tr("Mix between key-alpha and key-alpha multiplied with source-alpha channel"), 1.0,  0.,1.,  0.05);

        p_mix_black = to->params()->createFloatParameter("mix_black", tr("alpha to black"),
                                                         tr("Mix between transparency and blackness"), 0.0,  0.,1.,  0.05);

        p_mix_white = to->params()->createFloatParameter("mix_white", tr("white mask"),
                                                         tr("Mix the output color to white"), 0.0,  0.,1.,  0.05);


    to->params()->endParameterGroup();
}

void KeyTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_hsvComp
        || p == p_->p_mode)
        requestReinitGl();

}

void KeyTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void KeyTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    const bool
            rgb = p_->p_mode->baseValue() == 0,
            hsv = p_->p_mode->baseValue() == 1,
            depth = p_->p_mode->baseValue() == 2;
    p_->p_r->setVisible(rgb);
    p_->p_g->setVisible(rgb);
    p_->p_b->setVisible(rgb);
    p_->p_h->setVisible(hsv);
    p_->p_s->setVisible(hsv);
    p_->p_v->setVisible(hsv);
    p_->p_near->setVisible(depth);
    p_->p_far->setVisible(depth);
    p_->p_dist->setVisible(depth);
    p_->p_dist_range->setVisible(depth);
    p_->p_range->setVisible(rgb || hsv);
    p_->p_tolerance->setVisible(rgb || hsv);
    p_->p_hsvComp->setVisible(!depth);
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
        if (p_mode->baseValue() == 2)
            src.addDefine("#define DEPTH_COMPARE");
        else
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

    u_color = shader->getUniform("u_color", false);
    u_range = shader->getUniform("u_range", true);
    u_mix = shader->getUniform("u_mix", true);
}

void KeyTO::Private::releaseGl()
{

}

void KeyTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    // update uniforms

    Float r=1.f, g=0.f, b=0.f;
    if (p_mode->baseValue() == 0)
    {
        r = p_r->value(time, thread);
        g = p_g->value(time, thread);
        b = p_b->value(time, thread);
    }

    if (p_hsvComp->baseValue() == 0)
    {
        // hsv -> rgb
        if (p_mode->baseValue() == 1)
        {
            /** @todo use same hsv/rgb conversion in c++ as in inc/color.glsl */
            auto c = QColor::fromHsvF(
                                MATH::moduloSigned(p_h->value(time, thread), 1.),
                                p_s->value(time, thread),
                                p_v->value(time, thread));
            if (u_color)
                u_color->setFloats(c.redF(), c.greenF(), c.blueF());
        }
        // rgb -> rgb
        else
            if (u_color)
                u_color->setFloats(r, g, b);
    }
    else
    {
        // hsv -> hsv
        if (p_mode->baseValue() == 1)
        {
            if (u_color)
            u_color->setFloats( p_h->value(time, thread),
                                p_s->value(time, thread),
                                p_v->value(time, thread));
        }
        // rgb -> hsv
        else
        {
            auto c = QColor::fromRgbF(r, g, b);
            if (u_color)
            u_color->setFloats(c.hueF(), c.saturationF(), c.valueF());
        }
    }

    // depth compare ?
    if (p_mode->baseValue() == 2)
    {
        Float   near = p_near->value(time, thread),
                far = p_far->value(time, thread),
                len = std::max(0.0001f, far - near),
                depth = p_dist->value(time, thread),
                range = p_dist_range->value(time, thread),

                // convert ranges to z-buffer value
                a = (far + near) / (2.f * len) + .5f,
                b = (-far * near) / len,

                th = a + (1.f / depth) * b,
                tol = th - (a + (1.f / (depth - range)) * b);

        if (p_invert->value(time, thread))
            u_range->setFloats(0.f, tol, th);
        else
            u_range->setFloats(tol, 0.f, th);
        MO_PRINT(th);
    }
    // color compare
    else
    {
        Float th = 1. - p_tolerance->value(time, thread),
              s = p_invert->value(time, thread) ? -1.f : 1.f,
              r = s * p_range->value(time, thread);

        u_range->setFloats(  std::max(0.f, std::min(1.f, th - r)),
                             std::max(0.f, std::min(1.f, th + r)));
    }

    u_mix->setFloats(           p_mix_alpha->value(time, thread),
                                p_mix_white->value(time, thread),
                                p_mix_black->value(time, thread));

    //to->shaderQuad(0)->shader()->dumpUniforms();
    to->renderShaderQuad(time, thread);
}


} // namespace MO
