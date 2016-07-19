/** @file lensdistto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.05.2015</p>
*/

#include "LensDistTO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterText.h"
#include "gl/ScreenQuad.h"
#include "gl/Shader.h"
#include "gl/ShaderSource.h"
#include "math/functions.h"
#include "io/DataStream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(LensDistTO)

struct LensDistTO::Private
{
    Private(LensDistTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    LensDistTO * to;

    ParameterSelect
            * m_lens, * m_chroma;
    ParameterFloat
            * p_dist, * p_dist_chroma, * p_zoom,
            * p_exp, * p_norm_radius,
            * p_center_x, * p_center_y,
            * p_spec1, * p_spec2, * p_sat;
    ParameterInt
            * p_num;
    ParameterText
            * p_user_code;

    GL::Uniform
            * u_lens, * u_lens_set, * u_lens_center,
            * u_spec, * u_num_samples;
};


LensDistTO::LensDistTO()
    : TextureObjectBase ()
    , p_                (new Private(this))
{
    setName("LensDistortion");
    initMaximumTextureInputs(1);
}

LensDistTO::~LensDistTO()
{
    delete p_;
}

void LensDistTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("tolens", 1);
}

void LensDistTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("tolens", 1);
}

void LensDistTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void LensDistTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void LensDistTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void LensDistTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void LensDistTO::Private::createParameters()
{
    to->params()->beginParameterGroup("lens", tr("lens distortion"));
    to->initParameterGroupExpanded("lens");

        m_lens = to->params()->createSelectParameter("mode", tr("lens type"),
                    tr("Selects the type of lens function"),
        { "q", "exp", "user" },
        { tr("quadratic"), tr("exponential"), tr("user defined") },
        { tr("A quadratic term is used to create a typical barrel/pincushion distortion"),
          tr("A completely unrealistic exponential term with adjustable exponent"),
          tr("A distortion function can be specified in GLSL code") },
        { 0, 1, 2 },
        0, true, false);

        p_dist = to->params()->createFloatParameter("amt", tr("amount"),
                            tr("Amount of lens distortion, positive or negative"),
                            0.05,  0.01);

        p_user_code = to->params()->createTextParameter("user_code", tr("distortion function"),
                        tr("A piece of glsl code to distort the image coordinates"),
                        TT_GLSL,
                        "// uv and f are in range [-1,1]\n"
                        "vec2 lens_distortion(in vec2 uv, float f)\n{\n"
                        "\treturn uv + f * uv * dot(uv, uv);\n}\n"
                        , true, false);

        p_exp = to->params()->createFloatParameter("exp", tr("exponent"),
                            tr("The exponent for the exponential distortion function"),
                            2.0,  0.05);
        p_exp->setMinValue(0.00001);
        p_zoom = to->params()->createFloatParameter("zoom", tr("zoom"),
                            tr("Scale of the image"),
                            1.0,  0.01);
        p_zoom->setMinValue(0.00001);
        p_norm_radius = to->params()->createFloatParameter("focus_radius", tr("focus radius"),
                            tr("The radius (or angle of incident) where the lens is undistorted"),
                            0.0,  0.05);
        p_center_x = to->params()->createFloatParameter("center_x", tr("lens x"),
                            tr("Center of the lens in range [-1,1]"),
                            0.0,  0.05);
        p_center_y = to->params()->createFloatParameter("center_y", tr("lens y"),
                            tr("Center of the lens in range [-1,1]"),
                            0.0,  0.05);

        m_chroma = to->params()->createBooleanParameter("use_chroma", tr("chromatic abberation"),
                            tr("Simulates the chromatic abberation effect known from photography"),
                            tr("Off"),
                            tr("The input image is multi-sampled and each color is distorted differently"),
                            false,
                            true, false);
        p_dist_chroma = to->params()->createFloatParameter("amt_chroma", tr("chroma shift"),
                            tr("Amount of lens distortion depending on spectral component"),
                            0.07,  0.01);
        p_num = to->params()->createIntParameter("num_samples", tr("number of samples"),
                            tr("The number of samples to use for the chromatic shift - should at least be three"),
                            10, 1, 1000, 1, true, true);
        p_sat = to->params()->createFloatParameter("sat", tr("saturation"),
                            tr("Saturation of the chromatic abberation effect"),
                            1.0,  0., 1.,  0.01);
        p_spec1 = to->params()->createFloatParameter("spec1", tr("spectral shift 1"),
                            tr("A modifier to the spectral equation that splits the color"),
                            0.0,  0.01);
        p_spec2 = to->params()->createFloatParameter("spec2", tr("spectral shift 2"),
                            tr("A modifier to the spectral equation that splits the color"),
                            0.0,  0.01);

    to->params()->endParameterGroup();
}

void LensDistTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->m_lens
        || p == p_->m_chroma
        || p == p_->p_user_code)
        requestReinitGl();

}

void LensDistTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void LensDistTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    const bool
            chroma = p_->m_chroma->baseValue(),
            exp = p_->m_lens->baseValue() == 1,
            user = p_->m_lens->baseValue() == 2;

    p_->p_dist_chroma->setVisible(chroma);
    p_->p_num->setVisible(chroma);
    p_->p_spec1->setVisible(chroma);
    p_->p_spec2->setVisible(chroma);
    p_->p_sat->setVisible(chroma);

    p_->p_exp->setVisible(exp);
    p_->p_user_code->setVisible(user);
}




void LensDistTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/lens.frag");
        src.pasteDefaultIncludes();
        src.addDefine(QString("#define LENS_MODE %1").arg(m_lens->baseValue()));
        src.addDefine(QString("#define CHROMATIC %1").arg(m_chroma->baseValue()));

        if (m_lens->baseValue() == 2)
        {
            src.replace("//%user_code%", p_user_code->baseValue(), true);
            src.pasteDefaultIncludes();
        }
    }
    catch (Exception& )
    {
        // XXX Signal to gui
        throw;
    }

    auto shader = to->createShaderQuad(
                src, { "u_tex" })->shader();

    // uniforms

    u_lens = shader->getUniform("u_lens", true);
    u_lens_set = shader->getUniform("u_lens_set", false);
    u_lens_center = shader->getUniform("u_lens_center", false);
    u_spec = shader->getUniform("u_spec", false);
    u_num_samples = shader->getUniform("u_num_samples", false);
}

void LensDistTO::Private::releaseGl()
{

}

void LensDistTO::Private::renderGl(const GL::RenderSettings& , const RenderTime& time)
{
    // update uniforms

    if (u_lens)
        u_lens->setFloats(  p_dist->value(time),
                            p_dist_chroma->value(time),
                            1.f / p_zoom->value(time));

    if (u_lens_set)
        u_lens_set->setFloats(
                            p_exp->value(time),
                            p_norm_radius->value(time));

    if (u_lens_center)
        u_lens_center->setFloats(
                            p_center_x->value(time),
                            p_center_y->value(time));

    if (u_spec)
        u_spec->setFloats(  p_spec1->value(time),
                            p_spec2->value(time),
                            p_sat->value(time));

    if (u_num_samples)
        u_num_samples->ints[0] = p_num->value(time);

    to->renderShaderQuad(time);
}


} // namespace MO
