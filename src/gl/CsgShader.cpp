/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#include "CsgShader.h"
#include "types/Properties.h"
#include "math/CsgBase.h"
#include "gl/ScreenQuad.h"
#include "gl/Shader.h"
#include "gl/ShaderSource.h"
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
      //          * u_projection,
                * u_fudge,                 // ray step precision
                * u_epsilon,
                * u_time,
                * u_max_trace_dist,
                * u_scale,
                * u_frequency;

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

CsgShader::RenderMode CsgShader::renderMode() const
{
    return (RenderMode)p_->props.get("render_mode").toInt();
}

void CsgShader::setProperties(const MO::Properties& p)
{
    p_->props = p;
    // XXX Could be more selective
    p_->doRecompile = true;
}

void CsgShader::Private::createProps()
{
    MO::Properties::NamedValues rmodes;
    rmodes.set("flat", QObject::tr("flat"),
               QObject::tr("Slice through distance-field"), RM_FLAT);
    rmodes.set("solid", QObject::tr("solid"),
               QObject::tr("Solid non-transparent raymarching"), RM_SOLID);
    props.set("render_mode", QObject::tr("render mode"),
              QObject::tr("Selects one of different rendering modes"),
              rmodes, (int)RM_SOLID);

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

    props.set("scale", QObject::tr("slice scale"),
              QObject::tr("Scale of the slice"),
              5., .1);
    props.set("frequency", QObject::tr("frequency"),
              QObject::tr("Frequency of the distance color indicator"),
              1., .1);

    props.setUpdateVisibilityCallback([](MO::Properties& p)
    {
        const RenderMode rm = (RenderMode)p.get("render_mode").toInt();
        const bool raym = (rm == RM_SOLID),
                   flat = (rm == RM_FLAT);
        p.setVisible("fudge", raym);
        p.setVisible("epsilon", raym);
        p.setVisible("max_dist", raym);
        p.setVisible("max_ray_dist", raym);
        p.setVisible("max_ray_steps", raym);
        p.setVisible("max_reflect", raym);
        p.setVisible("scale", flat);
        p.setVisible("frequency", flat);
    });
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
    src->addDefine(QString("#define RENDER_MODE %1").arg(props.get("render_mode").toInt()));

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
    //u_projection = quad->shader()->getUniform("u_inverse_frustum");
    //if (u_projection)
    //    u_projection->setAutoSend(true);
    u_transformation = quad->shader()->getUniform("u_vtmatrix");
    if (u_transformation)
        u_transformation->setAutoSend(true);
    u_fudge = quad->shader()->getUniform("u_fudge");
    u_epsilon = quad->shader()->getUniform("u_epsilon");
    u_time = quad->shader()->getUniform("u_time");
    u_max_trace_dist = quad->shader()->getUniform("u_max_trace_dist");
    u_scale = quad->shader()->getUniform("u_scale");
    u_frequency = quad->shader()->getUniform("u_frequency");

    doRecompile = false;
}

void CsgShader::Private::releaseQuad()
{
    if (quad && quad->isCreated())
        quad->release();
    delete quad;
    quad = 0;
}

void CsgShader::Private::updateUniforms(const QSize& res,
                                        const Mat4& /*proj*/, const Mat4& trans, Float time)
{
    //RenderMode rm = p->renderMode();

    if (u_transformation)
        u_transformation->set(trans);

    if (u_resolution)
        u_resolution->setFloats(res.width(), res.height(),
                                1. / std::max(1, res.width()),
                                1. / std::max(1, res.height()));

    if (u_scale)
        u_scale->floats[0] = props.get("scale").toFloat();

    if (u_frequency)
        u_frequency->floats[0] = props.get("frequency").toFloat();

    if (u_fudge)
        u_fudge->floats[0] = props.get("fudge").toFloat();

    if (u_epsilon)
        u_epsilon->floats[0] = props.get("epsilon").toFloat();

    if (u_max_trace_dist)
        u_max_trace_dist->setFloats(
                    props.get("max_ray_dist").toFloat(),
                    props.get("max_dist").toFloat() );

    //if (u_projection)
    //    u_projection->set(glm::inverse(proj));

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
