#ifdef MO_ENABLE_FFMPEG


#ifndef MCWSRC_VIDEOSTREAM_H
#define MCWSRC_VIDEOSTREAM_H

#include <string>

#ifdef MCW_USE_QT
#include <QImage>
#endif

#include "video/decoderframe.h"
//#include "tool/mediafileinfo.h"

namespace FFM {


class VideoStream
{
public:
    VideoStream();
    ~VideoStream();

    // ---------- getter ------------

    /** Is stream opened */
    bool isReady() const;

    //MediaFileInfo getFileInfo() const;

    /** Returns the currently open filename */
    const std::string& currentFilename() const;

    /** Informational string */
    std::string toString() const;

    /** Returns current position in input stream */
    size_t curFilePos() const;

    /** Returns length of the whole video in seconds */
    double lengthInSeconds() const;

    /** Returns the number of decoded frames since openFile() */
    int64_t numDecodedFrames() const;

    /** Returns the frames per second of the video stream, or 0.0.
        The value is valid after the first successful decodeFrame() */
    double framesPerSecond() const;

    /** Returns the number of threads used for decoding.
        0 for auto/max. */
    int threadCount() const;

    // ------ setter --------

    /** Sets the number of threads to use for decoding.
        0 for auto/max, which is the default.
        @note Must be called before openFile() */
    void setThreadCount(int num);

    // --------- io ---------

    /** Opens the given file/stream.
        @throws Exception on any error */
    void openFile(const std::string& url);
    /** Closes the stream completely */
    void close();

    /** Seek to the next keyframe >= specified second */
    void seekKeyframe(double sec);
    /** Seek to specified second in video stream */
    void seekSecond(double sec);
    void rewind();

    // -------- frames ------

    /** Read the next @p frame.
        Returns NULL when the stream has no more frames.
        Ownership is with caller. */
    DecoderFrame* getVideoFrame();


#ifdef MCW_USE_QT
    /** Like currentFrame().
        Not very efficient and more for debugging purposes. */
    QImage getQImage() const;
#endif

private:
    struct Private;
    Private * p_;
};

} // namespace FFM

#endif // MCWSRC_VIDEOSTREAM_H

#endif // #ifdef MO_ENABLE_FFMPEG
