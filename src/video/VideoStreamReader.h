/   ** @file videostreamreader.h

    @brief play video URIs through gstreamer

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 20/10/2014</p>
*/

#ifndef MO_DISABLE_GST

#ifndef MOSRC_VIDEO_VIDEOSTREAMREADER_H
#define MOSRC_VIDEO_VIDEOSTREAMREADER_H

#include <iostream>
#include <string>
#include <sstream>

#include <gst/gst.h>

class QImage;

class VideoStreamReader {

public:
    VideoStreamReader(std::string &uri,
                      const char* format="RGB",
                      int reqested_width=640);

    ~VideoStreamReader();

    // -------- getter/state --------

    int width()  const { return m_width;  }
    int height() const { return m_height; }

    bool isPlaying() const { return m_isPlaying; }
    bool isPaused()  const { return m_isPaused; }

    // -------- interface -----------

    void initialize(std::string &uri);

    void play();
    void stop();
    void pause();

    void seek_time(float sec);

    unsigned char* getGlTexture();

    void requestWidth(int w) { v_width = w; }
    void requestFormat(std::string &format) { v_format = format; }


private:
    GstElement *m_pipeline, *m_sink;
    GstStateChangeReturn ret;
    GstSample *m_sample;
    GstMapInfo m_map;
    QImage *m_image;

    gint64 duration, position;
    int m_width, m_height;

    std::string v_format;
    int v_width;

    bool m_isPlaying;
    bool m_isPaused;
};

#endif // VIDEOSTREAMREADER_H

#endif // MO_DISABLE_GST
