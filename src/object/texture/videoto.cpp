/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 5/4/2016</p>
*/

#ifdef MO_ENABLE_FFMPEG

#include "videoto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfilename.h"
#include "gl/texture.h"
#include "gl/shader.h"
#include "video/ffm/videostream.h"
#include "video/decoderframe.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(VideoTO)

struct VideoTO::Private
{
    Private(VideoTO* to)
        : to            (to)
        , pFilename     (0)
        , tex           (0)
    { }
    ~Private()
    {
        delete tex;
    }

    VideoTO* to;
    QString initFilename;
    ParameterFilename * pFilename;
    ParameterInt * pMipmaps;
    GL::Texture * tex;
    FFM::VideoStream stream;
};

VideoTO::VideoTO()
    : TextureObjectBase ()
    , p_    (new Private(this))

{
    setName("Video");
    initMaximumTextureInputs(0);
}

VideoTO::~VideoTO()
{
    delete p_;
}

void VideoTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("tovideo", 1);
}

void VideoTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("tovideo", 1);
}

void VideoTO::createParameters()
{
    TextureObjectBase::createParameters();

    params()->beginParameterGroup("video", tr("video"));
    initParameterGroupExpanded("video");

        p_->pFilename = params()->createFilenameParameter(
                    "filename", tr("image file"), tr("Filename of the image"),
                    IO::FT_TEXTURE,
                    p_->initFilename);
        /*
        pMipmaps_ = params()->createIntParameter(
                    "mipmaps", tr("mip-map levels"),
                    tr("The number of mip-map levels to create, "
                       "where each level is half the size of the previous level, "
                       "0 means no mip-maps"),
                    0, true, false);
        pMipmaps_->setMinValue(0);
        pMipmaps_->setDefaultEvolvable(false);
        */
    params()->endParameterGroup();
}

void VideoTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->pFilename
        //|| p == pMipmaps_
            )
        requestReinitGl();
}

void VideoTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void VideoTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();
}

void VideoTO::setVideoFilename(const QString &fn)
{
    if (!p_->pFilename)
        p_->initFilename = fn;
    else
    {
        if (fn == p_->pFilename->baseValue())
            return;
        p_->pFilename->setValue(fn);
        requestReinitGl();
    }
}

void VideoTO::getNeededFiles(IO::FileList & files)
{
    TextureObjectBase::getNeededFiles(files);

    if (!p_->pFilename->baseValue().isEmpty())
        files << IO::FileListEntry(p_->pFilename->baseValue(), IO::FT_TEXTURE);
}

void VideoTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);

    if (p_->tex && p_->tex->isAllocated())
        p_->tex->release();
    delete p_->tex;

    try
    {
        QString fn = IO::fileManager().localFilename(p_->pFilename->baseValue());
        p_->stream.openFile(fn.toStdString());
        //tex_ = GL::Texture::createFromImage(fn, getDesiredTextureFormat(),
        //                                    pMipmaps_->baseValue());
    }
    catch (const Exception& e)
    {
        p_->tex = 0;
        setErrorMessage(e.what());
    }
}

void VideoTO::releaseGl(uint thread)
{
    if (p_->tex && p_->tex->isAllocated())
        p_->tex->release();

    p_->stream.close();

    TextureObjectBase::releaseGl(thread);
}

const GL::Texture * VideoTO::valueTexture(uint chan, const RenderTime& ) const
{

    return chan==0 ? p_->tex : 0;
}



} // namespace MO

#endif // #ifdef MO_ENABLE_FFMPEG
