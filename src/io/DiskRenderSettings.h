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
#include "audio/Configuration.h"

namespace MO {
namespace IO { class CommandLineParser; class XmlStream; }

/** Complete settings for render-to-disk.
    This is quite something if you think about it..
    @todo support xml IO */
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
    size_t startFrame() const;
    Double startSecond() const;
    SamplePos lengthSample() const { return p_time_length_; }
    size_t lengthFrame() const;
    Double lengthSecond() const;
    SamplePos endSample() const { return startSample() + lengthSample(); }
    size_t endFrame() const { return startFrame() + lengthFrame(); }
    Double endSecond() const { return startSecond() + lengthSecond(); }

    /** Enable image rendering */
    bool imageEnable() const { return p_image_enable_; }
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
    size_t imageCompression() const { return p_image_comp_; }
    size_t imageNumThreads() const { return p_image_threads_; }
    size_t imageNumQue() const { return p_image_ques_; }
    size_t imageSizeBytes() const;

    /** Enable audio rendering */
    bool audioEnable() const { return p_audio_enable_; }
    /** Enable splitting of audio files */
    bool audioSplitEnable() const { return p_audio_split_enable_; }
    /** Enable normalizing when splitting. */
    bool audioNormalizeEnable() const { return p_audio_norm_enable_; }
    const AUDIO::Configuration & audioConfig() const { return p_audio_conf_; }
    const QString& audioPattern() const { return p_audio_pattern_; }
    size_t audioPatternOffset() const { return p_audio_num_offset_; }
    size_t audioPatternWidth() const { return p_audio_num_width_; }
    /** Returns the index in audioFormats() */
    size_t audioFormatIndex() const { return p_audio_format_idx_; }
    /** Returns id of current audio format */
    QString audioFormatId() const;
    QString audioFormatExt() const;
    size_t audioBitsPerChannel() const { return p_audio_bpc_; }

    /** Returns the filename for the frame number according to settings */
    QString makeImageFilename(size_t frame) const;

    /** Returns the filename for the unnormalized multi-channel audio file */
    QString makeAudioFilename() const;

    /** Returns the filename for the audio file according to settings */
    QString makeAudioFilename(size_t channel) const;

    // ------------- setter --------------------

    void setDirectory(const QString& dir) { p_directory_ = dir; }

    void setStartSample(SamplePos p) { p_time_start_ = p; }
    void setStartFrame(size_t frame);
    void setStartSecond(Double sec);
    void setLengthSample(SamplePos p) { p_time_length_ = p; }
    void setLengthFrame(size_t frame);
    void setLengthSecond(Double sec);

    void setImageEnable(bool e) { p_image_enable_ = e; }
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
    void setImageCompression(size_t c) { p_image_comp_ = c; }
    void setImageNumThreads(size_t t) { p_image_threads_ = t; }
    void setImageNumQue(size_t q) { p_image_ques_ = q; }

    void setAudioEnable(bool e) { p_audio_enable_ = e; }
    void setAudioSplitEnable(bool e) { p_audio_split_enable_ = e; }
    void setAudioNormalizeEnable(bool e) { p_audio_norm_enable_ = e; }
    /** Write access */
    AUDIO::Configuration & audioConfig() { return p_audio_conf_; }
    void setAudioFormat(size_t index);
    /** Sets format by id */
    void setAudioFormat(const QString& id);
    void setAudioPattern(const QString& pattern) { p_audio_pattern_ = pattern; }
    void setAudioPatternOffset(size_t o) { p_audio_num_offset_ = o; }
    void setAudioPatternWidth(size_t w) { p_audio_num_width_ = w; }
    void setAudioBitsPerChannel(size_t b) { p_audio_bpc_ = b; }

    // ------------ conversion ----------------

    SamplePos frame2sample(size_t frame) const
        { return (double)frame / imageFps() * audioConfig().sampleRate(); }

    Double frame2second(size_t frame) const { return audioConfig().sampleRateInv() * frame2sample(frame); }

private:

    void p_setDefault_();

    static QList<ImageFormat> p_image_formats_;
    static QList<AudioFormat> p_audio_formats_;

    QString p_directory_,
            p_image_pattern_,
            p_audio_pattern_;

    bool    p_image_enable_,
            p_audio_enable_,
            p_audio_split_enable_,
            p_audio_norm_enable_;

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
            p_image_quality_,
            p_image_comp_,
            p_image_threads_,
            p_image_ques_,
            p_audio_format_idx_,
            p_audio_num_offset_,
            p_audio_num_width_,
            p_audio_bpc_;
};

} // namespace MO

#endif // MOSRC_IO_DISKRENDERSETTINGS_H
