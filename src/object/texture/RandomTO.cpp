/** @file randomto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.05.2015</p>
*/

#include "RandomTO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "gl/ScreenQuad.h"
#include "gl/Shader.h"
#include "gl/ShaderSource.h"
#include "math/constants.h"
#include "io/DataStream.h"
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
            * p_smooth,
            * p_range_min,
            * p_range_max,
            * p_thresh,
            * p_recursive_x,
            * p_recursive_y,
            * p_recursive_z,
            * p_recursive_w,
            * p_shapeSize,
            * p_rndShapePos,
            * p_rndShapeSize,
            * p_turb_x,
            * p_turb_y
            ;
    ParameterInt
            * p_seed,
            * p_steps,
            * p_turbSteps;
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
            * u_smooth,
            * u_mask,
            * u_max_steps,
            * u_recursive,
            * u_shape,
            * u_turb;

};


RandomTO::RandomTO()
    : TextureObjectBase ()
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

        p_turbSteps = to->params()->createIntParameter(
            "turbulence_steps", tr("turbulence steps"),
            tr("Number of iterations of perturbations of coordinate, 0 = off"),
            0, 0, 30, 1, true, false);
        p_turb_x = to->params()->createFloatParameter(
                "turbulence_x", tr("turbulence amount"),
                tr("Perturbation of coordinate per iteration"), 0.1,  0.01);
        p_turb_y = to->params()->createFloatParameter(
                "turbulence_y", tr("turbulence scale"),
                tr("Scale of perturbation of coordinate per iteration"), 3.,  0.1);

        m_noise = to->params()->createSelectParameter(
                    "noise_type", tr("noise type"),
                    tr("The type of basis noise function"),
                    { "rect", "perlin", "iqvoronoise", "circle" },
                    { tr("rectangular"), tr("rectangular noise"), tr("voronoi noise"),
                      tr("circles") },
                    { tr("Hard-edged rectangular sections"),
                      tr("A typical Perlin-like noise function"),
                      tr("Voronoi noise by Iñigo Quilez"),
                      tr("Randomly displaced circles") },
                    { P_RECT, P_PERLIN, P_VORONOISE, P_CIRCLE },
                    P_PERLIN,
                    true, false);

        p_seed = to->params()->createIntParameter(
                    "seed", tr("seed"),
                    tr("Random seed"), 0, true, true);

        p_voro_cell = to->params()->createFloatParameter(
                    "voro_cell", tr("voronoicity"),
                    tr("Mix between rectangular and voronoi shapes"), 1.0,  0.0, 1.,  0.05);

        p_smooth = to->params()->createFloatParameter(
                    "smoothness", tr("smoothness"),
                    tr("Mix between sharp-edged and smooth"), 1.0,  0.0, 1.,  0.05);
        p_smooth->addSynonymId("voro_smooth");

        p_shapeSize = to->params()->createFloatParameter(
                    "shape_size", tr("shape size"),
                    tr("Size (multiplier) of the shape"),
                    1., 0., 1., 0.05);
        p_rndShapeSize = to->params()->createFloatParameter(
                    "rnd_shape_size", tr("random shape size"),
                    tr("Randomness of the shape size"),
                    .5, 0., 1., 0.05);
        p_rndShapePos = to->params()->createFloatParameter(
                    "rnd_shape_pos", tr("random shape position"),
                    tr("Randomness of the shape position"),
                    .5, 0., 1., 0.05);

        m_fractal = to->params()->createSelectParameter(
                    "fractal_type", tr("combination type"),
                    tr("The way the different noise layers are combined"),
                    { "0", "avg", "max", "rec" },//, "rnd" },
                    { tr("off"), tr("average"), tr("maximum"), tr("recursive") },//, tr("random") },
                    { tr("One noise layer is produced"),
                      tr("Serveral layers of noise are combined by averaging"),
                      tr("The maximum value of all noise layers is used"),
                      tr("Each noise layer uses the previous noise as position modifier")
                        }, //tr("The position is randomly modified in each layer") },
                    { FM_SINGLE, FM_AVERAGE, FM_MAX, FM_RECURSIVE },//, FM_RANDOM },
                    FM_AVERAGE,
                    true, false);

        p_steps = to->params()->createIntParameter(
                    "steps", tr("number layers"),
                    tr("The maximum number of noise layers to combine"), 4, true, true);
        p_steps->setMinValue(1);

        p_amp_mul = to->params()->createFloatParameter(
                    "amp_mul", tr("amplitude per layer"),
                    tr("Multiplier of the amplitude per layer"), 0.8,  0.05);

        p_size = to->params()->createFloatParameter(
                    "size", tr("scale"), tr("The zoom-out factor"), 10.0,  0.1);
        p_size_add = to->params()->createFloatParameter(
                    "size_add", tr("scale per layer"),
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

        p_recursive_w = to->params()->createFloatParameter(
                    "recursive_w", tr("recursion"),
                    tr("Influence of all noise values to the next layer's position"), 1.,  0.05);
        p_recursive_x = to->params()->createFloatParameter(
                    "recursive_x", tr("recursion x"),
                    tr("Influence of x value to the next layer's position"), 1.,  0.05);
        p_recursive_y = to->params()->createFloatParameter(
                    "recursive_y", tr("recursion y"),
                    tr("Influence of y value to the next layer's position"), 1.,  0.05);
        p_recursive_z = to->params()->createFloatParameter(
                    "recursive_z", tr("recursion z"),
                    tr("Influence of z value to the next layer's position"), 1.,  0.05);

        m_mask = to->params()->createBooleanParameter(
                    "mask", tr("value range"),
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
        || p == p_->p_turbSteps
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
    bool turb = p_->p_turbSteps->baseValue() > 0;

    p_->p_turb_x->setVisible(turb);
    p_->p_turb_y->setVisible(turb);
    p_->p_voro_cell->setVisible(nmode == P_VORONOISE);
    p_->p_smooth->setVisible(nmode == P_VORONOISE || nmode == P_CIRCLE);
    p_->p_rnd_rot->setVisible(p_->m_rotate->baseValue());
    p_->p_amp_mul->setVisible(fmode == FM_AVERAGE || fmode == FM_MAX);
    p_->p_size_add->setVisible(fmode > 0 && fmode != FM_RANDOM);
    p_->p_steps->setVisible(fmode > 0);
    p_->p_range_min->setVisible(mask);
    p_->p_range_max->setVisible(mask);
    p_->p_thresh->setVisible(mask);
    bool fmoder = fmode == FM_RECURSIVE || fmode == FM_RANDOM;
    p_->p_recursive_x->setVisible(fmoder);
    p_->p_recursive_y->setVisible(fmoder);
    p_->p_recursive_z->setVisible(fmoder);
    p_->p_recursive_w->setVisible(fmoder);
    p_->p_shapeSize->setVisible(nmode == P_CIRCLE);
    p_->p_rndShapeSize->setVisible(nmode == P_CIRCLE);
    p_->p_rndShapePos->setVisible(nmode == P_CIRCLE);
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
        src.addDefine(QString("#define NUM_TURB_STEPS %1").arg(p_turbSteps->baseValue()));
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
    u_smooth = shader->getUniform("u_smooth", false);
    u_shape = shader->getUniform("u_shape", false);
    u_mask = shader->getUniform("u_mask", false);
    u_max_steps = shader->getUniform("u_max_steps", false);
    u_recursive = shader->getUniform("u_recursive", false);
    u_turb = shader->getUniform("u_turb", false);
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
        u_voro->setFloats(p_voro_cell->value(time));
    if (u_smooth)
        u_smooth->setFloats(p_smooth->value(time));
    if (u_shape)
        u_shape->setFloats(p_shapeSize->value(time),
                           p_rndShapeSize->value(time),
                           p_rndShapePos->value(time));
    if (u_mask)
        u_mask->setFloats(p_range_min->value(time),
                          p_range_max->value(time),
                          p_thresh->value(time));
    if (u_max_steps)
        u_max_steps->ints[0] = p_steps->value(time);

    if (u_recursive)
        u_recursive->setFloats(p_recursive_x->value(time),
                               p_recursive_y->value(time),
                               p_recursive_z->value(time),
                               p_recursive_w->value(time));
    if (u_turb)
        u_turb->setFloats(p_turb_x->value(time),
                          p_turb_y->value(time));

    to->renderShaderQuad(time);
}


} // namespace MO
