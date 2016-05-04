#ifdef MO_ENABLE_FFMPEG

#include <sstream>

#include "ffmpeg.h"


namespace FFM {


void initFfmpeg()
{
    static bool isInit = false;
    if (!isInit)
    {
        av_register_all();
        isInit = true;
    }
}


std::string errorString(int avError)
{
    std::string buf;
    buf.resize(AV_ERROR_MAX_STRING_SIZE+1, '\0');
    if (av_strerror(avError, &buf[0], AV_ERROR_MAX_STRING_SIZE) != 0)
        return "*unknown error*";
    return buf;
}

std::string versionString()
{
    std::stringstream s;

    s << av_version_info()
      << "\navutil:   " << LIBAVUTIL_IDENT << " " << avutil_license()
      << "\navformat: " << LIBAVFORMAT_IDENT << " " << avformat_license()
      << "\navcodec:  " << LIBAVCODEC_IDENT << " " << avcodec_license()
      << "\nswscale:  " << LIBSWSCALE_IDENT << " " << swscale_license()
         ;

    return s.str();
}

} // namespace FFM

#endif // #ifdef MO_ENABLE_FFMPEG
