#ifdef MO_ENABLE_FFMPEG

#include <sstream>
#include <algorithm>
#include <queue>

#include "VideoStream.h"
#include "ffmpeg.h"
#include "io/error.h"
#include "io/log.h"

namespace FFM {

struct VideoStream::Private
{
    Private(VideoStream * p)
        : p             (p)
        , threadCount   (0)
        , isAudioEnabled_(true)
        , formatCtx     (0)
        , videoCodecCtx (0)
        , audioCodecCtx (0)
        , videoStream   (0)
        , audioStream   (0)
        , videoStreamIndex(-1)
        , audioStreamIndex(-1)
        , videoFramesDecoded (0)
        , videoCodec    (0)
        , audioCodec    (0)
        , swsContext    (0)
        , videoFrame    (0)
        , videoConvFrame(0)
        , audioFrame    (0)
        , packet        (0)
        , lastDecodedPts(-1.)
        , numBufferedAudioSeconds(0.)
    { }

    void openFile(const std::string& fn);
    void close();
    void pushAudioFrame();
    void clearAudioQueue();
    DecoderFrame* createVideoFrame();
    std::string getFrameDescription(const AVFrame*) const;

    VideoStream * p;

    std::string url;
    int threadCount;
    bool isAudioEnabled_;

    AVFormatContext* formatCtx;
    AVCodecContext* videoCodecCtx, *audioCodecCtx;
    AVStream* videoStream, *audioStream;
    int videoStreamIndex, audioStreamIndex;
    int64_t videoFramesDecoded;
    AVCodec* videoCodec, *audioCodec;
    SwsContext* swsContext;

    AVFrame* videoFrame, *videoConvFrame, *audioFrame;
    AVPacket* packet;

    std::queue<AudioFrame*> audioFrames;

