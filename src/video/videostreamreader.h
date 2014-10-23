/** @file videostreamreader.h

    @brief play video URIs through gstreamer

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 20/10/2014</p>
*/

#ifndef MO_DISABLE_GST

#ifndef VIDEOSTREAMREADER_H
#define VIDEOSTREAMREADER_H

#include <iostream>
#include <string>
#include <sstream>
#include <gst/gst.h>

#include <QImage>

class VideoStreamReader {

public:
    VideoStreamReader(std::string &uri,
                      const char* format="RGB",
                      int reqested_width=640);

    ~VideoStreamReader();

    void initialize(std::string &uri);

    void pause();
    void play();
    void stop();

    void seek_time(float sec);

    unsigned char* getGlTexture();

    int width()  { return m_width;  }
    int height() { return m_height; }

    void requestWidth(int w) { v_width = w; }
    void requestFormat(std::string &format) { v_format = format; }

    bool isPlaying() { return m_isPlaying; }
    bool isPaused() { return m_isPaused; }

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
