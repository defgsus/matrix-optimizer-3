/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/17/2015</p>
*/

#include <cassert>
#include <cmath>
#include <thread>
#include <mutex>
#include <list>
#include <set>
#include <map>
#include <atomic>
#include <algorithm>
#include <iomanip>

#include "DecoderThread.h"
//#include "audio/audiothread.h"
#ifndef MO_ENABLE_FFMPEG
#   include "decoder.h" // XXX
#else
#   include "ffm/VideoStream.h"
#endif
#include "tool/MutexLocker.h"
//#include "DecoderFrame.h"
#include "io/time.h"
#include "io/error.h"
#include "io/log.h"
#ifdef MO_ENABLE_FPS_TRACE
#   include "tool/valuebuffer.h"
#endif

#ifdef _MSC_VER
// argument conversion, possible loss of data
#   pragma warning(disable : 4244)
#endif

#if 0
#   define MO_DEBUG_DT(arg__) MO_DEBUG("DecoderThread::" << arg__)
#else
#   define MO_DEBUG_DT(unused__) { }
#endif

#if 0
#   define MO_DEBUG_DT2(arg__) MO_DEBUG("DecoderThread::" << arg__)
#else
#   define MO_DEBUG_DT2(unused__) { }
#endif




//MO_BEGIN_NAMESPACE

struct DecoderThread::Private
{
    Private(DecoderThread * p)
        : p                 (p)
#ifndef MO_ENABLE_FFMPEG
        , decoder           (new Decoder())
#else
        , decoder           (new FFM::VideoStream())
#endif
        , thread            (nullptr)
        //, audioThread       (nullptr)
        , curBuffer         (nullptr)
        , maxBufferFrames   (1000)
        , maxBufferBytes    (1 * 1024 * 1024 * 1024)
        , isRunning         (false)
        , doStop            (false)
        , hasVideoEnded     (false)
        , videoFps          (0.)
        , videoLength       (0.)
        , seekAvailTime     (-1.)
        , decodingFps       (0.)
        , decodingFpsAv     (0.)
    { }

    ~Private()
    {
        delete decoder;
    }

    struct BufferedFrames;

    void mainLoop();
    void clearBuffer(BufferedFrames*);
    bool deleteUnusedFrame(BufferedFrames*);
    void deleteUnusedFrames(BufferedFrames*);
    void getFrames(BufferedFrames*, double pts,
                   DecoderFrame** frameA, DecoderFrame** frameB, double* mix);

    DecoderThread * p;
#ifndef MO_ENABLE_FFMPEG
    Decoder * decoder;
#else
    FFM::VideoStream * decoder;
#endif
    std::thread * thread;
    std::mutex decoderMutex, framesMutex;
    //AUDIO::AudioThread* audioThread;

    struct BufferedFrame
    {
        DecoderFrame* frame;
        int refCount;
        bool freeToRemove;
    };

    struct BufferedFrames
    {
        std::map<int64_t, BufferedFrame*> frames;
        std::atomic<double>
            minBufferTime,
            maxBufferTime;
        std::atomic<size_t>
            numBufferedFrames,
            numBufferedBytes;
        volatile bool
            framesChanged,
            anyFrameReferenced;
    };
    /** Currently filled/served buffer */
    BufferedFrames* curBuffer;

    /** Maximum number of pre-buffered frames */
    size_t  maxBufferFrames,
    /** Maximum number of bytes of pre-buffered frames */
            maxBufferBytes;

    volatile bool
            isRunning,
            doStop,
            hasVideoEnded;

    // -- local copies from Decoder --
    std::atomic<double>
        videoFps, videoLength,
        seekAvailTime;
    std::atomic<int> currentWidth, currentHeight;
    std::atomic<double> decodingFps, decodingFpsAv;
#ifdef MO_ENABLE_FPS_TRACE
    ValueBuffer fpsTrace, fpsTraceAv;
#endif
};

