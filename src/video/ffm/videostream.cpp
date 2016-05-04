#ifdef MO_ENABLE_FFMPEG

#include <sstream>
#include <algorithm>

#include "videostream.h"
#include "ffmpeg.h"
#include "io/error.h"
#include "io/log.h"

namespace FFM {

struct VideoStream::Private
{
    Private(VideoStream * p)
        : p             (p)
        , threadCount   (0)
        , formatCtx     (0)
        , videoCodecCtx (0)
        , videoStream   (0)
        , videoStreamIndex(-1)
        , videoFramesDecoded (0)
        , videoCodec    (0)
        , swsContext    (0)
        , videoFrame    (0)
        , videoConvFrame(0)
        , packet        (0)
        , lastDecodedPts(-1.)
    { }

    void openFile(const std::string& fn);
    void close();
    DecoderFrame* createVideoFrame();
    std::string getFrameDescription(const AVFrame*) const;

    VideoStream * p;

    std::string url;
    int threadCount;

    AVFormatContext* formatCtx;
    AVCodecContext* videoCodecCtx;
    AVStream* videoStream;
    int videoStreamIndex;
    int64_t videoFramesDecoded;
    AVCodec* videoCodec;
    SwsContext* swsContext;

    AVFrame* videoFrame, *videoConvFrame;
    AVPacket* packet;

