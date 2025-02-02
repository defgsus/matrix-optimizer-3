/** @file blurto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#include "BlurTO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "gl/FrameBufferObject.h"
#include "gl/Texture.h"
#include "gl/ScreenQuad.h"
#include "gl/Shader.h"
#include "gl/ShaderSource.h"
#include "io/DataStream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(BlurTO)

struct BlurTO::Private
{
    Private(BlurTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    struct Stage
    {
        uint index;
        Vec2 dir;
        GL::Uniform
                * u_size_sigma,
                * u_num,
                * u_dir,
                * u_alpha,
                * u_mask_range;
    };

    BlurTO * to;
    QList<Stage> stages;

    ParameterFloat
            * p_size,
            * p_sigma,
            * p_num,
            * p_alpha,
            * p_mask_min,
            * p_mask_max,
            * p_mask_thresh;
    ParameterSelect
            * p_sizeIsPixels,
            * p_alphaMode,
            * m_mask;
};


BlurTO::BlurTO()
    : TextureObjectBase ()
    , p_                (new Private(this))
{
    setName("Blur");
    initMaximumTextureInputs( { tr("input"), tr("mask") } );
}

BlurTO::~BlurTO()
{
    delete p_;
}

void BlurTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("toblur", 1);
}

void BlurTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("toblur", 1);
}

void BlurTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void BlurTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void BlurTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void BlurTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void BlurTO::Private::createParameters()
{
    to->params()->beginParameterGroup("blur", tr("blur"));
    to->initParameterGroupExpanded("blur");

        p_size = to->params()->createFloatParameter(
                    "size", tr("size"), tr("Radius of the blur kernel"), 10.0,  1.);
        p_size->setMinValue(1.);

        p_sizeIsPixels = to->params()->createBooleanParameter(
                    "size_in_pix", tr("size in pixels"),
                    tr("Controls whether the blur size is in actual pixels or in a resolution invariant unit"),
                    tr("The size is invariant to the resolution, messured in 1000th of the image size"),
                    tr("The size is messured in pixels"),
                    false, true, true);

        p_sigma = to->params()->createFloatParameter(
                    "sigma", tr("smoothness"), tr("A modifier to adjust the amount of blur, range is about [0, 1]"),
                    .7,  0.05);
        p_sigma->setMinValue(0.);

        p_num = to->params()->createFloatParameter(
                    "num", tr("num samples"), tr("Size of the blur kernel - there will be (num*2+1)*2 samples"),
                    10.0,  1., 1000.,  1.);
        p_num->setDefaultEvolvable(false);

        m_mask = to->params()->createBooleanParameter(
                    "use_mask", tr("use blur mask"),
                    tr("Enables a texture to be used as a modifier for the blur smoothness"),
                    tr("Off"), tr("Connect a texture to the second input"),
                    false,
                    true, false);
        p_mask_min = to->params()->createFloatParameter(
                    "mask_min", tr("mask range min"), tr("The mask value for which the blurring is weakest"),
                    0.,  .01);
        p_mask_max = to->params()->createFloatParameter(
                    "mask_max", tr("mask range max"), tr("The mask value for which the blurring is strongest"),
                    1.,  .01);
        p_mask_thresh = to->params()->createFloatParameter(
                    "mask_thresh", tr("mask range threshold"), tr("A value for shifting where the blurring is strongest"),
                    0.,  .01);


        p_alphaMode = to->params()->createSelectParameter(
                    "alpha_mode", tr("alpha channel"),
                    tr("The way the alpha channel is calculated"),
        { "blur", "fix", "mag" },
        { tr("blur"), tr("fixed"), tr("magnitude") },
        { tr("The alpha channels is blurred the same way as the other channels"),
          tr("The alpha channel remains a fixed value"),
          tr("The alpha channel is set to the magnitude of the blured color channels") },
        { 0, 1, 2 },
                    1, true, true);

        p_alpha = to->params()->createFloatParameter(
                    "alpha", tr("alpha"), tr("A modifier for the output alpha channel - controls transparency"),
                    1.0,  0.05);
        p_alpha->setMinValue(0.);

    to->params()->endParameterGroup();
}

void BlurTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_alphaMode
            || p == p_->m_mask)
        requestReinitGl();
}

void BlurTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void BlurTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    bool mask = p_->m_mask->baseValue();
    p_->p_mask_min->setVisible(mask);
    p_->p_mask_max->setVisible(mask);
    p_->p_mask_thresh->setVisible(mask);
}




void BlurTO::Private::initGl()
{
    // shader-quad

    for (int i=0; i<2; ++i)
    {
        // setup a shader stage
        Stage s;
        s.index = i;
        s.dir = i == 0 ? Vec2(1, 0) : Vec2(0, 1);

        GL::ShaderSource src;
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/blur.frag");
        src.pasteDefaultIncludes();

        src.addDefine(QString("#define USE_MASK %1").arg(m_mask->baseValue()));
        if (i == 1)
            src.addDefine(QString("#define MO_ALPHA_MODE %1").arg(p_alphaMode->baseValue()));

        auto shader = to->createShaderQuad(src, { "u_tex", "u_tex_mask" })->shader();

        // uniforms

        s.u_size_sigma = shader->getUniform("u_size_sigma", true);
        s.u_num = shader->getUniform("u_num", true);
        s.u_dir = shader->getUniform("u_direction", true);
        s.u_alpha = shader->getUniform("u_alpha", true);
        s.u_mask_range = shader->getUniform("u_mask_range", false);

        stages << s;
    }
}

void BlurTO::Private::releaseGl()
{
    stages.clear();
}

void BlurTO::Private::renderGl(const GL::RenderSettings& , const RenderTime& time)
{
    auto texIn = to->inputTexture(0, time);
    if (!texIn)
        return;

    // Determine size
    Float six = 1. / 1000.,
          siy = six;

    /** XXX @bug Bug in TextureObjectBase::resolution()
        for RM_INPUT mode. First call to resolution() returns
        custom resolution... */
    auto res = //to->adjustResolution(
                QSize(texIn->width(), texIn->height());
                //to->resolution();

    if (!res.isEmpty())
    {
        if (p_sizeIsPixels->value(time))
        {
            //std::cout << res.width() << "x" << res.height() << std::endl;
            auto ma = std::max(res.width(), res.height());
            six = siy = 1.f / ma;
        }

        // aspect
        siy = siy * res.width() / res.height();
    }

    // scale size by samples
    Float numSam = std::max(1., p_num->value(time));
    six /= numSam;
    siy /= numSam;

    // get other values
    Float size = p_size->value(time),
          sigma = std::max(0.001, p_sigma->value(time) * numSam * .5),
          alpha = p_alpha->value(time),
          maskmin = p_mask_min->value(time),
          maskmax = p_mask_max->value(time),
          maskthresh = p_mask_thresh->value(time);

    // for each stage
    for (const Stage & s : stages)
    {
        // update uniforms
        s.u_size_sigma->setFloats( size, sigma );
        s.u_num->setFloats( numSam );
        s.u_dir->setFloats( six * s.dir.x, siy * s.dir.y );
        s.u_alpha->setFloats( alpha );
        if (s.u_mask_range)
            s.u_mask_range->setFloats(maskmin, maskmax, maskthresh);

        //MO_PRINT("STAGE " << s.index);
        //s.u_alpha->shader()->dumpUniforms();

        uint texSlot = 0;
        to->renderShaderQuad(s.index, time, &texSlot);
    }
}


} // namespace MO
