/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#include "TextureDO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterTexture.h"
#include "gl/Texture.h"
#include "io/DataStream.h"
#include "io/log_FloatMatrix.h"

namespace MO {

MO_REGISTER_OBJECT(TextureDO)

struct TextureDO::Private
{
    Private(TextureDO* p)
        : p         (p)
        , doReload  (true)
    { }

    bool download(const RenderTime& time);

    TextureDO* p;
    bool doReload;
    ParameterTexture* p_texIn;
    FloatMatrix matrix;
};

TextureDO::TextureDO()
    : Object    ()
    , p_        (new Private(this))
{
    setName("Texture2Matrix");
    setNumberOutputs(ST_FLOAT_MATRIX, 1);
}

TextureDO::~TextureDO() { delete p_; }

void TextureDO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("texdo", 1);
}

void TextureDO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("texdo", 1);
}

void TextureDO::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("modval", tr("value"));
    initParameterGroupExpanded("modval");

        p_->p_texIn = params()->createTextureParameter(
            "texture", tr("texture"),
            tr("Input texture"));
        p_->p_texIn->setVisibleGraph(true);

    params()->endParameterGroup();
}

void TextureDO::onParameterChanged(Parameter* p)
{
    Object::onParameterChanged(p);

    if (p == p_->p_texIn)
        p_->doReload = true;
}


FloatMatrix TextureDO::valueFloatMatrix(uint, const RenderTime& time) const
{
    if (time.thread() == MO_GFX_THREAD)
    if (p_->doReload || p_->p_texIn->hasChanged(time))
    {
        if (!p_->download(time))
            p_->matrix.clear();
        p_->doReload = false;
    }
    return p_->matrix;
}

bool TextureDO::hasFloatMatrixChanged(
        uint , const RenderTime& time) const
{
    return p_->doReload || p_->p_texIn->hasChanged(time);
}


bool TextureDO::Private::download(const RenderTime& time)
{
    p->clearError();

    auto tex = p_texIn->value(time);
    if (!tex)
    {
        p->setErrorMessage(tr("No input texture"));
        return false;
    }

    std::vector<size_t> dims;
    if (tex->is3d())
    {
        dims = { tex->width(), tex->height(), tex->depth() };
    }
    else if (tex->isCube())
    {
        p->setErrorMessage(tr("Cube texture not supported currently"));
        return false;
    }
    else
    {
        dims = { tex->height(), tex->width() };
    }

    matrix.setDimensions(dims);

    try
    {
        tex->bind();
        std::vector<gl::GLfloat> data(matrix.size());
        tex->download(data.data(), gl::GL_RED, gl::GL_FLOAT, 0);
        for (size_t i=0; i<matrix.size(); ++i)
            *matrix.data(i) = data[i];
    }
    catch (const Exception& e)
    {
        p->setErrorMessage(e.what());
        return false;
    }

    return true;
}


} // namespace MO
