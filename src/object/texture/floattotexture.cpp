/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/15/2016</p>
*/

#include "floattotexture.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/util/objecteditor.h"
#include "gl/texture.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(FloatToTextureTO)

struct FloatToTextureTO::Private
{
    Private(FloatToTextureTO * to)
        : to            (to)
        , tex           (nullptr)
    { }

    ~Private()
    {
        delete tex;
    }

    void createParameters();
    void updateTexture();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    FloatToTextureTO * to;

    GL::Texture* tex;

    ParameterFloat
            *p_amplitude, *p_offset, *p_time_range;
    ParameterInt
            *p_numParams, *p_width;
    ParameterSelect
            *p_flipX, *p_flipY;

    std::vector<ParameterFloat*> p_inputs;
    std::vector<gl::GLfloat> buffer;
};


FloatToTextureTO::FloatToTextureTO()
    : TextureObjectBase ()
    , p_                (new Private(this))
{
    setName("FloatToTexture");
    initMaximumTextureInputs(0);
    initDefaultUpdateMode(UM_ALWAYS);
    initInternalFbo(false);
}

FloatToTextureTO::~FloatToTextureTO()
{
    delete p_;
}

void FloatToTextureTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texfloat", 1);

}

void FloatToTextureTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    //const int ver =
            io.readHeader("texfloat", 1);
}

void FloatToTextureTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void FloatToTextureTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void FloatToTextureTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void FloatToTextureTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void FloatToTextureTO::Private::createParameters()
{
    to->params()->beginParameterGroup("float_input", tr("float input"));
    to->initParameterGroupExpanded("float_input");

        p_numParams = to->params()->createIntParameter(
                    "num_params", tr("number inputs"),
                    tr("The number of float inputs / scanlines"),
                    4, true, false);
        p_numParams->setMinValue(1);

        p_width = to->params()->createIntParameter(
                    "width", tr("number samples"),
                    tr("The number of input samples and width of texture"),
                    1024, true, false);
        p_width->setMinValue(4);

        p_amplitude = to->params()->createFloatParameter(
                    "amp", tr("amplitude"),
                    tr("Amplitude of input signal"), 1., 0.01);

        p_offset = to->params()->createFloatParameter(
                    "offset", tr("offset"),
                    tr("Offset to input signal"), 0., 0.01);

        p_time_range = to->params()->createFloatParameter(
                    "time_range", tr("time range"),
                    tr("Range of seconds to cover in input"), 1., 0.01);

        p_flipX = to->params()->createBooleanParameter(
                    "flip_x", tr("flip X"),
                    tr("Flip texture on X axis"), tr("Off"), tr("On"),
                    false);
        p_flipY = to->params()->createBooleanParameter(
                    "flip_y", tr("flip Y"),
                    tr("Flip texture on Y axis"), tr("Off"), tr("On"),
                    false);

    to->params()->endParameterGroup();

    to->params()->beginParameterGroup("float_inputs", tr("float inputs"));

        for (int i=0; i<64; ++i)
        {
            ParameterFloat* par = to->params()->createFloatParameter(
                        QString("amp%1").arg(i),
                        QString("%1").arg(i+1),
                        tr("The %1th input").arg(i+1),
                        0., 0.1, true, true);
            par->setVisibleGraph(true);

            p_inputs.push_back(par);
        }

    to->params()->endParameterGroup();

}

void FloatToTextureTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_numParams
        || p == p_->p_width)
        requestReinitGl();
}

void FloatToTextureTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void FloatToTextureTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    int k = 0, num = p_->p_numParams->baseValue();
    for (auto p : p_->p_inputs)
        p->setVisible(k++ < num);
}


void FloatToTextureTO::Private::initGl()
{
    // -- update texture --

    updateTexture();
}

void FloatToTextureTO::Private::updateTexture()
{
    const size_t
        width = p_width->baseValue(),
        height = p_numParams->baseValue();

    if (tex)
    {
        if (tex->isHandle() &&
            (!tex->isAllocated() || tex->width() != width || tex->height() != height))
        {
            tex->release();
            delete tex;
            tex = nullptr;
        }
    }

    else
        tex = new GL::Texture();

    if (!tex->isAllocated())
        tex->create(width, height,
                gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT, nullptr);
    /*
    if (!tex)
    {
        tex = new GL::Texture(width, height,
                gl::GL_RGBA, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
        tex->create();
    }
    */
}

void FloatToTextureTO::Private::releaseGl()
{
    if (tex && tex->isHandle())
        tex->release();
    delete tex;
    tex = nullptr;
}


void FloatToTextureTO::Private::renderGl(
        const GL::RenderSettings& , const RenderTime& time)
{
    updateTexture();

    const size_t
            width = tex->width(),
            height = tex->height();

    // -- update buffer

    buffer.resize(tex->width() * tex->height() * 4);

    const bool
            flipX = !p_flipX->value(time),
            flipY = p_flipY->value(time);
    const Double
            timeRange = p_time_range->value(time),
            timeShiftAbs = 1. / Double(width) * timeRange,
            timeShift = flipX ? timeShiftAbs : -timeShiftAbs,
            ampl = p_amplitude->value(time),
            offset = p_offset->value(time);

    //for (auto& f : buffer)
    //    f = offset + ampl * float(rand()) / RAND_MAX;

    RenderTime startTime(flipX ? time - timeRange : time);

    for (size_t j = 0; j < height; ++j)
    {
        size_t y = flipY ? height - 1 - j : j;
        RenderTime ti(startTime);
        for (size_t i = 0; i < width; ++i)
        {
            ti += timeShift;
            buffer[y * width + i] =
                    offset + ampl * p_inputs[j]->value(ti);
        }
    }

    // -- upload --

    tex->bind();
    tex->upload(&buffer[0]);
    tex->setChanged();
}

const GL::Texture* FloatToTextureTO::valueTexture(uint chan, const RenderTime &) const
{
    if (chan != 0)
        return nullptr;
    return p_->tex;
}

} // namespace MO

