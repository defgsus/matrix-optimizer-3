/** @file texture3dGento.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2015</p>
*/

#include "texture3dgento.h"
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

//MO_REGISTER_OBJECT(Texture3dGenTO)

struct Texture3dGenTO::Private
{
    Private(Texture3dGenTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

    Texture3dGenTO * to;

    GL::Uniform * u_depth_slice;

    ParameterInt
            * p_width,
            * p_height,
            * p_depth;
    ParameterText
            * p_glsl;
};


Texture3dGenTO::Texture3dGenTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Texture3D-Generator");
}

Texture3dGenTO::~Texture3dGenTO()
{
    delete p_;
}

void Texture3dGenTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("to3dgen", 1);
}

void Texture3dGenTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("to3dgen", 1);
}

void Texture3dGenTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void Texture3dGenTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void Texture3dGenTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void Texture3dGenTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
}


void Texture3dGenTO::Private::createParameters()
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

void Texture3dGenTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_width
            || p == p_->p_height
            || p == p_->p_depth
            || p == p_->p_glsl)
        requestReinitGl();
}

void Texture3dGenTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void Texture3dGenTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();
}




void Texture3dGenTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    src.loadVertexSource(":/shader/to/gen3d.vert");
    src.loadFragmentSource(":/shader/to/gen3d.frag");
    src.pasteDefaultIncludes();

    auto shader = to->createShaderQuad(src)->shader();

    // uniforms

    u_depth_slice = shader->getUniform("u_depth_slice");
}

void Texture3dGenTO::Private::releaseGl()
{

}

void Texture3dGenTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
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