DecoderThread::DecoderThread()
    : p_        (new Private(this))
{
    p_->curBuffer = new Private::BufferedFrames();
    p_->clearBuffer(p_->curBuffer);
}

DecoderThread::~DecoderThread()
{
    MO_DEBUG("DecoderThread::~DeoderThread()");
    stop(true);
    p_->clearBuffer(p_->curBuffer);
    delete p_->curBuffer;
    delete p_;
}

bool DecoderThread::isReady() const { return p_->decoder->isReady(); }
bool DecoderThread::isRunning() const { return p_->isRunning; }
size_t DecoderThread::getNumFramesInBuffer() const
{ return p_->curBuffer->numBufferedFrames; }
size_t DecoderThread::getNumBytesInBuffer() const
{ return p_->curBuffer->numBufferedBytes; }
double DecoderThread::getMinBufferTime() const
{ return p_->curBuffer->minBufferTime; }
double DecoderThread::getMaxBufferTime() const
{ return p_->curBuffer->maxBufferTime; }
double DecoderThread::getSeekAvailableTime() const { return p_->seekAvailTime; }
bool DecoderThread::hasVideoEnded() const { return p_->hasVideoEnded; }

int DecoderThread::currentWidth() const { return p_->currentWidth; }
int DecoderThread::currentHeight() const { return p_->currentHeight; }

bool DecoderThread::hasAudioTrack() const { return p_->decoder->hasAudioTrack(); }
double DecoderThread::decodingFps() const { return p_->decodingFps; }

#ifdef MO_ENABLE_FPS_TRACE
ValueBuffer DecoderThread::getFpsTrace() const
{
    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:getFpsTrace");
    return p_->fpsTrace;
}
ValueBuffer DecoderThread::getFpsTraceAv() const
{
    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:getFpsTraceAv");
    return p_->fpsTraceAv;
}
#endif

/*MediaFileInfo DecoderThread::getFileInfo() const
{
    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:getFileInfo");
    auto info = p_->decoder->getFileInfo();
    return info;
}*/

void DecoderThread::clearSeekAvailableTime() { p_->seekAvailTime = -1.; }
void DecoderThread::setThreadCount(int num) { p_->decoder->setThreadCount(num); }

/*void DecoderThread::setAudioThread(AUDIO::AudioThread* t)
{
    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:setAudioThread");
    p_->audioThread = t;
}*/


void DecoderThread::start()
{
    MO_DEBUG_DT("start() "
                 << "isRunning=" << p_->isRunning
                 << " doStop=" << p_->doStop
                 << " thread=" << p_->thread);

    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:start");

    // XXX There is a race condition somewhere
    if (p_->thread && !p_->isRunning)
    {
        MO_ERROR("RACE CONDITION in DecoderThread::start()");
        return;
    }

    // if no stop request is pending, keep running
    if (p_->isRunning && !p_->doStop)
        return;

    // wait for earlier thread to finish
    if (p_->thread)
    {
        if (p_->thread->joinable())
        {
            MO_DEBUG_DT("start() join previous");
            p_->thread->join();
        }
        delete p_->thread;
    }

    p_->clearBuffer(p_->curBuffer);

    // run Private::mainLoop()
    p_->thread = new std::thread([=]()
    {
        p_->mainLoop();
        //delete p_->thread; p_->thread = 0;
    });
}

void DecoderThread::stop(bool blocking)
{
    MO_DEBUG_DT("stop(" << blocking << ")");

    if (!p_->isRunning || !p_->thread || p_->doStop)
        return;

    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:stop");

    p_->doStop = true;

    if (blocking && p_->thread->joinable())
    {
        MO_DEBUG_DT("stop(" << blocking << ") join");
        p_->thread->join();
    }

    delete p_->thread;
    p_->thread = 0;
}

size_t DecoderThread::currentFilePos() const
{
    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:getCurrentFilePos");
    return p_->decoder->curFilePos();
}

