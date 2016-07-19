    /** @file videostreamreader.cpp

    @brief play video uris through gstreamer

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 20/10/2014</p>
*/

#ifndef MO_DISABLE_GST

#include "videostreamreader.h"

#include <QGLWidget>
#include <QImage>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

VideoStreamReader::VideoStreamReader(std::string &uri,
                                     const char* format,
                                     int reqested_width)
    : m_pipeline(NULL)
    , m_sink(NULL)
    , m_sample(NULL)
    , m_image(NULL)
    , duration(0)
    , position(0)
    , v_format(format)
    , v_width(reqested_width)
    , m_isPlaying(false)
    , m_isPaused(false)
{
    initialize(uri);
    m_image = new QImage;
}

VideoStreamReader::~VideoStreamReader()
{
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    gst_object_unref(m_pipeline);
    if(m_image) delete m_image;
}

void VideoStreamReader::initialize(std::string &uri)
{

    gst_init(0,NULL);

    std::stringstream descr;
    descr << "playbin uri=" << uri;
    GError *error = NULL;
    m_pipeline = gst_parse_launch(descr.str().c_str(), &error);
    if(error != NULL) {
        std::cerr << "Could not construct pipeline with uri: "
                  << uri << std::endl;
        g_error_free(error);
        exit(-1);
    }

    m_sink   = gst_element_factory_make("appsink", "sink");

    gst_app_sink_set_max_buffers((GstAppSink*)m_sink, 100);

    GstElement
            *scale  = gst_element_factory_make("videoscale",   "scale"),
            *flip   = gst_element_factory_make("videoflip",    "flip"),
#if(0)
            *asink  = gst_element_factory_make("fakesink", "asink");
#else
#ifdef __APPLE__
            *asink  = gst_element_factory_make("osxaudiosink", "asink");
#else
            *asink  = gst_element_factory_make("autoaudiosink", "asink");
#endif
#endif
    GstElement
            *bin    = gst_bin_new("postprocess");
    gst_bin_add_many(GST_BIN(bin), scale, flip, m_sink, NULL);

    g_object_set(flip, "method", 5, NULL);
    GstCaps *caps   = gst_caps_new_simple("video/x-raw",
                                          "format", G_TYPE_STRING, v_format.c_str(),
                                          "width", G_TYPE_INT, v_width,
                                          "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                                          NULL);
    if(!gst_element_link_filtered(scale, flip, caps)) {
        std::cerr << "Linking of 'scale' and 'flip' failed.\n";
        gst_object_unref(m_pipeline);
        exit(-1);
    }
    if(!gst_element_link_many(flip, m_sink, NULL)) {
        std::cerr << "Linking of 'flip' and 'sink' failed.\n";
        gst_object_unref(m_pipeline);
        exit(-1);
    }

    GstPad  *pad    = gst_element_get_static_pad(scale, "sink");
    gst_element_add_pad(bin,gst_ghost_pad_new("sink", pad));
    gst_object_unref(pad);

    g_object_set(G_OBJECT(m_pipeline), "video-sink", bin, NULL);
    g_object_set(G_OBJECT(m_pipeline), "audio-sink", asink, NULL);

}

void VideoStreamReader::pause()
{
    ret = gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
    switch(ret) {
    case GST_STATE_CHANGE_FAILURE:
        std::cerr << "Failed to pause the file" << std::endl;
        exit(-1);
    case GST_STATE_CHANGE_NO_PREROLL:
        std::cerr << "No live sources supported yet!\n";
        exit(-1);
    default:
        std::cerr << "Pausing the file\n";
        m_isPlaying = true;
        m_isPaused = true;
        break;
    }
}

void VideoStreamReader::play()
{
    ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    switch(ret) {
    case GST_STATE_CHANGE_FAILURE:
        std::cerr << "Failed to play the file" << std::endl;
        exit(-1);
    case GST_STATE_CHANGE_NO_PREROLL:
        std::cerr << "No live sources supported yet!\n";
        exit(-1);
    default:
        std::cerr << "Playing the file\n";
        m_isPlaying = true;
        m_isPaused = false;
        break;
    }
}

void VideoStreamReader::stop()
{
    ret = gst_element_set_state(m_pipeline, GST_STATE_NULL);
    switch(ret) {
    case GST_STATE_CHANGE_FAILURE:
        std::cerr << "Failed to stop the file" << std::endl;
        exit(-1);
    case GST_STATE_CHANGE_NO_PREROLL:
        std::cerr << "No live sources supported yet!\n";
        exit(-1);
    default:
        std::cerr << "Stopping the file\n";
        m_isPlaying = false;
        m_isPaused = false;
        break;
    }
}

void VideoStreamReader::seek_time(float sec)
{
    ret = gst_element_get_state(m_pipeline, NULL, NULL, sec*GST_SECOND);
    if(ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "Failed to play the file" << std::endl;
        exit(-1);
    }

    /* How to get the duration of the file: */
    gst_element_query_duration(m_pipeline, GST_FORMAT_TIME, &duration);
    if(duration != -1) {
        position = sec * GST_SECOND;
    } else {
        position = 1 * GST_SECOND;
    }

    /* How to seek to a position */
    gst_element_seek_simple(m_pipeline,
                            GST_FORMAT_TIME,
                            GstSeekFlags(GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH),
                            position);
}

unsigned char *VideoStreamReader::getGlTexture()
{
    if(m_isPaused) return NULL;
    GstAppSink *appsink = (GstAppSink*) m_sink;
    if(gst_app_sink_is_eos(appsink)) return NULL;
    m_sample = gst_app_sink_pull_sample(appsink);
    if(m_sample) {
        GstCaps      *caps = gst_sample_get_caps(m_sample);
        if(!caps) {
            std::cerr << "Could not get snapshot format\n";
            exit(-1);
        }
        GstStructure *s = gst_caps_get_structure(caps, 0);

        gboolean res;
        res  = gst_structure_get_int(s, "width", &m_width);
        res |= gst_structure_get_int(s, "height", &m_height);
        if(!res) {
            std::cerr << "Could not get snapshot dimensions\n";
            exit(-1);
        }
        GstBuffer    *buffer = gst_sample_get_buffer(m_sample);
        gst_buffer_map(buffer, &m_map, GST_MAP_READ);
        gst_buffer_unmap(buffer, &m_map);
        gst_sample_unref(m_sample);

        QImage tmp(m_map.data,
                   m_width,
                   m_height,
                   GST_ROUND_UP_4(m_width*3),
                   QImage::Format_RGB888);
        *m_image = QGLWidget::convertToGLFormat(tmp);

        return m_image->bits();
    } else {
        std::cerr << "Could not make snapshot!\n";
        return NULL;
    }
}


#endif // MO_DISABLE_GST
