/** @file normalmapto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 14.05.2015</p>
*/

#include "NormalMapTO.h"
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

MO_REGISTER_OBJECT(NormalMapTO)

struct NormalMapTO::Private
{
    Private(NormalMapTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    NormalMapTO * to;

    ParameterFloat
            * p_r, * p_g, * p_b, * p_a,
            * p_eps, *p_eps2;
    ParameterSelect
            * m_kernel,
            * m_signed,
            * m_mul_alpha;

    GL::Uniform
            * u_color,
            * u_eps;

};


NormalMapTO::NormalMapTO()
    : TextureObjectBase ()
    , p_                (new Private(this))
{
    setName("NormalMap");
    initDefaultUpdateMode(UM_ON_CHANGE);
    initMaximumTextureInputs(1);
}

NormalMapTO::~NormalMapTO()
{
    delete p_;
}

void NormalMapTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("tonrm", 1);
}

void NormalMapTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("tonrm", 1);
}

void NormalMapTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void NormalMapTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void NormalMapTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void NormalMapTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void NormalMapTO::Private::createParameters()
{
    to->params()->beginParameterGroup("normal", tr("normal estimation"));
    to->initParameterGroupExpanded("normal");

        m_kernel = to->params()->createSelectParameter(
                    "kernel_type", tr("kernel size"),
                    tr("The size of the interpolation kernel"),
                    { "0", "1", "2", "3" },
                    { tr("none"), "3x3", "5x5", "7x7" },
                    { tr("A single texel is considered"),
                      "", "", "" },
                    { 0, 1, 2, 3 },
                    1,
                    true, false);

        p_eps = to->params()->createFloatParameter(
                    "eps", tr("radius"),
                    tr("Radius for derivative estimation"), 0.1,  0.0, 1.,  .01, true, true);

        p_eps2 = to->params()->createFloatParameter(
                    "eps2", tr("kernel radius"),
                    tr("Radius for kernel interpolation"), 1.,  0.0, 1.,  .01, true, true);

        p_r = to->params()->createFloatParameter("red", tr("red"), tr("Red contribution to bump value"), 1.0, 0.1);
        p_g = to->params()->createFloatParameter("green", tr("green"), tr("Green contribution to bump value"), 1.0, 0.1);
        p_b = to->params()->createFloatParameter("blue", tr("blue"), tr("Blue contribution to bump value"), 1.0, 0.1);
        p_a = to->params()->createFloatParameter("alpha", tr("alpha"), tr("Alpha contribution to bump value"), 1.0, 0.1);
        p_a->setDefaultEvolvable(false);

        m_mul_alpha = to->params()->createBooleanParameter(
                    "mul_alpha", tr("multiply alpha"),
                    tr("Use the alpha channel to scale the colors"),
                    tr("Alpha is considered as a normal channel"),
                    tr("The RGB values are multiplied with the alpha channel"),
                    true,
                    true, false);

        m_signed = to->params()->createBooleanParameter(
                    "signed", tr("signed output"),
                    tr("Allows the normalmap texture in the negative range as well"),
                    tr("Texture values are in the range [0,1]"),
                    tr("Texture values are in the range [-1,1] - does not work with 8-bit texture channels!"),
                    true,
                    true, false);

    to->params()->endParameterGroup();

}

void NormalMapTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->m_signed
        || p == p_->m_kernel)
        requestReinitGl();

}

void NormalMapTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void NormalMapTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    int kmode = p_->m_kernel->baseValue();

    p_->p_eps2->setVisible(kmode > 0);
}




void NormalMapTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/derivative.frag");

        src.pasteDefaultIncludes();

        src.addDefine(QString("#define KERNEL %1").arg(m_kernel->baseValue()));
        src.addDefine(QString("#define SIGNED %1").arg(m_signed->baseValue()));
        src.addDefine(QString("#define MUL_ALPHA %1").arg(m_mul_alpha->baseValue()));
    }
    catch (Exception& e)
    {
        to->setErrorMessage(e.what());
        throw;
    }

    auto shader = to->createShaderQuad(src, { "u_tex" })->shader();

    // uniforms

    u_color = shader->getUniform("u_color", false);
    u_eps = shader->getUniform("u_eps", false);
}

void NormalMapTO::Private::releaseGl()
{

}

void NormalMapTO::Private::renderGl(const GL::RenderSettings& , const RenderTime& time)
{
    // update uniforms

    if (u_color)
    {
        u_color->setFloats( p_r->value(time),
                            p_g->value(time),
                            p_b->value(time),
                            p_a->value(time));
    }

    if (u_eps)
    {
        Float eps = std::max(0.000001, std::pow(p_eps->value(time), 2.) / 3. ),
              eps2 = std::pow(p_eps2->value(time), 2.);
        u_eps->setFloats( eps, eps * eps2 );
    }

    to->renderShaderQuad(time);
}


} // namespace MO