#ifndef MO_ENABLE_FFMPEG
void DecoderThread::seek(size_t filePos)
{
    MO::MutexLocker lock(p_->decoderMutex);
    p_->decoder->seek(filePos);
}
#endif

void DecoderThread::seekSecond(double sec)
{
    MO_DEBUG_DT("seekSecond(" << sec << ")");
    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:seekSecond");
    p_->decoder->seekSecond(sec);
}

void DecoderThread::seekKeyframe(double sec)
{
    MO_DEBUG_DT("seekKeyframe(" << sec << ")");
    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:seekKeyframe");
    p_->decoder->seekKeyframe(sec);
}

void DecoderThread::rewind()
{
    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:rewind");
    p_->decoder->rewind();
}

bool DecoderThread::openFile(const std::string& fn)
{
    try
    {
        MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:openFile");
        //p_->clearBuffer();
        doneFrames();
        //if (p_->audioThread)
        //    p_->audioThread->reset();
        p_->decoder->openFile(fn.c_str());
        p_->hasVideoEnded = false;
        return p_->decoder->isReady();
    }
    catch (MO::Exception& e)
    {
        MO_ERROR(e.what());
        return false;
    }
}

void DecoderThread::close()
{
    MO::MutexLocker lock(p_->decoderMutex, "DT:decoderMutex:close");
    p_->decoder->close();
}

const std::string& DecoderThread::currentFilename() const
{
    return p_->decoder->currentFilename();
}

void DecoderThread::Private::clearBuffer(BufferedFrames* buf)
{
    for (auto& f : buf->frames)
    {
        delete f.second->frame;
        delete f.second;
    }
    buf->frames.clear();
    buf->numBufferedFrames = 0;
    buf->numBufferedBytes = 0;
    buf->minBufferTime = buf->maxBufferTime = -1.;
    buf->framesChanged = true;
    buf->anyFrameReferenced = false;
}