    double lastDecodedPts;
};

VideoStream::VideoStream()
    : p_        (new Private(this))
{
}

VideoStream::~VideoStream()
{
    p_->close();
    delete p_;
}

int VideoStream::threadCount() const { return p_->threadCount; }
bool VideoStream::isReady() const { return p_->videoCodecCtx != nullptr; }
const std::string& VideoStream::currentFilename() const { return p_->url; }
size_t VideoStream::curFilePos() const
    { return p_->packet ? p_->packet->pos >= 0 ? p_->packet->pos : 0 : 0; }
int64_t VideoStream::numDecodedFrames() const { return p_->videoFramesDecoded; }
double VideoStream::framesPerSecond() const
    { return !p_->videoStream ? 0. :
                double(p_->videoStream->avg_frame_rate.num) / p_->videoStream->avg_frame_rate.den; }
double VideoStream::lengthInSeconds() const
    { return !p_->videoStream ? 0. :
                double(p_->videoStream->duration)
                    * p_->videoStream->time_base.num
                    / p_->videoStream->time_base.den; }
/*
MediaFileInfo VideoStream::getFileInfo() const
{
    MediaFileInfo info;
    info.filename = currentFilename();
    if (p_->videoCodecCtx)
    {
        info.fps = framesPerSecond();
        info.width = p_->videoCodecCtx->width;
        info.height = p_->videoCodecCtx->height;
        info.isVideo = true;
        info.lengthSeconds = lengthInSeconds();
    }
    return info;
}
*/

void VideoStream::setThreadCount(int num) { p_->threadCount = std::max(0, num); }

void VideoStream::openFile(const std::string& url) { p_->openFile(url); }
void VideoStream::close() { p_->close(); }
void VideoStream::rewind() { seekSecond(0.); }

void VideoStream::seekKeyframe(double sec)
{
    if (!p_->formatCtx || !p_->videoStream)
        return;
    int64_t pos = sec * p_->videoStream->time_base.den
                        / p_->videoStream->time_base.num;
    CHECK_FFM_THROW( av_seek_frame(p_->formatCtx, p_->videoStreamIndex, pos, 0) );
}
void VideoStream::seekSecond(double sec)
{
    if (!p_->formatCtx || !p_->videoStream)
        return;
    int64_t pos = sec * p_->videoStream->time_base.den
                        / p_->videoStream->time_base.num;
    CHECK_FFM_THROW(
                // XXX function is not part of ABI yet
                avformat_seek_file(p_->formatCtx, p_->videoStreamIndex,
                                   pos,pos,pos, AVSEEK_FLAG_ANY));
}


std::string VideoStream::toString() const
{
    std::stringstream s;
    s << "[" << p_->url << "]";
    if (p_->formatCtx)
        s << "\nnum streams " << p_->formatCtx->nb_streams;
//      s << "\ncodec " << p_->formatCtx->video_codec_id
//        << "\nbitrate " << p_->formatCtx->bit_rate
    if (p_->videoStream)
        s << "\nvideo start_time " << p_->videoStream->start_time
          << "\nvideo time_base " << p_->videoStream->time_base.num
                                 << "/" << p_->videoStream->time_base.den
                                    ;
    if (p_->videoCodecCtx)
        s << "\npixel format " << av_get_pix_fmt_name(p_->videoCodecCtx->pix_fmt)
             ;

    return s.str();
}

void VideoStream::Private::openFile(const std::string& fn)
{
    initFfmpeg();

    if (p->isReady())
        p->close();

    videoFramesDecoded = 0;

    try
    {
        // TODO set seek2any option
        CHECK_FFM_THROW( avformat_open_input(&formatCtx, fn.c_str(), NULL, NULL) );

        CHECK_FFM_THROW( avformat_find_stream_info(formatCtx, NULL) );
        //av_dump_format(formatCtx, 0, fn.c_str(), 0);

        AVCodecContext* vcCtx = 0;

        for (unsigned i = 0; i < formatCtx->nb_streams; ++i)
        {
            if (formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                videoStreamIndex = i;
                videoStream = formatCtx->streams[i];
                vcCtx = videoStream->codec;
            }
        }

        if (!videoStream)
            MO_ERROR("No video stream in '" << fn << "'");

        // get video codec
        videoCodec = avcodec_find_decoder(vcCtx->codec_id);
        if (!videoCodec)
            MO_ERROR("Unsupported codec '" << videoCodecCtx->codec_id);

        // create a useable copy
        videoCodecCtx = avcodec_alloc_context3(videoCodec);
        if (!videoCodecCtx)
            MO_ERROR("Can not allocate codec context");
        CHECK_FFM_THROW( avcodec_copy_context(videoCodecCtx, vcCtx) );

        videoCodecCtx->thread_count = threadCount;
        CHECK_FFM_THROW( avcodec_open2(videoCodecCtx, videoCodec, NULL) );

        // init frame buffer
        videoFrame = av_frame_alloc();
        if (!videoFrame)
            MO_ERROR("Failed to alloc video frame");

        url = fn;
        lastDecodedPts = -1.;

        MO_DEBUG( p->toString() );

        if (videoCodecCtx->pix_fmt != AV_PIX_FMT_YUV420P)
        {
            MO_ERROR("Video's pixel format is "
                     << av_get_pix_fmt_name(videoCodecCtx->pix_fmt)
                     //<< ".\nDecoding will be more efficient with yuv420p!"
                     << ".\nCurrently only yuv420p is supported."
                          );
        }

    }
    catch (...)
    {
        close();
        throw;
    }
}

void VideoStream::Private::close()
{
    if (packet)
        av_packet_unref(packet);
    delete packet; packet = 0;

    if (videoFrame)
        av_frame_free(&videoFrame); // sets NULL
    if (videoConvFrame)
        av_frame_free(&videoConvFrame); // sets NULL

    if (swsContext)
        sws_freeContext(swsContext);
    swsContext = 0;

    if (videoCodecCtx)
    {
        avcodec_close(videoCodecCtx);
        avcodec_free_context(&videoCodecCtx); // sets NULL
    }

    videoCodec = 0;
    videoStream = 0;
    videoStreamIndex = -1;

    if (formatCtx)
        avformat_close_input(&formatCtx); // sets NULL

    url.clear();
}

DecoderFrame* VideoStream::getVideoFrame()
{
    DecoderFrame* frame = nullptr;

    // init packet once
    if (!p_->packet)
    {
        p_->packet = new AVPacket;
        memset(p_->packet, 0, sizeof(AVPacket));
        av_init_packet(p_->packet);
    }
    int frameFinished = 0;
    // loop through stream
    while (!frameFinished && av_read_frame(p_->formatCtx, p_->packet) >= 0)
    {
        if (p_->packet->stream_index == p_->videoStreamIndex)
        {
            int bytes = avcodec_decode_video2(p_->videoCodecCtx, p_->videoFrame,
                                  &frameFinished, p_->packet);
            if (bytes < 0)
                MO_ERROR("Error while decoding video: "
                              << errorString(bytes));

            if (frameFinished)
                frame = p_->createVideoFrame();
        }

        //av_packet_
        av_packet_unref(p_->packet);
    }

    // read past end to get all frames
    if (!frame && (p_->videoCodec->capabilities & AV_CODEC_CAP_DELAY))
    {
        AVPacket packet;
        packet.data = NULL;
        packet.size = 0;
        av_init_packet(&packet);

        frameFinished = 0;
        int bytes;
        do
        {
            bytes = avcodec_decode_video2(p_->videoCodecCtx, p_->videoFrame,
                                  &frameFinished, &packet);
            if (bytes < 0)
                MO_ERROR("Error while decoding final video frames: "
                              << errorString(bytes));

            if (frameFinished)
                frame = p_->createVideoFrame();
        } while (!frameFinished && bytes > 0);
    }

    return frame;
}

DecoderFrame* VideoStream::Private::createVideoFrame()
{
    double pts = double(av_frame_get_best_effort_timestamp(videoFrame))
            * videoStream->time_base.num / videoStream->time_base.den;
    //MCW_INFO("GOT " << pts);
#if 0
    MCW_DEBUG("frame width: " << videoFrame->width
              << " ls " <<videoFrame->linesize[0]
              << " " << videoFrame->linesize[1]
              << " " << videoFrame->linesize[2]
                 );
#endif

    AVFrame* theFrame = videoFrame;
    // convert pixel format?
    if (videoFrame->format != AV_PIX_FMT_YUV420P)
    {
        // create resampling context
        if (!swsContext)
        {
            MO_DEBUG("Creating swscale context");
            swsContext = sws_getCachedContext(
                swsContext,
                videoCodecCtx->width,
                videoCodecCtx->height,
                AVPixelFormat(videoFrame->format),
                videoCodecCtx->width,
                videoCodecCtx->height,
                AV_PIX_FMT_YUV420P,
                SWS_FAST_BILINEAR, NULL, NULL, NULL);
        }

        // init conversion buffer
        if (!videoConvFrame)
        {
            MO_DEBUG("creating conversion frame ");

            videoConvFrame = av_frame_alloc();
            if (!videoConvFrame)
                MO_ERROR("Failed to alloc conversion video frame");
            //videoConvFrame->channels = 3;
            videoConvFrame->width = videoCodecCtx->width;
            videoConvFrame->height = videoCodecCtx->height;

#if 1
            CHECK_FFM_THROW( av_image_alloc(
                                 videoConvFrame->data,
                                 videoConvFrame->linesize,
                                 videoCodecCtx->width, videoCodecCtx->height,
                                 AV_PIX_FMT_YUV420P, 16) );
#else
            size_t numBytes = avpicture_get_size(
                        AV_PIX_FMT_YUV420P,
                        videoCodecCtx->width, videoCodecCtx->height);
            auto buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
            CHECK_FFM_THROW( avpicture_fill(
                                 (AVPicture*)videoConvFrame, buffer,
                                 AV_PIX_FMT_YUV420P, videoCodecCtx->width, videoCodecCtx->height
                                 )
                             );
#endif
        }

        MO_DEBUG("resampling frame " << getFrameDescription(videoFrame));
        sws_scale(swsContext, (uint8_t const * const *)videoFrame->data,
                  videoFrame->linesize, 0, videoCodecCtx->height,
                  videoConvFrame->data, videoConvFrame->linesize);

        theFrame = videoConvFrame;
        MO_DEBUG("done " << getFrameDescription(theFrame));
    }

    // -- create own frame structure and copy data --

    bool isConsec = lastDecodedPts < 0.
            // one frame apart?
            || std::abs(pts - lastDecodedPts) < 1.1 / p->framesPerSecond();

    /** @todo using linesize instead of width for
        initializing the framesize is not elegant.
        Better to implement a strided version of DecoderFrame::fillPlaneX() */
    auto f = new DecoderFrame(
                theFrame->linesize[0],
                theFrame->height,
                videoFramesDecoded++,
                pts,
                isConsec
                );
    lastDecodedPts = pts;


#ifndef MCW_PLANE_UNIFY
    f->fillPlaneY(videoFrame->data[0]);
    f->fillPlaneU(videoFrame->data[1]);
    f->fillPlaneV(videoFrame->data[2]);
#else
    f->fillPlaneY(videoFrame->data[0]);
    f->fillPlaneUV(videoFrame->data[1], videoFrame->data[2]);
#endif

    return f;
}

std::string VideoStream::Private::getFrameDescription(const AVFrame* f) const
{
    std::stringstream s;
    s << f->width << "x" << f->height << "x" << f->channels
      << ", " << av_get_pix_fmt_name(AVPixelFormat(f->format))
      << ", ls=";
    for (int i=0; i<AV_NUM_DATA_POINTERS && f->linesize[i] > 0; ++i)
    {
        if (i != 0)
            s << ",";
        s << f->linesize[i];
    }
    //s << ", pts=" << f->pts;
    return s.str();
}



} // namespace FFM

#endif // #ifdef MO_ENABLE_FFMPEG
