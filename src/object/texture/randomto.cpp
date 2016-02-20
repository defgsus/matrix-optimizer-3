/** @file randomto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.05.2015</p>
*/

#include "randomto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "math/constants.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(RandomTO)

struct RandomTO::Private
{
    Private(RandomTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    RandomTO * to;

    ParameterFloat
            * p_bright, * p_r, * p_g, * p_b, * p_a,
            * p_sat, *p_hue, *p_gamma, *p_gamma_r, *p_gamma_g, *p_gamma_b,
            * p_size, *p_size_add,
            * p_amp_mul,
            * p_rnd_rot,
            * p_voro_cell,
            * p_voro_smooth,
            * p_range_min,
            * p_range_max,
            * p_thresh;
    ParameterInt
            * p_seed,
            * p_steps;
    ParameterSelect
            * m_rotate,
            * m_noise,
            * m_mask,
            * m_alpha,
            * m_fractal;

    GL::Uniform
            * u_color,
            * u_hsv,
            * u_gamma_exp,
            * u_start_seed,
            * u_scale,
            * u_amp,
            * u_rnd_rotate,
            * u_voro,
            * u_mask,
            * u_max_steps;

};


RandomTO::RandomTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Random");
    initDefaultUpdateMode(UM_ON_CHANGE);
}

RandomTO::~RandomTO()
{
    delete p_;
}

void RandomTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("tornd", 1);
}

void RandomTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("tornd", 1);
}

void RandomTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void RandomTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void RandomTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void RandomTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void RandomTO::Private::createParameters()
{
    to->params()->beginParameterGroup("noise", tr("noise"));
    to->initParameterGroupExpanded("noise");

        m_noise = to->params()->createSelectParameter(
                    "noise_type", tr("noise type"),
                    tr("The type of basis noise function"),
                    { "0", "1", "2" },
                    { tr("rectangular"), tr("rectangular noise"), tr("voronoi noise") },
                    { tr("Hard-edged rectangular sections"),
                      tr("A typical Perlin-like noise function"),
                      tr("Voronoi noise by Iñigo Quilez") },
                    { 0, 1, 2 },
                    1,
                    true, false);

        p_seed = to->params()->createIntParameter(
                    "seed", tr("seed"),
                    tr("Random seed"), 0, true, true);

        p_voro_cell = to->params()->createFloatParameter(
                    "voro_cell", tr("voronoicity"),
                    tr("Mix between rectangular and voronoi shapes"), 1.0,  0.0, 1.,  0.05);

        p_voro_smooth = to->params()->createFloatParameter(
                    "voro_smooth", tr("smoothness"),
                    tr("Mix between sharp-edged and smooth"), 1.0,  0.0, 1.,  0.05);


        m_fractal = to->params()->createSelectParameter(
                    "fractal_type", tr("fractal type"),
                    tr("The type of multi-sampling function"),
                    { "0", "1", "2", "3" },
                    { tr("off"), tr("weighted average"), tr("maximum"), tr("recursive") },
                    { tr("One noise layer is produced"),
                      tr("Serveral layers of noise are combined by averaging"),
                      tr("The maximum value of all noise layers is used"),
                      tr("Each noise layer uses the previous noise as position modifier") },
                    { 0, 1, 2, 3 },
                    1,
                    true, false);

        p_steps = to->params()->createIntParameter(
                    "steps", tr("number steps"),
                    tr("The maximum number of noise layers to combine"), 4, true, true);
        p_steps->setMinValue(1);

        p_amp_mul = to->params()->createFloatParameter(
                    "amp_mul", tr("amplitude per layer"),
                    tr("Multiplier of the amplitude per layer"), 0.8,  0.05);

        p_size = to->params()->createFloatParameter(
                    "size", tr("size"), tr("The zoom-out factor"), 1.0,  0.1);
        p_size_add = to->params()->createFloatParameter(
                    "size_add", tr("size per layer"),
                    tr("Increase of the zoom-out factor per noise layer"), 2.0,  0.1);


        m_rotate = to->params()->createBooleanParameter(
                    "rotate", tr("rotate randomly"),
                    tr("Randomly rotates the noise function in each layer"),
                    tr("Off"), tr("On"),
                    false,
                    true, false);

        p_rnd_rot = to->params()->createFloatParameter(
                    "rotation", tr("rotation amount"),
                    tr("Amount of random rotation per layer in degree"), 45.0,  9.);


        m_mask = to->params()->createBooleanParameter(
                    "mask", tr("mask values"),
                    tr("Enables the selection of a specific value range"),
                    tr("Off"), tr("On"),
                    false,
                    true, false);

        p_range_min = to->params()->createFloatParameter(
                    "range_min", tr("range min"),
                    tr("Noise values below this value remain black"), 0.1,  0.0, 1.,  0.05);

        p_range_max = to->params()->createFloatParameter(
                    "range_max", tr("range max"),
                    tr("Noise values above this value are fully on"), 1.0,  0.0, 1.,  0.05);

        p_thresh = to->params()->createFloatParameter(
                    "threshold", tr("threshold"),
                    tr("Threshold value around which noise values are included"), .0,  0.0, 1.,  0.05);


        m_alpha = to->params()->createBooleanParameter(
                    "alpha_random", tr("random alpha channel"),
                    tr("Selects if the alpha channel is also randomized"),
                    tr("Off"), tr("On"),
                    false,
                    true, false);
        m_alpha->setDefaultEvolvable(false);

    to->params()->endParameterGroup();


    to->params()->beginParameterGroup("color", tr("color"));

        p_hue = to->params()->createFloatParameter(
                "hue", tr("hue"), tr("Hue value [0,1]"), 0.0,  0.05);

        p_r = to->params()->createFloatParameter("red", tr("red"), tr("Red amount of output Random"), 1.0, 0.1);
        p_g = to->params()->createFloatParameter("green", tr("green"), tr("Green amount of output Random"), 1.0, 0.1);
        p_b = to->params()->createFloatParameter("blue", tr("blue"), tr("Blue amount of output Random"), 1.0, 0.1);
        p_a = to->params()->createFloatParameter("alpha", tr("alpha"),
                    tr("Defines the opaqueness/transparency of the output [0,1]"),
                    1.0,
                    0.0, 1.0, 0.05);
        p_a->setDefaultEvolvable(false);

        p_sat = to->params()->createFloatParameter(
                    "saturation", tr("saturation"), tr("Saturation"), 1.0,  0.0, 1.,  0.1);

        p_bright = to->params()->createFloatParameter(
                    "bright", tr("brightness"), tr("Amplifier for output Random"), 1.0, 0.1);


        p_gamma = to->params()->createFloatParameter(
                    "gamma", tr("gamma"), tr("Gamma"), 1.0,  0.001, 1000.,  0.1);

        p_gamma_r = to->params()->createFloatParameter(
                    "gammar", tr("gamma red"), tr("Gamma of red channel"), 1.0,  0.001, 1000.,  0.1);

        p_gamma_g = to->params()->createFloatParameter(
                    "gammag", tr("gamma green"), tr("Gamma of green channel"), 1.0,  0.001, 1000.,  0.1);

        p_gamma_b = to->params()->createFloatParameter(
                    "gammab", tr("gamma blue"), tr("Gamma of blue channel"), 1.0,  0.001, 1000.,  0.1);

    to->params()->endParameterGroup();

}

void RandomTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->m_alpha
        || p == p_->m_fractal
        || p == p_->m_mask
        || p == p_->m_noise
        || p == p_->m_rotate)
        requestReinitGl();

}

void RandomTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void RandomTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    int nmode = p_->m_noise->baseValue();
    int fmode = p_->m_fractal->baseValue();
    bool mask = p_->m_mask->baseValue();

    p_->p_voro_cell->setVisible(nmode == 2);
    p_->p_voro_smooth->setVisible(nmode == 2);
    p_->p_rnd_rot->setVisible(p_->m_rotate->baseValue());
    p_->p_amp_mul->setVisible(fmode == 1 || fmode == 2);
    p_->p_size_add->setVisible(fmode > 0);
    p_->p_steps->setVisible(fmode > 0);
    p_->p_range_min->setVisible(mask);
    p_->p_range_max->setVisible(mask);
    p_->p_thresh->setVisible(mask);
}




void RandomTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/random.frag");

        src.pasteDefaultIncludes();

        src.addDefine(QString("#define RANDOM_ROTATE %1").arg(m_rotate->baseValue()));
        src.addDefine(QString("#define NOISE_FUNC %1").arg(m_noise->baseValue()));
        src.addDefine(QString("#define MASK %1").arg(m_mask->baseValue()));
        src.addDefine(QString("#define USE_ALPHA %1").arg(m_alpha->baseValue()));
        src.addDefine(QString("#define FRACTAL_MODE %1").arg(m_fractal->baseValue()));
    }
    catch (Exception& )
    {
        // XXX Signal to gui
        throw;
    }

    auto shader = to->createShaderQuad(src)->shader();

    // uniforms

    u_color = shader->getUniform("u_color", false);
    u_hsv = shader->getUniform("u_hsv", false);
    u_gamma_exp = shader->getUniform("u_gamma_exp", false);

    u_start_seed = shader->getUniform("u_start_seed", false);
    u_scale = shader->getUniform("u_scale", false);
    u_amp = shader->getUniform("u_amp", false);
    u_rnd_rotate = shader->getUniform("u_rnd_rotate", false);
    u_voro = shader->getUniform("u_voro", false);
    u_mask = shader->getUniform("u_mask", false);
    u_max_steps = shader->getUniform("u_max_steps", false);

}

void RandomTO::Private::releaseGl()
{

}

void RandomTO::Private::renderGl(const GL::RenderSettings& , const RenderTime& time)
{
    // update uniforms

    if (u_color)
    {
        float b = p_bright->value(time);
        u_color->setFloats( p_r->value(time) * b,
                            p_g->value(time) * b,
                            p_b->value(time) * b,
                            p_a->value(time));
    }

    if (u_hsv)
        u_hsv->setFloats(   p_hue->value(time),
                            p_sat->value(time),
                            p_bright->value(time));

    if (u_gamma_exp)
    {
        Double g = p_gamma->value(time);
        u_gamma_exp->setFloats( 1./(g*p_gamma_r->value(time)),
                                1./(g*p_gamma_g->value(time)),
                                1./(g*p_gamma_b->value(time)));
    }

    if (u_start_seed)
    {
        int seed = p_seed->value(time);
        u_start_seed->setFloats(seed % 101, (seed / 101) % 101, seed / 101 / 101);
    }

    if (u_scale)
        u_scale->setFloats(p_size->value(time),
                           p_size_add->value(time));

    if (u_amp)
        u_amp->setFloats(p_amp_mul->value(time));
    if (u_rnd_rotate)
        u_rnd_rotate->setFloats(p_rnd_rot->value(time) / 180. * PI);
    if (u_voro)
        u_voro->setFloats(p_voro_cell->value(time),
                          p_voro_smooth->value(time));
    if (u_mask)
        u_mask->setFloats(p_range_min->value(time),
                          p_range_max->value(time),
                          p_thresh->value(time));
    if (u_max_steps)
        u_max_steps->ints[0] = p_steps->value(time);

    to->renderShaderQuad(time);
}


} // namespace MO