void DecoderThread::Private::mainLoop()
{
    MO_DEBUG("DecoderThread:: thread start");

    doStop = false;
    isRunning = true;
    hasVideoEnded = false;

    while (!doStop)
    {
        // ---- test if buffer has space ----

        if (curBuffer->numBufferedFrames >= maxBufferFrames
         || curBuffer->numBufferedBytes >= maxBufferBytes)
        {
            while ((  curBuffer->numBufferedFrames+1 >= maxBufferFrames
                   || curBuffer->numBufferedBytes+1024*1024 >= maxBufferBytes)
                   && deleteUnusedFrame(curBuffer));

            // sleep to avoid spinning at 100%
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }

        // break as soon as possible
        if (doStop || !decoder->isReady())
            break;

        // -------- get a frame --------

        MO::MutexLocker decoderLock(decoderMutex, "DT:decoderMutex:get-frame");

        MO::TimeMessure tm;

#ifndef MO_ENABLE_FFMPEG
        if (!decoder->decodeFrame())
        {
            // XXX try from beginning
            //if (!(decoder->rewind() && decoder->decodeFrame()))
            {
                decoderLock.unlock();
                // sleep to avoid spinning at 100%
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }
        }
#else
        DecoderFrame* frame;
        try
        {
            frame = decoder->getVideoFrame();
        }
        catch (const MO::Exception& e)
        {
            MO_ERROR("DecoderThread: " << e.what());
            decoderLock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }

        // did not receive a frame?
        if (!frame)
        {
            decoderLock.unlock();
            hasVideoEnded = true;
            // break as soon as possible
            if (doStop)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }
#endif
        // break as soon as possible
        if (doStop)
            break;

        // --- buffer the decoded frame ---
        {
            frame->convertToYUV(); // XXX Must be configurable in class interface

            videoFps = decoder->framesPerSecond();
            videoLength = decoder->lengthInSeconds();
#ifndef MO_ENABLE_FFMPEG
            int64_t fnum = decoder->currentFrameNumber();
            double fpsInv = decoderFps != 0.
                    ? 1. / decoderFps
                    : 1.;

            // copy data
            auto frame = new DecoderFrame(decoder->currentFrame(),
                                          fnum,
                                          fpsInv * fnum);
#endif
            currentWidth = frame->width();
            currentHeight = frame->height();

            // copy audio frames
            if (decoder->numAvailableAudioSeconds() > 1.)
            while (decoder->hasAudioFrame())
            {
                /*if (audioThread)
                {
                    // send audio frames to device
                    audioThread->pushFrame(decoder->getAudioFrame());
                }
                else*/
                    delete decoder->getAudioFrame();
            }

            // messure decoding speed (incl. data copying)
            {
                double elapsed = tm.time();
                if (elapsed > 0.)
                {
                    decodingFps = 1. / elapsed;
                    decodingFpsAv =
                        decodingFpsAv + 1./60. * (decodingFps - decodingFpsAv);
#ifdef MO_ENABLE_FPS_TRACE
                    fpsTrace.addValue(decodingFps);
                    fpsTraceAv.addValue(decodingFpsAv);
#endif
                }
            }

            decoderLock.unlock();

            //MO_PRINT("GOT " << frame->presentationTime());

            // keep in list
            {
                MO::MutexLocker lock(framesMutex, "DT:framesMutex:insert-frame");

                int64_t ipts = int64_t(frame->presentationTime() * 1024);
                // throw away if already in buffer
                // XXX Note: should be done before
                // creation of DecoderFrame instance
                // But locking needs some thinking..
                // (And it's very unlikely we get the same frame twice)
                if (curBuffer->frames.find(ipts) != curBuffer->frames.end())
                {
                    delete frame;
                    continue;
                }
                auto bframe = new BufferedFrame;
                bframe->frame = frame;
                bframe->refCount = 0;
                bframe->freeToRemove = false;
                curBuffer->frames.insert(std::make_pair(ipts, bframe));
                // suppose first frame after seek
                if (!bframe->frame->isConsecutive())
                    seekAvailTime = bframe->frame->presentationTime();
                curBuffer->framesChanged = true;

                curBuffer->numBufferedFrames = curBuffer->numBufferedFrames + 1;
                curBuffer->numBufferedBytes += frame->memory();
                if (curBuffer->minBufferTime < 0.)
                    curBuffer->minBufferTime = frame->presentationTime();
                else
                    curBuffer->minBufferTime = std::min(
                        (double)curBuffer->minBufferTime,
                                frame->presentationTime());
                if (curBuffer->maxBufferTime < 0.)
                    curBuffer->maxBufferTime = frame->presentationTime();
                else
                    curBuffer->maxBufferTime = std::max(
                        (double)curBuffer->maxBufferTime,
                                frame->presentationTime());

            }
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));

            MO_DEBUG_DT2("buffered frame "
                       << numBufferedFrames << "/" << maxBufferFrames
                       << ": " << frame->width() << "x" << frame->height()
                       << "@" << decoder->framesPerSecond() << "fps, num="
                       << frame->frameNumber() << ", pts="
                       << frame->presentationTime() << "sec");
        }
    }

    MO_DEBUG("DecoderThread:: thread end");
    isRunning = false;
}

bool DecoderThread::Private::deleteUnusedFrame(BufferedFrames* buf)
{
    MO::MutexLocker lock(framesMutex, "DT:framesMutex:deleteUnusedFrame");

    for (auto i = buf->frames.begin(); i != buf->frames.end(); ++i)
    if (i->second->freeToRemove)
    {
        MO_DEBUG_DT2("deleting frame: "
                   << i->second->frame->frameNumber() << " "
                   << i->second->frame->presentationTime());

        buf->numBufferedBytes -= i->second->frame->memory();
        delete i->second->frame;
        delete i->second;
        buf->frames.erase(i);

        // update min/max buffer time
        if (buf->frames.empty())
            buf->minBufferTime = buf->maxBufferTime = -1.;
        else
        {
            buf->minBufferTime = buf->frames.begin()->second->frame->presentationTime();
            buf->maxBufferTime = buf->frames.rbegin()->second->frame->presentationTime();
        }
        buf->framesChanged = true;

        buf->numBufferedFrames = buf->frames.size();

        return true;
    }
    return false;
}

