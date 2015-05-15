/** @file imageto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#include "imageto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterselect.h"
#include "gl/texture.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(ImageTO)

ImageTO::ImageTO(QObject *parent)
    : TextureObjectBase (parent)
    , tex_              (0)
{
    setName("Image");
    initMaximumTextureInputs(1);
}

ImageTO::~ImageTO()
{
    delete tex_;
}

void ImageTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("toimg", 1);
}

void ImageTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("toimg", 1);
}

void ImageTO::createParameters()
{
    TextureObjectBase::createParameters();

    params()->beginParameterGroup("image", tr("image"));
    initParameterGroupExpanded("image");

        pFilename_ = params()->createFilenameParameter(
                    "filename", tr("image file"), tr("Filename of the image"),
                    IO::FT_TEXTURE,
                    ":/texture/mo_black.png");

    params()->endParameterGroup();
}

void ImageTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == pFilename_)
        requestReinitGl();
}

void ImageTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void ImageTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();


}

void ImageTO::getNeededFiles(IO::FileList & files)
{
    if (!pFilename_->baseValue().isEmpty())
        files << IO::FileListEntry(pFilename_->baseValue(), IO::FT_TEXTURE);
}

void ImageTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);

    if (tex_ && tex_->isAllocated())
        tex_->release();
    delete tex_;

    try
    {
        QString fn = IO::fileManager().localFilename(pFilename_->baseValue());
        tex_ = GL::Texture::createFromImage(fn, gl::GL_RGBA);
        //Image img(567, 433, Image::F_RGBA_32);
        //img.pixel()
    }
    catch (Exception& )
    {
        // XXX signal gui
        tex_ = 0;
    }
}

void ImageTO::releaseGl(uint thread)
{
    if (tex_ && tex_->isAllocated())
        tex_->release();

    TextureObjectBase::releaseGl(thread);
}

const GL::Texture * ImageTO::valueTexture(uint , Double , uint ) const
{
    return tex_;
}



} // namespace MO
