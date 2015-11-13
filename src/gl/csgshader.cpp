/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#include "csgshader.h"
#include "types/properties.h"
#include "math/csgbase.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GL {

struct CsgShader::Private
{
    Private(CsgShader* p)
        : p             (p)
        , root          (0)
        , quad          (0)
        , doRecompile   (true)
    { }

    void createProps();
    void createShaderSource(GL::ShaderSource*) const;
    void initQuad();
    void releaseQuad();
    void updateUniforms(const QSize& res, const Mat4& proj, const Mat4& trans, Float time);

    CsgShader * p;
    MO::Properties props;
    const CsgRoot * root;
    GL::ScreenQuad * quad;
    bool doRecompile;

    GL::Uniform * u_resolution,
                * u_transformation,        // camera
                * u_projection,
                * u_fudge,                 // ray step precision
                * u_epsilon,
                * u_time,
                * u_max_trace_dist;

};

CsgShader::CsgShader()
    : p_        (new Private(this))
{
    p_->createProps();
}

CsgShader::~CsgShader()
{
    delete p_;
}

const MO::Properties& CsgShader::properties() const
{
    return p_->props;
}

void CsgShader::setProperties(const MO::Properties& p)
{
    p_->props = p;
    p_->doRecompile = true;
}

void CsgShader::Private::createProps()
{
    props.set("fudge", QObject::tr("fudge factor"),
              QObject::tr("The step multiplier of each ray step, range (0, 1]"),
              1., 0.0001, 1., 0.01);
    props.set("epsilon", QObject::tr("normal estimation size"),
              QObject::tr("The width in space units of the normal estimation"),
              0.01);
    props.setMin("epsilon", 0.000001);
    props.setStep("epsilon", 0.001);
    props.set("max_dist", QObject::tr("maxmimum distance"),
              QObject::tr("The overall maximum distance a ray can travel, including reflections"),
              100.);
    props.setMin("max_dist", 0.);
    props.set("max_ray_dist", QObject::tr("maximum ray length"),
              QObject::tr("The maximum distance that one ray can travel"),
              100.);
    props.setMin("max_ray_dist", 0.);
    props.set("max_ray_steps", QObject::tr("maximum ray steps"),
              QObject::tr("The maximum number of steps that one ray is allowed to do"),
              100u);
    props.setMin("max_ray_steps", 1u);
    props.set("max_reflect", QObject::tr("maximum reflections"),
              QObject::tr("The maximum number of reflections allowed"),
              0u, 0u, 100u);
    /*
    props.set("", QObject::tr(""),
              QObject::tr(""),
              );
    */
}

void CsgShader::Private::createShaderSource(GL::ShaderSource* src) const
{
    QString code;
    if (root)
    {
        code = root->toGlsl("DE_scene") + "\n";
        MO_PRINT(code);
        code.replace("MAX_DIST", "u_max_trace_dist.y");
    }
    else
        code = "float DE_scene(in vec3 pos) { return length(pos) - 1.; }\n\n";

    src->loadVertexSource(":/shader/csgshader.vert");
    src->loadFragmentSource(":/shader/csgshader.frag");

    src->addDefine(QString("#define MAX_TRACE_STEPS %1").arg(props.get("max_ray_steps").toInt()));
    src->addDefine(QString("#define MAX_REFLECTIONS %1").arg(props.get("max_reflections").toInt()));

    src->replace("//%dist_func%", code);

    src->pasteDefaultIncludes();
}

void CsgShader::Private::initQuad()
{
    if (!quad)
    {
        quad = new GL::ScreenQuad("csg_shader_quad");
        doRecompile = true;
    }

    auto src = new GL::ShaderSource;
    createShaderSource(src);

    if (quad->isCreated())
        quad->release();

    try
    {
        quad->create(src);
    }
    catch (Exception&)
    {
        delete quad;
        quad = 0;
        throw;
    }

    // -- get uniforms --

    u_resolution = quad->shader()->getUniform("u_resolution");
    u_projection = quad->shader()->getUniform("u_inverse_frustum");
    if (u_projection)
        u_projection->setAutoSend(true);
    u_transformation = quad->shader()->getUniform("u_vtmatrix");
    if (u_transformation)
        u_transformation->setAutoSend(true);
    u_fudge = quad->shader()->getUniform("u_fudge");
    u_epsilon = quad->shader()->getUniform("u_epsilon");
    u_time = quad->shader()->getUniform("u_time");
    u_max_trace_dist = quad->shader()->getUniform("u_max_trace_dist");

    doRecompile = false;
}

void CsgShader::Private::releaseQuad()
{
    if (quad && quad->isCreated())
        quad->release();
    delete quad;
    quad = 0;
}

void CsgShader::Private::updateUniforms(const QSize& res, const Mat4& proj, const Mat4& trans, Float time)
{
    if (u_fudge)
        u_fudge->floats[0] = props.get("fudge").toFloat();

    if (u_epsilon)
        u_epsilon->floats[0] = props.get("epsilon").toFloat();

    if (u_max_trace_dist)
        u_max_trace_dist->setFloats(
                    props.get("max_ray_dist").toFloat(),
                    props.get("max_dist").toFloat() );

    if (u_resolution)
        u_resolution->setFloats(res.width(), res.height(),
                                1. / std::max(1, res.width()),
                                1. / std::max(1, res.height()));

    if (u_transformation)
        u_transformation->set(trans);

    if (u_projection)
        u_projection->set(glm::inverse(proj));

    if (u_time)
        u_time->floats[0] = time;
}

void CsgShader::render(const QSize &resolution, const Mat4& projection, const Mat4& transform)
{
    if (p_->doRecompile)
        p_->initQuad();

    if (!p_->quad)
        return;

    p_->updateUniforms(resolution, projection, transform, 0.f);

    p_->quad->draw(resolution.width(), resolution.height());
}

void CsgShader::setRootObject(const CsgRoot* r)
{
    p_->root = r;
    p_->doRecompile = true;
}

GL::ShaderSource CsgShader::getShaderSource() const
{
    GL::ShaderSource src;
    p_->createShaderSource(&src);
    return src;
}

void CsgShader::createGl()
{
    try
    {
        p_->initQuad();
    }
    catch (Exception&)
    {
        p_->releaseQuad();
        throw;
    }
}

void CsgShader::releaseGl()
{
    p_->releaseQuad();
}

} // namespace GL
} // namespace MO
