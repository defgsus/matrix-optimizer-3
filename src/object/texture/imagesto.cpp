/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/27/2015</p>
*/

#include "imagesto.h"
#include "object/param/parameters.h"
#include "object/param/parameterint.h"
#include "object/param/parameterimagelist.h"
#include "object/param/parameterselect.h"
#include "gl/texture.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(ImagesTO)

ImagesTO::ImagesTO(QObject *parent)
    : TextureObjectBase (parent)
    , pFilenames_       (0)
{
    setName("Images");
    initMaximumTextureInputs(0);
    initFilenames_ = QStringList() << ":/texture/mo_black.png";
}

ImagesTO::~ImagesTO()
{
    for (auto t : tex_)
        delete t;
}

void ImagesTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("toimgs", 1);
}

void ImagesTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("toimgs", 1);
}

void ImagesTO::createParameters()
{
    TextureObjectBase::createParameters();

    params()->beginParameterGroup("images", tr("images"));
    initParameterGroupExpanded("images");

        pFilenames_ = params()->createImageListParameter(
                    "filenames", tr("image files"), tr("The list of images"),
                    initFilenames_);

        pIndex_ = params()->createIntParameter(
                    "index", tr("image index"),
                    tr("Selects the texture, first is 0"),
                    0, true, true);
        pIndex_->setMinValue(0);

    params()->endParameterGroup();
}

void ImagesTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == pFilenames_)
        requestReinitGl();
}

void ImagesTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void ImagesTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();
}

void ImagesTO::setImageFilenames(const QStringList &fn)
{
    if (!pFilenames_)
        initFilenames_ = fn;
    else
    {
        if (fn == pFilenames_->baseValue())
            return;
        pFilenames_->setValue(fn);
        requestReinitGl();
    }
}

void ImagesTO::getNeededFiles(IO::FileList & files)
{
    for (const QString& fn : pFilenames_->baseValue())
        files << IO::FileListEntry(fn, IO::FT_TEXTURE);
}

void ImagesTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);

    for (const QString& fn : pFilenames_->baseValue())
    {
        QString fnl = IO::fileManager().localFilename(fn);

        try
        {
            auto tex = GL::Texture::createFromImage(fnl, gl::GL_RGBA);
            tex_ << tex;
        }
        catch (const Exception& e)
        {
            setError(e.what());
        }
    }
}

void ImagesTO::releaseGl(uint thread)
{
    for (auto t : tex_)
    {
        if (t->isAllocated())
            t->release();
        delete t;
    }
    tex_.clear();

    TextureObjectBase::releaseGl(thread);
}

const GL::Texture * ImagesTO::valueTexture(uint , Double time, uint thread) const
{
    if (tex_.isEmpty())
        return 0;

    int index = std::min(pIndex_->value(time, thread), tex_.count() - 1);
    return tex_[index];
}



} // namespace MO
