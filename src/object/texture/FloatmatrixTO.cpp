/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#include "FloatMatrixTO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterFloatMatrix.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterText.h"
#include "object/util/ObjectEditor.h"
#include "gl/Texture.h"
#include "io/DataStream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(FloatMatrixTO)

struct FloatMatrixTO::Private
{
    Private(FloatMatrixTO * to)
        : to            (to)
        , tex           (nullptr)
    { }

    ~Private()
    {
        delete tex;
    }

    void createParameters();
    void updateTexture(size_t width, size_t height);
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    FloatMatrixTO * to;

    GL::Texture* tex;

    ParameterFloat
            *p_amplitude, *p_offset;
    ParameterSelect
            *p_clampWidth, *p_clampHeight, *p_flipX, *p_flipY;
    ParameterInt
            *p_offsetX, *p_offsetY,
            *p_width, *p_height;

    ParameterFloatMatrix* p_matrix;
    std::vector<gl::GLfloat> buffer;
};


FloatMatrixTO::FloatMatrixTO()
    : TextureObjectBase ()
    , p_                (new Private(this))
{
    setName("FloatMatrix");
    initMaximumTextureInputs(0);
    /** @todo implement change-check to ParameterFloatMatrix */
    initDefaultUpdateMode(UM_ALWAYS);
    initInternalFbo(false);
}

FloatMatrixTO::~FloatMatrixTO()
{
    delete p_;
}

void FloatMatrixTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texfloatm", 1);

}

void FloatMatrixTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    //const int ver =
            io.readHeader("texfloatm", 1);
}

void FloatMatrixTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void FloatMatrixTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void FloatMatrixTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void FloatMatrixTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void FloatMatrixTO::Private::createParameters()
{
    to->params()->beginParameterGroup("matrix_input", tr("input"));
    to->initParameterGroupExpanded("matrix_input");

        p_matrix = to->params()->createFloatMatrixParameter(
                    "matrix0", tr("matrix"),
                    tr("The float matrix to convert to texture"),
                    FloatMatrix(), true, true);
        p_matrix->setVisibleGraph(true);

        p_amplitude = to->params()->createFloatParameter(
                    "amp", tr("amplitude"),
                    tr("Amplitude of input signal"), 1., 0.01);

        p_offset = to->params()->createFloatParameter(
                    "offset", tr("offset"),
                    tr("Offset to input signal"), 0., 0.01);

        p_offsetX = to->params()->createIntParameter(
                    "offset_x", tr("skip X"),
                    tr("Skips the number of columns in the input"),
                    0, true, true);
        p_offsetX->setMinValue(0);

        p_offsetY = to->params()->createIntParameter(
                    "offset_y", tr("skip Y"),
                    tr("Skips the number of rows in the input"),
                    0, true, true);
        p_offsetY->setMinValue(0);

        p_clampWidth = to->params()->createBooleanParameter(
                    "clamp_x", tr("fixed width"),
                    tr("Selects a fixed width for the output texture"),
                    tr("Off"), tr("On"),
                    false, true, false);

        p_clampHeight = to->params()->createBooleanParameter(
                    "clamp_y", tr("fixed height"),
                    tr("Selects a fixed height for the output texture"),
                    tr("Off"), tr("On"),
                    false, true, false);

        p_width = to->params()->createIntParameter(
                    "width", tr("width"), tr("The texture width"),
                    1024, true, true);
        p_width->setMinValue(1);

        p_height = to->params()->createIntParameter(
                    "height", tr("height"), tr("The texture height"),
                    1024, true, true);
        p_height->setMinValue(1);


        p_flipX = to->params()->createBooleanParameter(
                    "flip_x", tr("flip X"),
                    tr("Flip texture on X axis"), tr("Off"), tr("On"),
                    false);
        p_flipY = to->params()->createBooleanParameter(
                    "flip_y", tr("flip Y"),
                    tr("Flip texture on Y axis"), tr("Off"), tr("On"),
                    false);

    to->params()->endParameterGroup();

}

void FloatMatrixTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

}

void FloatMatrixTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void FloatMatrixTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    p_->p_width->setVisible(p_->p_clampWidth->baseValue());
    p_->p_height->setVisible(p_->p_clampHeight->baseValue());
}


void FloatMatrixTO::Private::initGl()
{


}

void FloatMatrixTO::Private::updateTexture(size_t width, size_t height)
{
    if (width == 0 || height == 0)
    {
        if (tex)
        {
            if (tex->isAllocated())
                tex->release();
            delete tex;
            tex = nullptr;
        }
        return;
    }

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

    if (!tex)
        tex = new GL::Texture();

    if (!tex->isAllocated())
        tex->create(width, height,
                gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT, nullptr);
}

void FloatMatrixTO::Private::releaseGl()
{
    if (tex && tex->isHandle())
        tex->release();
    delete tex;
    tex = nullptr;
}


void FloatMatrixTO::Private::renderGl(
        const GL::RenderSettings& , const RenderTime& time)
{
    auto matrix = p_matrix->value(time);

    const size_t
            matrixWidth = matrix.numDimensions() > 0 ? matrix.size(0) : 0,
            matrixHeight = matrix.numDimensions() > 1 ? matrix.size(1) : 1;
    size_t  width = matrixWidth,
            height = matrixHeight,
            offsetX = p_offsetX->value(time),
            offsetY = p_offsetY->value(time);

    if (p_clampWidth->value(time))
        width = p_width->value(time);
    if (p_clampHeight->value(time))
        height = p_height->value(time);

    width -= std::min(width, offsetX);
    height -= std::min(height, offsetY);

    updateTexture(width, height);
    if (!tex)
        return;

    // -- update buffer

    const bool
            flipX = p_flipX->value(time),
            flipY = p_flipY->value(time);
    const Double
            ampl = p_amplitude->value(time),
            offset = p_offset->value(time);

    buffer.resize(width * height);

    for (size_t j=0; j<height; ++j)
    {
        size_t ry = j + offsetY,
               wy = flipY ? height-1-j : j;
        if (ry >= matrixHeight)
        {
            for (size_t i=0; i<width; ++i)
                buffer[wy*width + i] = 0.f;
        }
        else
        for (size_t i=0; i<width; ++i)
        {
            size_t rx = i + offsetX,
                   wx = flipX ? width-1-i : i;
            if (rx < matrixWidth)
                buffer[wy*width + wx] = offset + ampl *
                                    *matrix.data(ry*matrixWidth + rx);
            else
                buffer[wy*width + wx] = 0.f;
        }
    }

    // -- upload --

    tex->bind();
    tex->upload(&buffer[0]);
    tex->setChanged();
}

const GL::Texture* FloatMatrixTO::valueTexture(uint chan, const RenderTime &) const
{
    if (chan != 0)
        return nullptr;
    return p_->tex;
}

} // namespace MO

