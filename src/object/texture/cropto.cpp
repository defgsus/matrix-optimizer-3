/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/10/2016</p>
*/

#include "cropto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(CropTO)

struct CropTO::Private
{
    Private(CropTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    CropTO * to;

    ParameterFloat
            *p_off_x, *p_off_y,
            *p_scale_x, *p_scale_y;
    GL::Uniform
            * u_tex_scale;
};


CropTO::CropTO()
    : TextureObjectBase ()
    , p_                (new Private(this))
{
    setName("Crop");
    initMaximumTextureInputs(1);
    initEnableColorRange(true);
}

CropTO::~CropTO()
{
    delete p_;
}

void CropTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texcrop", 1);
}

void CropTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("texcrop", 1);
}

void CropTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void CropTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void CropTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void CropTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void CropTO::Private::createParameters()
{
    to->params()->beginParameterGroup("crop", tr("crop"));
    to->initParameterGroupExpanded("crop");

        p_off_x = to->params()->createFloatParameter(
                    "offset_x", tr("offset x"),
                    tr("Offset of crop area on x axis [0,1]"),
                    0.0,  0.01);
        p_off_y = to->params()->createFloatParameter(
                    "offset_y", tr("offset y"),
                    tr("Offset of crop area on y axis [0,1]"),
                    0.0,  0.01);

        p_scale_x = to->params()->createFloatParameter(
                    "scale_x", tr("scale x"),
                    tr("Scale of crop area on x axis [0,1]"),
                    0.5,  0.01);

        p_scale_y = to->params()->createFloatParameter(
                    "scale_y", tr("scale y"),
                    tr("Scale of crop area on y axis [0,1]"),
                    0.5,  0.01);

    to->params()->endParameterGroup();
}

void CropTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    /*if (p == p_->p_invert)
        requestReinitGl();*/

}

void CropTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void CropTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();


}




void CropTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    src.loadVertexSource(":/shader/to/default.vert");
    src.loadFragmentSource(":/shader/to/crop.frag");

    auto shader = to->createShaderQuad(
                src, { "u_tex" })->shader();

    // uniforms

    u_tex_scale = shader->getUniform("u_tex_scale", true);
}

void CropTO::Private::releaseGl()
{

}

void CropTO::Private::renderGl(const GL::RenderSettings& , const RenderTime& time)
{
    // update uniforms

    u_tex_scale->setFloats( p_scale_x->value(time),
                            p_scale_y->value(time),
                            p_off_x->value(time),
                            p_off_y->value(time));

    to->renderShaderQuad(time);
}


} // namespace MO
