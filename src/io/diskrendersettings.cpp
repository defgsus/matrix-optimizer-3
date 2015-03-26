/** @file rendersettings.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/26/2015</p>
*/

#include <QImageWriter>

#include "diskrendersettings.h"
#include "io/commandlineparser.h"

namespace MO {

QList<DiskRenderSettings::ImageFormat> DiskRenderSettings::p_image_formats_;
QList<DiskRenderSettings::AudioFormat> DiskRenderSettings::p_audio_formats_;

DiskRenderSettings::DiskRenderSettings()
{
    p_setDefault_();
}

void DiskRenderSettings::p_setDefault_()
{
    p_directory_ = "./output";

    p_time_start_ = 0;
    p_time_length_ = p_audio_conf_.sampleRate() * 60;

    p_image_pattern_ = "image_%num%.%ext%";
    p_image_num_offset_ = 1;
    p_image_w_ = p_image_h_ = 1024;
    p_image_fps_ = 30;
    p_image_format_ = 0;
    p_image_bpc_ = 8;

    p_audio_pattern_ = "audio_%num%.%ext%";
    p_audio_conf_ = AUDIO::Configuration(44100, 256, 0, 2);
    p_audio_format_ = 0;
    p_audio_bpc_ = 16;
}


const QList<DiskRenderSettings::ImageFormat>& DiskRenderSettings::imageFormats() const
{
    // create static list
    if (p_image_formats_.isEmpty())
    {
        int index = 0;
        auto bas = QImageWriter::supportedImageFormats();
        for (const QByteArray& ba : bas)
        {
            ImageFormat f;
            f.ext = QString::fromUtf8(ba);
            f.name = f.ext;
            f.id = f.ext;
            f.index = index++;

            p_image_formats_ << f;
        }

        // create dummy in unlikely case that there are no formats available
        if (p_image_formats_.isEmpty())
        {
            ImageFormat f;
            f.ext = f.id = "-";
            f.name = QObject::tr("none available");
            f.index = 0;
            p_image_formats_ << f;
        }

    }

    return p_image_formats_;
}


const QList<DiskRenderSettings::AudioFormat>& DiskRenderSettings::audioFormats() const
{
    // create static list
    if (p_audio_formats_.isEmpty())
    {
        /** @todo create the extensive libsndfile list */
        AudioFormat f;
        f.ext = f.id = "wav";
        f.name = "RIFF Wave";
        p_audio_formats_ << f;

        for (int i = 0; i < p_audio_formats_.size(); ++i)
            p_audio_formats_[i].index = i;
    }

    return p_audio_formats_;
}

QString DiskRenderSettings::imageFormatId() const
{
    return p_image_format_ < size_t(imageFormats().size())
            ? imageFormats()[p_image_format_].id
            : "-";
}

void DiskRenderSettings::setImageFormat(const QString &id)
{
    for (int i=0; i<imageFormats().size(); ++i)
    if (id == imageFormats()[i].id)
    {
        p_image_format_ = i;
        break;
    }
}

size_t DiskRenderSettings::lengthFrame() const
{
    return p_time_length_ * p_image_fps_ / p_audio_conf_.sampleRate();
}

Double DiskRenderSettings::lengthSecond() const
{
    return p_time_length_ * p_audio_conf_.sampleRateInv();
}








IO::CommandLineParser * DiskRenderSettings::createCommandLineParser() const
{
    auto cl = new IO::CommandLineParser();

    // string of all image formats
    QString ifmtstr;
    for (const ImageFormat & f : imageFormats())
    {
        if (!ifmtstr.isEmpty())
            ifmtstr += "\n";
        ifmtstr += f.id + ": " + f.name + " (." + f.ext + ")";
    }


    cl->addParameter("dir", "o, directory",
                        QObject::tr("The output directory"),
                        directory());

    cl->addParameter("image_fmt", "if, image-format",
                        QObject::tr("Format of output images, can be one of:")
                        + " " + ifmtstr,
                        imageFormats()[imageFormat()].id);

    return cl;
}

void DiskRenderSettings::applyCommandLine(IO::CommandLineParser * cl)
{
    if (cl->contains("dir"))
        p_directory_ = cl->value("dir").toString();

    if (cl->contains("image_fmt"))
        setImageFormat(cl->value("image_fmt").toString());
}

} // namespace MO
