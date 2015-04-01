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
namespace IO { class CommandLineParser; class XmlStream; }

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
    static const QList<ImageFormat>& imageFormats();
    /** Returns a list of all supported formats */
    static const QList<AudioFormat>& audioFormats();

    // ------------ io -------------

    /** Writes the settings into the xml stream.
        The stream is expected to be writeable.
        The section "disk-render-settings" is created.
        The section on return is the same as on entry.
        @throws IoException on any errors. */
    void serialize(IO::XmlStream&) const;

    /** Reads the settings from the xml stream.
        The stream is expected to be readable
        and the current section must be "disk-render-settings".
        The section on return is the same as on entry.
        @throws IoException on any errors. */
    void deserialize(IO::XmlStream&);

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
    size_t startFrame() const;
    Double startSecond() const;
    size_t lengthFrame() const;
    Double lengthSecond() const;

    /** The image filename pattern */
    const QString& imagePattern() const { return p_image_pattern_; }
    /** The offset for image frame numbers */
    size_t imagePatternOffset() const { return p_image_num_offset_; }
    size_t imagePatternWidth() const { return p_image_num_width_; }
    size_t imageWidth() const { return p_image_w_; }
    size_t imageHeight() const { return p_image_h_; }
    size_t imageBitsPerChannel() const { return p_image_bpc_; }
    size_t imageFps() const { return p_image_fps_; }
    /** Returns the index in imageFormats() */
    size_t imageFormatIndex() const { return p_image_format_idx_; }
    /** Returns id of current image format */
    QString imageFormatId() const;
    QString imageFormatExt() const;
    size_t imageQuality() const { return p_image_quality_; }

    const QString& audioPattern() const { return p_audio_pattern_; }
    const AUDIO::Configuration & audioConfig() const { return p_audio_conf_; }
    size_t audioFormat() const { return p_audio_format_; }
    size_t audioBitsPerChannel() const { return p_audio_bpc_; }

    /** Returns the filename for the frame number according to settings */
    QString makeImageFilename(size_t frame) const;

    // ------------- setter --------------------

    void setDirectory(const QString& dir) { p_directory_ = dir; }

    void setStartSample(SamplePos p) { p_time_start_ = p; }
    void setStartFrame(size_t frame);
    void setStartSecond(Double sec);
    void setLengthSample(SamplePos p) { p_time_length_ = p; }
    void setLengthFrame(size_t frame);
    void setLengthSecond(Double sec);

    void setImageFormat(size_t index);
    /** Sets format by id */
    void setImageFormat(const QString& id);
    void setImagePattern(const QString& pattern) { p_image_pattern_ = pattern; }
    void setImagePatternOffset(size_t o) { p_image_num_offset_ = o; }
    void setImagePatternWidth(size_t w) { p_image_num_width_ = w; }
    void setImageSize(size_t w, size_t h) { p_image_w_ = w; p_image_h_ = h; }
    void setImageFps(size_t fps) { p_image_fps_ = fps; }
    void setImageBitsPerChannel(size_t b) { p_image_bpc_ = b; }
    void setImageQuality(size_t q) { p_image_quality_ = q; }
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
            p_image_num_width_,
            p_image_w_,
            p_image_h_,
            p_image_bpc_,
            p_image_format_idx_,
            p_image_fps_,
            p_audio_format_,
            p_image_quality_,
            p_audio_bpc_;
};

} // namespace MO

#endif // MOSRC_IO_DISKRENDERSETTINGS_H
