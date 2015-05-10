/** @file blurto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#include "blurto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/datastream.h"
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
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

    struct Stage
    {
        uint index;
        Vec2 dir;
        GL::Uniform
                * u_size_sigma,
                * u_num,
                * u_dir,
                * u_alpha;
    };

    BlurTO * to;
    QList<Stage> stages;

    ParameterFloat
            * p_size,
            * p_sigma,
            * p_num,
            * p_alpha;
    ParameterSelect
            * p_sizeIsPixels,
            * p_alphaMode;
};


BlurTO::BlurTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Blur");
    initMaximumTextureInputs(1);
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

void BlurTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
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
                    1.0,  0.05);
        p_sigma->setMinValue(0.);

        p_num = to->params()->createFloatParameter(
                    "num", tr("num samples"), tr("Size of the blur kernel - there will be (width*2+1)*2 samples"),
                    10.0,  1., 1000.,  1.);

        p_alphaMode = to->params()->createSelectParameter(
                    "alpha_mode", tr("alpha channel"),
                    tr("The way the alpha channel is calculated"),
        { "blur", "fix", "mag" },
        { tr("blur"), tr("fixed"), tr("magnitude") },
        { tr("The alpha channels is blurred the same way as the other channels"),
          tr("The alpha channel remains a fixed value"),
          tr("The alpha channel is set to the magnitude of the blured color channels") },
        { 0, 1, 2 },
                    0, true, true);

        p_alpha = to->params()->createFloatParameter(
                    "alpha", tr("alpha"), tr("A modifier for the output alpha channel - controls transparency"),
                    1.0,  0.05);
        p_alpha->setMinValue(0.);

    to->params()->endParameterGroup();
}

void BlurTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_alphaMode)
        requestReinitGl();
}

void BlurTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void BlurTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();


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
        if (i == 1)
            src.addDefine(QString("#define MO_ALPHA_MODE %1").arg(p_alphaMode->baseValue()));

        auto shader = to->createShaderQuad(src, { "u_tex" })->shader();

        // uniforms

        s.u_size_sigma = shader->getUniform("u_size_sigma", true);
        s.u_num = shader->getUniform("u_num", true);
        s.u_dir = shader->getUniform("u_direction", true);
        s.u_alpha = shader->getUniform("u_alpha", true);

        stages << s;
    }
}

void BlurTO::Private::releaseGl()
{
    stages.clear();
}

void BlurTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    // Determine size
    Float six = 1. / 1000., siy = six * to->aspectRatio();
    if (p_sizeIsPixels->value(time, thread))
    {
        auto res = to->resolution();
        six = 1.f / std::max(1, res.width());
        siy = 1.f / std::max(1, res.width());
    }

    // scale size by samples
    Float numSam = std::max(1., p_num->value(time, thread));
    six /= numSam;
    siy /= numSam;

    // get other values
    Float size = p_size->value(time, thread),
          sigma = std::max(0.001, p_sigma->value(time, thread) * numSam * .5),
          alpha = p_alpha->value(time, thread);

    // for each stage
    for (const Stage & s : stages)
    {
//        MO_PRINT("SRAGE " << s.index);
//        s.u_alpha->shader()->dumpUniforms();

        // update uniforms
        s.u_size_sigma->setFloats( size, sigma );
        s.u_num->setFloats( numSam );
        s.u_dir->setFloats( six * s.dir.x, siy * s.dir.y );
        s.u_alpha->setFloats( alpha );

        uint texSlot = 0;
        to->renderShaderQuad(s.index, time, thread, texSlot);
    }
}


} // namespace MO
