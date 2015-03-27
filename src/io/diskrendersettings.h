/** @file rendersettings.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/26/2015</p>
*/

#ifndef MOSRC_IO_DISKRENDERSETTINGS_H
#define MOSRC_IO_DISKRENDERSETTINGS_H

#include <QList>

#include "types/float.h"
#include "audio/configuration.h"

namespace MO {
namespace IO { class CommandLineParser; }

/** Complete settings for render-to-disk. */
class DiskRenderSettings
{
public:

    struct ImageFormat
    {
        QString name, ext, id;
        size_t index;
    };

    struct AudioFormat
    {
        QString name, ext, id;
        size_t index;
    };

    DiskRenderSettings();

    /** Returns a list of all supported formats */
    const QList<ImageFormat>& imageFormats() const;
    /** Returns a list of all supported formats */
    const QList<AudioFormat>& audioFormats() const;

    // ------------ command line --------------

    /** Creates all commandline options.
        Ownership is with caller. */
    IO::CommandLineParser * createCommandLineParser() const;
    /** Applies the options given on command line */
    void applyCommandLine(IO::CommandLineParser*);

    // -------------- getter ------------------

    /** Returns the output directory */
    const QString& directory() const { return p_directory_; }

    SamplePos startSample() const { return p_time_start_; }
    SamplePos lengthSample() const { return p_time_length_; }
    size_t lengthFrame() const;
    Double lengthSecond() const;

    /** The image filename pattern */
    const QString& imagePattern() const { return p_image_pattern_; }
    /** The offset for image frame numbers */
    size_t imagePatternOffset() const { return p_image_num_offset_; }
    size_t imageWidth() const { return p_image_w_; }
    size_t imageHeight() const { return p_image_h_; }
    size_t imageBitsPerChannel() const { return p_image_bpc_; }
    size_t imageFps() const { return p_image_fps_; }
    /** Returns the index in imageFormats() */
    size_t imageFormat() const { return p_image_format_; }
    /** Returns id of current image format */
    QString imageFormatId() const;

    const QString& audioPattern() const { return p_audio_pattern_; }
    size_t audioFormat() const { return p_audio_format_; }
    size_t audioBitsPerChannel() const { return p_audio_bpc_; }

    // ------------- setter --------------------

    /** Sets format by id */
    void setImageFormat(size_t index);
    void setImageFormat(const QString& id);

private:

    void p_setDefault_();

    static QList<ImageFormat> p_image_formats_;
    static QList<AudioFormat> p_audio_formats_;

    QString p_directory_,
            p_image_pattern_,
            p_audio_pattern_;

    AUDIO::Configuration
            p_audio_conf_;

    SamplePos
            p_time_start_,
            p_time_length_;

    size_t  p_image_num_offset_,
            p_image_w_,
            p_image_h_,
            p_image_bpc_,
            p_image_format_,
            p_image_fps_,

            p_audio_format_,
            p_audio_bpc_;
};

} // namespace MO

#endif // MOSRC_IO_DISKRENDERSETTINGS_H
