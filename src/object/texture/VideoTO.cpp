/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 5/4/2016</p>
*/

#ifdef MO_ENABLE_FFMPEG

#include "VideoTO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterFilename.h"
#include "gl/Texture.h"
#include "gl/Shader.h"
//#include "gl/VideoTextureBuffer.h"
//#include "video/ffm/VideoStream.h"
#include "video/DecoderThread.h"
#include "video/DecoderFrame.h"
#include "io/FileManager.h"
#include "io/DataStream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(VideoTO)

struct VideoTO::Private
{
    Private(VideoTO* to)
        : to            (to)
        , tex           (nullptr)
        , pFilename     (nullptr)
    { }
    ~Private()
    {
        delete tex;
    }

    void setTex(GL::Texture*);

    VideoTO* to;
    QString initFilename;
    DecoderThread decoder;
    GL::Texture * tex;
    double lastTime;

    ParameterFilename * pFilename;
    ParameterInt * pMipmaps;

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
                    IO::FT_VIDEO,
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

void VideoTO::Private::setTex(GL::Texture* t)
{
    if (tex)
        tex->release();
    delete tex;
    tex = t;
}

void VideoTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);

    p_->setTex(nullptr);

    try
    {
        QString fn = IO::fileManager().localFilename(p_->pFilename->baseValue());
        if (fn.isEmpty())
            p_->decoder.close();
        else
        {
            p_->decoder.openFile(fn.toStdString());
            p_->decoder.start();
            p_->lastTime = 0.;
        }
    }
    catch (const Exception& e)
    {
        setErrorMessage(QString("VIDEO: ") + e.what());
    }
}

void VideoTO::releaseGl(uint thread)
{
    p_->decoder.close();
    p_->setTex(nullptr);

    TextureObjectBase::releaseGl(thread);
}

const GL::Texture * VideoTO::valueTexture(uint chan, const RenderTime& time) const
{
    if (chan != 0 || !p_->decoder.isReady())
        return nullptr;

    if (time.second() < p_->lastTime)
    {
        p_->decoder.doneFrames();
        p_->decoder.seekSecond(time.second());
    }
    p_->lastTime = time.second();

    DecoderFrame *frameA, *frameB;
    double mix;
    p_->decoder.getFrames(time.second(), &frameA, &frameB, &mix);
    p_->decoder.doneFramesBefore(time.second()-.2);
    if (!frameA)
        return nullptr;

    p_->setTex(frameA->createTextureYUV());
    return p_->tex;
}



} // namespace MO

#endif // #ifdef MO_ENABLE_FFMPEG
