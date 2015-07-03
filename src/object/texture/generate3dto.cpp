/** @file Generate3dTO.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2015</p>
*/

#ifndef MO_DISABLE_EXP

#include "generate3dto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(Generate3dTO)

struct Generate3dTO::Private
{
    Private(Generate3dTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

    Generate3dTO * to;

    GL::FrameBufferObject * fbo;
    GL::Uniform * u_depth_slice;

    ParameterInt
            * p_width,
            * p_height,
            * p_depth;
    ParameterText
            * p_glsl;
};


Generate3dTO::Generate3dTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Generator3d");
    init3dFramebuffer(32);
}

Generate3dTO::~Generate3dTO()
{
    delete p_;
}

void Generate3dTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("togen3d", 1);
}

void Generate3dTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("togen3d", 1);
}

void Generate3dTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void Generate3dTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void Generate3dTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void Generate3dTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
}


void Generate3dTO::Private::createParameters()
{
    to->params()->beginParameterGroup("tex3d", tr("Gen-surface texture"));
    to->initParameterGroupExpanded("tex3d");

        p_width = to->params()->createIntParameter(
                    "width", tr("width"), tr("Width of the 3d-texture in pixels"), 64, true, false);
        p_width->setMinValue(8);
        p_width->setSmallStep(16);

        p_height = to->params()->createIntParameter(
                    "height", tr("height"), tr("Height of the 3d-texture in pixels"), 64, true, false);
        p_height->setMinValue(8);
        p_height->setSmallStep(16);

        p_depth = to->params()->createIntParameter(
                    "depth", tr("depth"), tr("Depth of the 3d-texture in pixels"), 64, true, false);
        p_depth->setMinValue(1);
        p_depth->setSmallStep(16);

        p_glsl = to->params()->createTextParameter(
                    "glsl_fragment",
                    tr("GLSL source"),
                    tr("The GLSL source to generate the content of the 3d-texture"),
                    TT_GLSL,
                    "// " + tr("input is position in 3d-texture, range [-1,1]") + "\n"
                    "// " + tr("output is color for each pixel") + "\n"
                    "vec4 texture_pixel(in vec3 pos)\n"
                    "{\n"
                    "\treturn vec4( 1. - length(pos) / sqrt(3.) );\n"
                    "}\n"
                    , true, false);

    to->params()->endParameterGroup();
}

void Generate3dTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_width
            || p == p_->p_height
            || p == p_->p_depth
            || p == p_->p_glsl)
        requestReinitGl();
}

void Generate3dTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void Generate3dTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();
}




void Generate3dTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    src.loadVertexSource(":/shader/to/default.vert");
    src.loadFragmentSource(":/shader/to/gen3d.frag");
    src.pasteDefaultIncludes();

    src.replace("//%mo_user_code%", p_glsl->baseValue(), true);

    auto shader = to->createShaderQuad(src)->shader();

    // uniforms

    u_depth_slice = shader->getUniform("u_depth_slice");
}

void Generate3dTO::Private::releaseGl()
{

}

void Generate3dTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    int depth = p_depth->baseValue();

    // for each depth-slice
    for (int i = 0; i < depth; ++i)
    {
        // update uniforms
        if (u_depth_slice)
            u_depth_slice->ints[0] = i;

        to->renderShaderQuad(time, thread);
    }
}


} // namespace MO

#endif // #ifndef MO_DISABLE_EXP