void DecoderThread::Private::deleteUnusedFrames(BufferedFrames* buf)
{
    MO::MutexLocker lock(framesMutex, "DT:framesMutex:deleteUnusedFrames");

    auto tframes = buf->frames;
    buf->frames.clear();
    for (auto i = tframes.begin(); i != tframes.end(); ++i)
    if (!i->second->freeToRemove)
    {
        buf->frames.insert(std::make_pair(i->first, i->second));
    }
    else
        buf->framesChanged = true;

    // dispose memory
    for (auto i = tframes.begin(); i != tframes.end(); ++i)
        if (i->second->freeToRemove)
            { delete i->second->frame; delete i->second; }

    // update min/max buffer time
    if (buf->frames.empty())
        buf->minBufferTime = buf->maxBufferTime = -1.;
    else
    {
        buf->minBufferTime = buf->frames.begin()->second->frame->presentationTime();
        buf->maxBufferTime = buf->frames.rbegin()->second->frame->presentationTime();
    }
    buf->numBufferedFrames = buf->frames.size();
    buf->numBufferedBytes = 0;
    for (auto& f : buf->frames)
        buf->numBufferedBytes += f.second->frame->memory();
}


double DecoderThread::framesPerSecond() const { return p_->videoFps; }
double DecoderThread::lengthInSeconds() const { return p_->videoLength; }

#if 0
DecoderFrame* DecoderThread::getFrame(double pts)
{
    DecoderFrame *frameA, *frameB;
    double mix;

    getFrames(pts, &frameA, &frameB, &mix);
    //if (frameB)
    //    doneFrame(frameB);
    return frameA;
}
#endif

void DecoderThread::getFrames(
        double pts,
        DecoderFrame** frameA, DecoderFrame** frameB, double* mix)
{

    double len = p_->decoder->lengthInSeconds();
    (void)len;
#if 0
    // loop time
    if (len > 0. && pts > len)
        pts = std::fmod(pts, len);
#endif


    MO::MutexLocker lock(p_->framesMutex, "DT:framesMutex:get-frame-from-buffer");

    p_->getFrames(p_->curBuffer, pts, frameA, frameB, mix);
}