    double  lastDecodedPts,
            numBufferedAudioSeconds;
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
bool VideoStream::isAudioEnabled() const { return p_->isAudioEnabled_; }
bool VideoStream::hasAudioFrame() const { return !p_->audioFrames.empty(); }
double VideoStream::numAvailableAudioSeconds() const
    { return p_->numBufferedAudioSeconds; }
const std::string& VideoStream::currentFilename() const { return p_->url; }
size_t VideoStream::curFilePos() const
    { return p_->packet ? p_->packet->pos >= 0 ? p_->packet->pos : 0 : 0; }
int64_t VideoStream::numDecodedFrames() const { return p_->videoFramesDecoded; }
double VideoStream::framesPerSecond() const
    { return !p_->videoStream ? 0. :
                double(p_->videoStream->avg_frame_rate.num) / p_->videoStream->avg_frame_rate.den; }
double VideoStream::lengthInSeconds() const
{
    return !p_->videoStream
            ? 0. : p_->videoStream->duration != int64_t(0x8000000000000000)
                ? double(p_->videoStream->duration)
                    * p_->videoStream->time_base.num
                    / p_->videoStream->time_base.den
                : -1.; /*< XXX fallback value*/
}

/*MediaFileInfo VideoStream::getFileInfo() const
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
        info.pixFormatString = pixelFormatString();
        info.videoFormatString = videoFormatString();
    }
    if (p_->audioCodecCtx)
    {
        info.audioBitrate = p_->audioCodecCtx->sample_rate;
        info.audioChannels = p_->audioCodecCtx->channels;
        info.audioFormatString = audioFormatString();
        info.sampleFormatString = sampleFormatString();
    }
    if (info.lengthSeconds < 0.)
    {
        info.errorString = "undetermined length of video";
    }
    return info;
}*/

bool VideoStream::hasAudioTrack() const { return p_->audioCodecCtx; }

void VideoStream::setThreadCount(int num) { p_->threadCount = std::max(0, num); }
void VideoStream::setAudioEnabled(bool e) { p_->isAudioEnabled_ = e; }

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
        s << "\nnum streams " << p_->formatCtx->nb_streams
//      s << "\ncodec " << p_->formatCtx->video_codec_id
//        << "\nbitrate " << p_->formatCtx->bit_rate
        ;
    if (p_->videoStream)
        s << "\nvideo start_time " << p_->videoStream->start_time
          << "\nvideo time_base " << p_->videoStream->time_base.num
                                 << "/" << p_->videoStream->time_base.den
          << "\nvideo duration " << p_->videoStream->duration
        ;
    if (p_->videoCodecCtx)
        s << "\nvideo format " << videoFormatString()
          << "\npixel format " << pixelFormatString()
        ;
    if (p_->audioCodecCtx)
        s << "\naudio format " << audioFormatString()
          << "\nsample format " << sampleFormatString()
        ;
    return s.str();
}

std::string VideoStream::videoFormatString() const
{
    std::string s;
    if (!p_->videoCodecCtx)
        return s;
    const AVCodecDescriptor *desc =
            av_codec_get_codec_descriptor(p_->videoCodecCtx);
    s += desc->long_name;
    /*
    size_t size = av_get_codec_tag_string(0,0, p_->videoCodecCtx->codec_tag);
    std::string s;
    s.resize(size);
    av_get_codec_tag_string(&s[0],s.size(), p_->videoCodecCtx->codec_tag);
    */

    //        av_get_profile_name(p_->videoCodec, p_->videoCodecCtx->profile)
    //s += "/";
    //s += av_get_pix_fmt_name(p_->videoCodecCtx->pix_fmt);
    return s;
}

std::string VideoStream::pixelFormatString() const
{
    return p_->videoCodecCtx
            ? av_get_pix_fmt_name(p_->videoCodecCtx->pix_fmt)
            : "";
}

std::string VideoStream::sampleFormatString() const
{
    return p_->audioCodecCtx
            ? av_get_sample_fmt_name(p_->audioCodecCtx->sample_fmt)
            : "";
}

std::string VideoStream::audioFormatString() const
{
    std::string s;
    if (!p_->audioCodecCtx)
        return s;
    const AVCodecDescriptor *desc =
            av_codec_get_codec_descriptor(p_->audioCodecCtx);
    s += desc->long_name;
    return s;
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

#ifdef MO_FFM_NEW_VERSION
        for (unsigned i = 0; i < formatCtx->nb_streams; ++i)
        {
            formatCtx->streams[i]->codecpar = avcodec_parameters_alloc();
        }
#endif

        CHECK_FFM_THROW( avformat_find_stream_info(formatCtx, NULL) );
        //av_dump_format(formatCtx, 0, fn.c_str(), 0);

#ifndef MO_FFM_NEW_VERSION
        AVCodecContext
                *vcCtx = 0, *acCtx = 0;

        for (unsigned i = 0; i < formatCtx->nb_streams; ++i)
        {
            if (formatCtx->streams[i]->codec->codec_type
                    == AVMEDIA_TYPE_VIDEO)
            {
                videoStreamIndex = i;
                videoStream = formatCtx->streams[i];
                vcCtx = videoStream->codec;
            }
            else if (formatCtx->streams[i]->codec->codec_type
                        == AVMEDIA_TYPE_AUDIO)
            {
                audioStreamIndex = i;
                audioStream = formatCtx->streams[i];
                acCtx = audioStream->codec;
            }
        }
#else
        AVCodecID vcId = AV_CODEC_ID_NONE,
                  acId = AV_CODEC_ID_NONE;
        for (unsigned i = 0; i < formatCtx->nb_streams; ++i)
        {
            if (!formatCtx->streams[i]->codecpar)
            {
                //MO_ERROR("codecpar is NULL in stream "
                //          << i << "/" << formatCtx->nb_streams);
                continue;
            }
            if (formatCtx->streams[i]->codecpar->codec_type
                    == AVMEDIA_TYPE_VIDEO)
            {
                videoStreamIndex = i;
                videoStream = formatCtx->streams[i];
                vcId = videoStream->codecpar->codec_id;
            }
            else if (formatCtx->streams[i]->codecpar->codec_type
                        == AVMEDIA_TYPE_AUDIO)
            {
                audioStreamIndex = i;
                audioStream = formatCtx->streams[i];
                acId = audioStream->codecpar->codec_id;
            }
        }
#endif

        // ---- init video ----

        if (!videoStream)
            MO_ERROR("No video stream in '" << fn << "'");

        // get video codec
#ifndef MO_FFM_NEW_VERSION
        videoCodec = avcodec_find_decoder(vcCtx->codec_id);
#else
        videoCodec = avcodec_find_decoder(vcId);
#endif
        if (!videoCodec)
            MO_ERROR("Unsupported video codec '" << videoCodecCtx->codec_id);

        // create a useable copy
        videoCodecCtx = avcodec_alloc_context3(videoCodec);
        if (!videoCodecCtx)
            MO_ERROR("Can not allocate video codec context");
#ifndef MO_FFM_NEW_VERSION
        CHECK_FFM_THROW( avcodec_copy_context(videoCodecCtx, vcCtx) );
#endif
        videoCodecCtx->thread_count = threadCount;
        CHECK_FFM_THROW( avcodec_open2(videoCodecCtx, videoCodec, NULL) );

        // init frame buffer
        videoFrame = av_frame_alloc();
        if (!videoFrame)
            MO_ERROR("Failed to alloc video frame");

        // ---- init audio ----

        if (audioStream)
        {
            // get audio codec
#ifndef MO_FFM_NEW_VERSION
            audioCodec = avcodec_find_decoder(acCtx->codec_id);
#else
            audioCodec = avcodec_find_decoder(acId);
#endif
            if (!audioCodec)
                MO_ERROR("Unsupported audio codec '"
                              << audioCodecCtx->codec_id);

            // create a useable copy
            audioCodecCtx = avcodec_alloc_context3(audioCodec);
            if (!audioCodecCtx)
                MO_ERROR("Can not allocate audio codec context");
#ifndef MO_FFM_NEW_VERSION
            CHECK_FFM_THROW( avcodec_copy_context(audioCodecCtx, acCtx) );
#endif
            CHECK_FFM_THROW( avcodec_open2(audioCodecCtx, audioCodec, NULL) );

            // init frame buffer
            audioFrame = av_frame_alloc();
            if (!audioFrame)
                MO_ERROR("Failed to alloc audio frame");
        }

        // ---- setup ----

        url = fn;
        lastDecodedPts = -1.;

        MO_DEBUG( p->toString() );

        if (videoCodecCtx->pix_fmt != AV_PIX_FMT_YUV420P)
        {
            MO_WARNING("Video's pixel format is "
                     << av_get_pix_fmt_name(videoCodecCtx->pix_fmt)
                     << ".\nDecoding will be more efficient with yuv420p!"
                     //<< ".\nCurrently only yuv420p is supported."
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

    if (audioCodecCtx)
    {
        avcodec_close(audioCodecCtx);
        avcodec_free_context(&audioCodecCtx); // sets NULL
    }

    audioCodec = 0;
    audioStream = 0;
    audioStreamIndex = -1;

    if (formatCtx)
        avformat_close_input(&formatCtx); // sets NULL

    clearAudioQueue();

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
        // read video frame
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
        // read audio frame
        else if (p_->isAudioEnabled_
                 && p_->audioStreamIndex != -1
                 && p_->packet->stream_index == p_->audioStreamIndex)
        {
            int audioFrameFinished;
            int bytes = avcodec_decode_audio4(p_->audioCodecCtx, p_->audioFrame,
                                  &audioFrameFinished, p_->packet);
            if (bytes < 0)
                MO_ERROR("Error while decoding audio: "
                              << errorString(bytes));
            if (audioFrameFinished)
                p_->pushAudioFrame();
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
    //MO_INFO("GOT " << pts);
#if 0
    MO_DEBUG("frame width: " << videoFrame->width
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
            //MO_INFO("Creating swscale context");
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
            //MO_INFO("creating conversion frame ");

            videoConvFrame = av_frame_alloc();
            if (!videoConvFrame)
                MO_ERROR("Failed to alloc conversion video frame");
            //videoConvFrame->channels = 3;
            videoConvFrame->format = AV_PIX_FMT_YUV420P;
            videoConvFrame->width = videoCodecCtx->width;
            videoConvFrame->height = videoCodecCtx->height;
            CHECK_FFM_THROW( av_frame_get_buffer(videoConvFrame, 16) );
#if 0
            CHECK_FFM_THROW( av_image_alloc(
                                 videoConvFrame->data,
                                 videoConvFrame->linesize,
                                 videoCodecCtx->width, videoCodecCtx->height,
                                 AV_PIX_FMT_YUV420P, 16) );
#elif 0
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

        //MO_INFO("resampling frame " << getFrameDescription(videoFrame));
        sws_scale(swsContext, (uint8_t const * const *)videoFrame->data,
                  videoFrame->linesize, 0, videoCodecCtx->height,
                  videoConvFrame->data, videoConvFrame->linesize);

        theFrame = videoConvFrame;
        //MO_INFO("done " << getFrameDescription(theFrame));
    }

    // -- create own frame structure and copy data --

    bool isConsec = lastDecodedPts < 0.
            // one frame apart?
            || std::abs(pts - lastDecodedPts) < 1.1 / p->framesPerSecond();

    auto f = new DecoderFrame(
                theFrame->width,
                theFrame->height,
                videoFramesDecoded++,
                pts,
                isConsec
                );
    lastDecodedPts = pts;

    //MO_INFO("filling DecoderFrame " << f->toString());
#ifndef MO_PLANE_UNIFY
    f->fillPlaneY(theFrame->data[0], theFrame->linesize[0]);
    f->fillPlaneU(theFrame->data[1], theFrame->linesize[1]);
    f->fillPlaneV(theFrame->data[2], theFrame->linesize[2]);
#else
    f->fillPlaneY(theFrame->data[0], theFrame->linesize[0]);
    f->fillPlaneUV(theFrame->data[1], theFrame->data[2],
                    theFrame->linesize[1], theFrame->linesize[2]);
#endif

    return f;
}

std::string VideoStream::Private::getFrameDescription(const AVFrame* f) const
{
    std::stringstream s;
    if (f->flags)
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




// ---------- audio sample conversion ----------


namespace {

template <typename T>
struct Conversion_traits { };

template <>
struct Conversion_traits<uint8_t>
{
    static const size_t bytes = 1;
    static F32 to_float(uint8_t v) { return (F32(v) - 127.)
                / F32(std::numeric_limits<uint8_t>::max()) * 2.; }
};
template <>
struct Conversion_traits<int16_t>
{
    static const size_t bytes = 2;
    static F32 to_float(int16_t v) { return F32(v)
                / F32(std::numeric_limits<int16_t>::max()); }
};
template <>
struct Conversion_traits<int32_t>
{
    static const size_t bytes = 4;
    static F32 to_float(int32_t v) { return F32(v)
                / F32(std::numeric_limits<int32_t>::max()); }
};
template <>
struct Conversion_traits<float>
{
    static const size_t bytes = 4;
    static F32 to_float(float v) { return F32(v); }
};
template <>
struct Conversion_traits<double>
{
    static const size_t bytes = 8;
    static F32 to_float(float v) { return F32(v); }
};

/** Convert interleaved T to interleaved F32 */
template <typename T>
void convert_interleaved(F32* dst, uint8_t* srcP, size_t length)
{
    const T* src = reinterpret_cast<const T*>(srcP);
    for (size_t i=0; i<length; ++i, ++src, ++dst)
    {
        *dst = Conversion_traits<T>::to_float(*src);
    }
}

/** Convert planar T to interleaved F32 */
template <typename T>
void convert_planar(F32* dstP, uint8_t* srcPlanes[],
                    size_t length, size_t numChan)
{
    for (size_t k = 0; k < numChan; ++k)
    {
        F32* dst = dstP + k;
        const T* src = reinterpret_cast<const T*>(srcPlanes[k]);
        for (size_t i=0; i<length; ++i, ++src, dst += numChan)
        {
            *dst = Conversion_traits<T>::to_float(*src);
        }
    }
}

} // namespace

void VideoStream::Private::pushAudioFrame()
{
    const double
            pts = double(av_frame_get_best_effort_timestamp(audioFrame))
            * audioStream->time_base.num / audioStream->time_base.den;

    const size_t
            sr = av_frame_get_sample_rate(audioFrame),
            numChan = av_frame_get_channels(audioFrame),
            length = audioFrame->nb_samples;

#if 0
    MO_INFO( pts
                << " sr=" << sr
                << " chan=" << numChan
                << " length=" << length
                << " layout=" << audioFrame->channel_layout
                << " fmt=" << audioFrame->format
                );
#endif

    auto frame = new AudioFrame(
                length, numChan, sr, pts);

    switch (audioFrame->format)
    {
        case AV_SAMPLE_FMT_U8:
            convert_interleaved<uint8_t>(
                        frame->data(), audioFrame->data[0], length * numChan);
        break;
        case AV_SAMPLE_FMT_S16:
            convert_interleaved<int16_t>(
                        frame->data(), audioFrame->data[0], length * numChan);
        break;
        case AV_SAMPLE_FMT_S32:
            convert_interleaved<int32_t>(
                        frame->data(), audioFrame->data[0], length * numChan);
        break;
        case AV_SAMPLE_FMT_FLT:
            convert_interleaved<float>(
                        frame->data(), audioFrame->data[0], length * numChan);
        break;
        case AV_SAMPLE_FMT_DBL:
            convert_interleaved<double>(
                        frame->data(), audioFrame->data[0], length * numChan);
        break;

        case AV_SAMPLE_FMT_U8P:
            convert_planar<uint8_t>(
                        frame->data(), audioFrame->data, length, numChan);
        break;
        case AV_SAMPLE_FMT_S16P:
            convert_planar<int16_t>(
                        frame->data(), audioFrame->data, length, numChan);
        break;
        case AV_SAMPLE_FMT_S32P:
            convert_planar<int32_t>(
                        frame->data(), audioFrame->data, length, numChan);
        break;
        case AV_SAMPLE_FMT_FLTP:
            convert_planar<float>(
                        frame->data(), audioFrame->data, length, numChan);
        break;
        case AV_SAMPLE_FMT_DBLP:
            convert_planar<double>(
                        frame->data(), audioFrame->data, length, numChan);
        break;

    };

    numBufferedAudioSeconds += frame->lengthSeconds();
    audioFrames.push(frame);
}

AudioFrame* VideoStream::getAudioFrame()
{
    if (p_->audioFrames.empty())
        return nullptr;
    auto f = p_->audioFrames.front();
    p_->audioFrames.pop();
    p_->numBufferedAudioSeconds -= f->lengthSeconds();
    return f;
}

void VideoStream::Private::clearAudioQueue()
{
    while (!audioFrames.empty())
    {
        delete audioFrames.front();
        audioFrames.pop();
    }
    numBufferedAudioSeconds = 0.;
}


} // namespace FFM

#endif // #ifdef MO_ENABLE_FFMPEG
