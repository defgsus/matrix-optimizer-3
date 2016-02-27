/** @file imageto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#include "imageto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfilename.h"
#include "gl/texture.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(ImageTO)

ImageTO::ImageTO()
    : TextureObjectBase ()
    , initFilename_     (":/texture/mo_black.png")
    , pFilename_        (0)
    , tex_              (0)
{
    setName("Image");
    initMaximumTextureInputs(0);
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
                    initFilename_);

        pMipmaps_ = params()->createIntParameter(
                    "mipmaps", tr("mip-map levels"),
                    tr("The number of mip-map levels to create, "
                       "where each level is half the size of the previous level, "
                       "0 means no mip-maps"),
                    0, true, false);
        pMipmaps_->setMinValue(0);
        pMipmaps_->setDefaultEvolvable(false);

    params()->endParameterGroup();
}

void ImageTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == pFilename_
        || p == pMipmaps_)
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

void ImageTO::setImageFilename(const QString &fn)
{
    if (!pFilename_)
        initFilename_ = fn;
    else
    {
        if (fn == pFilename_->baseValue())
            return;
        pFilename_->setValue(fn);
        requestReinitGl();
    }
}

void ImageTO::getNeededFiles(IO::FileList & files)
{
    TextureObjectBase::getNeededFiles(files);

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
        tex_ = GL::Texture::createFromImage(fn, getDesiredTextureFormat(),
                                            pMipmaps_->baseValue());
    }
    catch (const Exception& e)
    {
        tex_ = 0;
        setErrorMessage(e.what());
    }
}

void ImageTO::releaseGl(uint thread)
{
    if (tex_ && tex_->isAllocated())
        tex_->release();

    TextureObjectBase::releaseGl(thread);
}

const GL::Texture * ImageTO::valueTexture(uint chan, const RenderTime& ) const
{
    return chan==0 ? tex_ : 0;
}



} // namespace MO