void DecoderThread::Private::getFrames(
        BufferedFrames* buf, double pts,
        DecoderFrame** frameA, DecoderFrame** frameB, double* mix)
{
    int64_t ipts = pts * 1024;

    // find the first frame
    auto i = buf->frames.lower_bound(ipts),
         j = buf->frames.end();
    if (i != buf->frames.end()
        //&& !i->second->freeToRemove
        && i->second->frame->presentationTime() >= pts-1./1000.)
    {
        //MO_PRINT("pts " << pts << ", frame " << i->second->frame->presentationTime());
        // see if we need one frame earlier
        // (pts is between this and previous)
        if (i->second->frame->presentationTime() > pts)
        {
            j = i; --j;
            if (// no earlier frame
                i == buf->frames.begin()
                // or does not belong to this frame
             || (std::abs(j->second->frame->presentationTime()
                          - i->second->frame->presentationTime()))
                 > 1.1 / p->framesPerSecond())
            {

                MO_DEBUG_DT("Requested frame is earlier than buffer at "
                          << pts << "s, buffer has " << buf->frames.size()
                          << " frames [" << buf->minBufferTime
                          << ", " << buf->maxBufferTime << "] "
                          "video length = " << len);
                *frameA = *frameB = 0;
                *mix = 0.;
                return;
            }

            j = i; // got second frame
            --i;
            //MO_PRINT("one earlier pts " << pts << ", frame " << i->second->frame->presentationTime());
        }
        else
        {
            // find second frame
            j = i;
            ++j;
        }

        // get frameA
        *frameA = i->second->frame;
        i->second->freeToRemove = false;
        ++(i->second->refCount);
        buf->anyFrameReferenced = true;
        if (j == buf->frames.end()
          || (std::abs(j->second->frame->presentationTime()
                       - i->second->frame->presentationTime()))
              > 1.1 / p->framesPerSecond())
        {
            *frameB = 0;
            *mix = 0;
            return;
        }
        // get frameB
        *frameB = j->second->frame;
        j->second->freeToRemove = false;
        ++(j->second->refCount);

        // calculate mixing value
        *mix = (pts - (*frameA)->presentationTime()) * p->framesPerSecond();
        // consecutive frames with frameNumber difference > 1
        // means frames got skipped by decoder
        int fd = (*frameB)->frameNumber() - (*frameA)->frameNumber();
        if (fd > 1)
        {
            MO_DEBUG_DT("buffer has hole "
                      << (*frameA)->frameNumber() << " -> "
                      << (*frameB)->frameNumber()
                      << ", pts=" << pts);
            *mix /= fd;
        }
        // safety check
        if (*mix < 0. || *mix > 1.)
        {
            MO_DEBUG_DT("mix value out of range " << *mix
                      << ", pts=" << pts
                      << ", frameA=" << (*frameA)->presentationTime()
                      << ", frameB=" << (*frameB)->presentationTime());
            *mix = std::max(0.,std::min(1., *mix ));
        }
        return;
    }
    *frameA = *frameB = 0;
    *mix = 0.;

    MO_DEBUG_DT("Request for unbuffered frame at "
               << pts << "s, buffer has " << buf->frames.size()
               << " frames [" << buf->minBufferTime
               << ", " << buf->maxBufferTime << "] "
               "video length = " << len);
}

void DecoderThread::doneFrames()
{
    MO_DEBUG_DT("doneFrames()");

    MO::MutexLocker lock(p_->framesMutex, "DT:framesMutex:doneFrames");

    auto buf = p_->curBuffer;
    for (auto i = buf->frames.begin(); i != buf->frames.end(); ++i)
    {
        i->second->freeToRemove = true;
    }
}

void DecoderThread::doneFramesBefore(double presentationTime)
{
    MO_DEBUG_DT2("doneFramesBefore(" << presentationTime << ")");

    // key
    //int64_t ipts = presentationTime * 1024;

    {
        MO::MutexLocker lock(p_->framesMutex, "DT:framesMutex:doneFramesBefore");

        auto buf = p_->curBuffer;

        // flag all frames before pts
        for (auto i = buf->frames.begin();
             i != buf->frames.end(); ++i)
        {
            if (i->second->frame->presentationTime() < presentationTime)
            {
                MO_DEBUG_DT2("done frame " << i->second->frame->presentationTime());
                i->second->freeToRemove = true;
            }
            else
                break;
        }
    }
}


void DecoderThread::dumpFrames()
{
    MO::MutexLocker lock(p_->framesMutex, "DT:framesMutex:dumpFrames");

    MO_PRINT(std::setw(4) << "#"
              << std::setw(7) << "frame"
              << std::setw(11) << "pts"
              << std::setw(9) << "refcount"
              << std::setw(5) << "done"
              );

    //auto ti = systemTime();

    int k = 0;
    for (auto i : p_->curBuffer->frames)
    {
        Private::BufferedFrame* f = i.second;
        (void)f;
        MO_DEBUG(std::setw(4) << k
                  << std::setw(7) << f->frame->frameNumber()
                  << std::setw(11) << double(i.first) / 1024.
                  << std::setw(9) << f->refCount
                  << std::setw(5) << f->freeToRemove
                  );
        ++k;
    }
}


//MO_END_NAMESPACE
