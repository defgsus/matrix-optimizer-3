/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/17/2015</p>
*/

#ifndef MCWSRC_DECODERTHREAD_H
#define MCWSRC_DECODERTHREAD_H

#include <string>

//#include "config.h"
//#include "tool/mediafileinfo.h"

#ifdef MO_ENABLE_FPS_TRACE
#   include "tool/valuebuffer.h"
#endif

//namespace AUDIO { class AudioThread; }

class DecoderFrame;

//MO_BEGIN_NAMESPACE

/** Wraps an FFM::VideoStream in a separate thread and buffers
    its output.

    All member functions are threadsafe for a single user thread.

    getFrames() and doneFrames() are threadsafe for multiple user threads.

    Seeking and buffer redirection are currently under development..
*/
class DecoderThread
{
public:
    DecoderThread();
    ~DecoderThread();

    // --------- io ---------

    bool openFile(const std::string& fn);
    void close();

    // ------- getter --------

    /* Returns current mediafile info */
    //MediaFileInfo getFileInfo() const;

    /** Returns the current filename */
    const std::string& currentFilename() const;

    bool hasAudioTrack() const;

    /** Returns true if a file is open and ready to decode */
    bool isReady() const;

    /** Returns true if the decoder thread is running */
    bool isRunning() const;

    bool hasVideoEnded() const;

    /** Returns the number of decoded frames in the buffer */
    size_t getNumFramesInBuffer() const;
    /** Returns the number of decoded bytes in the buffer */
    size_t getNumBytesInBuffer() const;

    /** Returns the lowest presentation time in the buffer,
        or a negative number if no frame is in the buffer */
    double getMinBufferTime() const;

    /** Returns the highest presentation time in the buffer,
        or a negative number if no frame is in the buffer */
    double getMaxBufferTime() const;

    /** Returns the time of the first frame received after seeking,
        or a negative value if no seeking took place.
        Use clearSeekAvailableTime() after reading to reset. */
    double getSeekAvailableTime() const;
    void clearSeekAvailableTime();

    /** Returns the previous frame's decoding speed in fps */
    double decodingFps() const;
    /** Returns the running average decoding speed in fps */
    double decodingFpsAv() const;

#ifdef MO_ENABLE_FPS_TRACE
    /** Returns a copy of the decoder fps trace */
    ValueBuffer getFpsTrace() const;
    /** Returns a copy of the decoder average fps trace */
    ValueBuffer getFpsTraceAv() const;
#endif

    /* Access to the set AUDIO::AudioThread, or NULL */
    //AUDIO::AudioThread* audioThread();

    // ------- setter --------

    /** Sets the number of threads to use for decoding.
        0 for auto/max, which is the default.
        @note Must be called before openFile() */
    void setThreadCount(int num);

    /* Sets the audioThread.
        Ownerships stays with caller.
        Set to NULL to disable audio. */
    //void setAudioThread(AUDIO::AudioThread*);

    // -------- work ---------

    /** Starts the decoder thread.
        If the thread is already running this call is ignored,
        unless a stop request is pending, in which case the
        call blocks until the thread is finished before starting
        a new one. */
    void start();

    /** Stops the decoder thread.
        If @p blocking is true, the call waits until the
        thread is stopped */
    void stop(bool blocking = true);

#ifndef MO_USE_FFMPEG
    /** XXX Not fully working yet */
    void seek(size_t filePos);
#endif
    /** Seek to the specified second.
        If @p sec is smaller than getMinBufferTime(),
        the buffer has to be cleared in order to fill again. */
    void seekSecond(double sec);
    /** Seek to the next keyframe after specified second.
        If @p sec is smaller than getMinBufferTime(),
        the buffer has to be cleared in order to fill again. */
    void seekKeyframe(double sec);

    /** Seek to beginning */
    void rewind();

    /** Current file position in input stream */
    size_t currentFilePos() const;

    // ------- frames --------

    /** Returns the current frame size of the video in pixels */
    int currentWidth() const;
    int currentHeight() const;

    /** Returns the number of frames per second of the input stream.
        Value might by 0, if the input stream was not processed sufficiently. */
    double framesPerSecond() const;

    /** Returns the (estimated) length of the stream in seconds */
    double lengthInSeconds() const;

    /** Returns the two frames that are needed for the given presentation time.
        If @p frameA is NULL, the function failed, e.g. the buffer is not ready.
        If @p frameB is NULL it might not be ready or it is not needed for the
        given presentation time.
        @note Every frame that is returned by this function will increase
        it's internal reference counter. Be sure to call doneFrame() after you
        copied/uploaded the frames. The decoder thread will not delete frames
        that still hold a reference and the frame buffer might run out of storage. */
    void getFrames(double presentationTime,
                   DecoderFrame** frameA, DecoderFrame** frameB, double* mix);

    void doneFramesBefore(double presentationTime);

#if 0
    /** Returns the frame for the given pts (in seconds), or NULL.
        Simple wrapper around getFrames() which only returns frameA and
        calls doneFrame() for frameB. */
    DecoderFrame * getFrame(double presentationTime);

    /** @{ */
    /** Signals that you are done procesing the given frame.
        The given frame should be treated as destroyed by the caller.
        It is, however, possible that the next call to getFrames()
        returns the same frame.
        The caller is responsible to
        1) remember which frames got uploaded to the GPU to avoid duplicate uploads
        2) match every call to getFrames() with calls to doneFrame()
           for the returned frames. */
    void doneFrame(DecoderFrame *);
    void doneFrame(double presentationTime);
    /** @} */
#endif
    /** Signals that you are not interested in any frame at all anymore.
        Using any frames previously returned by getFrame() or getFrames()
        leads to undefined behaviour.
        @warning This function is not thread-safe for multiple reader threads! */
    void doneFrames();

    /** Dumps the current frames in the buffer to console */
    void dumpFrames();

private:
    struct Private;
    Private * p_;
};


//MO_END_NAMESPACE

#endif // MCWSRC_DECODERTHREAD_H
