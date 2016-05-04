#ifdef MO_ENABLE_FFMPEG

#ifndef MCWSRC_FFMPEG_H
#define MCWSRC_FFMPEG_H


#define __STDC_CONSTANT_MACROS
#include <cinttypes>

#include <string>

extern "C" {

    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/imgutils.h"
    #include "libswscale/swscale.h"
}

#include "io/error.h"

/** Execute ffmpeg 'int xx()' function and
    throw Exception with ffmpeg error text on error */
#define CHECK_FFM_THROW(avFunc_) \
{ int err__ = avFunc_; if (err__ < 0) MO_ERROR("libav: " \
    << ::FFM::errorString(err__) << " for '" #avFunc_ "'"); }

/** Namespace for all ffmpeg related wrapper */
namespace FFM {

/** Calls av_register_all once */
void initFfmpeg();

/** Convert ffmpeg error code to string */
std::string errorString(int avError);

/** FFmpeg component versions (multiline) */
std::string versionString();

} // namespace FFM

#endif // MCWSRC_FFMPEG_H

#endif // #ifdef MO_ENABLE_FFMPEG
