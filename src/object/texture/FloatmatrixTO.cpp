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
#include "io/log_FloatMatrix.h"

namespace MO {

MO_REGISTER_OBJECT(FloatMatrixTO)

struct FloatMatrixTO::Private
{
    Private(FloatMatrixTO * to)
        : to            (to)
        , tex           (nullptr)
        , doReUpload    (true)
    { }

    ~Private()
    {
        delete tex;
    }

    void createParameters();
    void updateTexture(size_t width, size_t height, size_t depth);
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    FloatMatrixTO * to;

    GL::Texture* tex;
    bool doReUpload;

    ParameterFloat
            *p_amplitude, *p_offset;
    ParameterSelect
            *p_clampWidth, *p_clampHeight, *p_clampDepth,
            *p_flipX, *p_flipY, *p_flipZ;
    ParameterInt
            *p_offsetX, *p_offsetY, *p_offsetZ,
            *p_width, *p_height, *p_depth;

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

        p_offsetZ = to->params()->createIntParameter(
                    "offset_z", tr("skip Z"),
                    tr("Skips the number of depth-planes in the input"),
                    0, true, true);
        p_offsetZ->setMinValue(0);

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

        p_clampDepth = to->params()->createBooleanParameter(
                    "clamp_z", tr("fixed depth"),
                    tr("Selects a fixed depth for the output texture"),
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

        p_depth = to->params()->createIntParameter(
                    "depth", tr("depth"), tr("The texture depth"),
                    16, true, true);
        p_depth->setMinValue(1);

        p_flipX = to->params()->createBooleanParameter(
                    "flip_x", tr("flip X"),
                    tr("Flip texture on X axis"), tr("Off"), tr("On"),
                    false);
        p_flipY = to->params()->createBooleanParameter(
                    "flip_y", tr("flip Y"),
                    tr("Flip texture on Y axis"), tr("Off"), tr("On"),
                    false);
        p_flipZ = to->params()->createBooleanParameter(
                    "flip_z", tr("flip Z"),
                    tr("Flip texture on Z axis"), tr("Off"), tr("On"),
                    false);

    to->params()->endParameterGroup();

}

void FloatMatrixTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    p_->doReUpload = true;
    requestRender();
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
    p_->p_depth->setVisible(p_->p_clampDepth->baseValue());
}


void FloatMatrixTO::Private::initGl()
{


}

void FloatMatrixTO::Private::updateTexture(
            size_t width, size_t height, size_t depth)
{
    if (width == 0 || height == 0 || depth == 0)
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
        if (tex->isHandle()
                && (!tex->isAllocated()
                    || tex->width() != width
                    || tex->height() != height
                    || (tex->depth() != depth && depth > 1))
           )
        {
            tex->release();
            delete tex;
            tex = nullptr;
        }
    }

    if (!tex)
        tex = new GL::Texture();

    if (!tex->isAllocated())
    {
        if (depth > 1)
            tex->create(width, height, depth,
                gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT, nullptr);
        else
            tex->create(width, height,
                gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT, nullptr);
    }
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
    if (tex && !p_matrix->hasChanged(time) && !doReUpload)
        return;
    doReUpload = false;

    auto matrix = p_matrix->value(time);

    MO_DEBUG_FM("FloatMatrixTO(" << to->idName()
                << "): update texture from matrix "
                << matrix.layoutString());

    const size_t
            matrixWidth =  matrix.numDimensions() > 0 ? matrix.size(0) : 0,
            matrixHeight = matrix.numDimensions() > 1 ? matrix.size(1) : 1,
            matrixDepth =  matrix.numDimensions() > 2 ? matrix.size(2) : 1;
    size_t  width = matrixWidth,
            height = matrixHeight,
            depth = matrixDepth,
            offsetX = p_offsetX->value(time),
            offsetY = p_offsetY->value(time),
            offsetZ = p_offsetZ->value(time);

    if (p_clampWidth->value(time))
        width = p_width->value(time);
    if (p_clampHeight->value(time))
        height = p_height->value(time);
    if (p_clampDepth->value(time))
        depth = p_depth->value(time);

    width -= std::min(width, offsetX);
    height -= std::min(height, offsetY);
    depth -= std::min(depth, offsetZ);

    updateTexture(width, height, depth);
    if (!tex)
        return;

    // -- update buffer

    const bool
            flipX = p_flipX->value(time),
            flipY = p_flipY->value(time),
            flipZ = p_flipZ->value(time);
    const Double
            ampl = p_amplitude->value(time),
            offset = p_offset->value(time);

    buffer.resize(width * height * depth);

    for (size_t k=0; k<depth; ++k)
    {
        size_t rz = k + offsetZ,
               wz = flipZ ? depth-1-k : k,
               bidxz = wz * height * width,
               midxz = rz * matrixHeight * matrixWidth;

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
                    buffer[bidxz + wy*width + wx] = offset + ampl *
                                     * matrix.data(midxz + ry*matrixWidth + rx);
                else
                    buffer[bidxz + wy*width + wx] = 0.f;
            }
        }
    }

    // -- upload --

    tex->bind();
    tex->upload(buffer.data());
    tex->setChanged();
}

const GL::Texture* FloatMatrixTO::valueTexture(
                                    uint chan, const RenderTime &) const
{
    if (chan != 0)
        return nullptr;
    return p_->tex;
}

} // namespace MO

