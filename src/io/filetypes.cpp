/** @file filetypes.cpp

    @brief Filetypes and extensions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/13/2014</p>
*/

#include <QObject> // for tr()

#include "filetypes.h"

namespace MO {
namespace IO {

    const QStringList fileTypeIds =
    { "any", "scene", "texture", "model", "geom-set", "sound" };

    const QStringList fileTypeNames =
    { QObject::tr("Any file"),
      QObject::tr("Scene"),
      QObject::tr("Texture"),
      QObject::tr("Model"),
      QObject::tr("Geometry settings"),
      QObject::tr("Audio file") };

    const QList<QStringList> fileTypeExtensions =
    {
        { "*" },
        { "mo3" },
        { "png", "jpg", "jpeg", "bmp" },
        { "obj" },
        { "mo3-geom" },
        { "wav", "voc", "au", "snd", "aiff", "aifc", "w64", "flac" }
    };

    const QList<QStringList> fileTypeDialogFilters =
    {
        { QObject::tr("all files") + " ( * )" },
        { QObject::tr("scene files") + " ( *.mo3 )" },
        { QObject::tr("all image files") + " ( *.png *.jpg *.jpeg *.bmp )",
          QObject::tr("portable network graphics") + " ( *.png )",
          QObject::tr("jpeg") + " ( *.jpg *.jpeg )",
          QObject::tr("windows bitmap") + " ( *.bmp )" },
        { QObject::tr("Wavefront Object") + " ( *.obj )" },
        { QObject::tr("geometry presets") + " ( *.mo3-geom )" },
        { QObject::tr("all audio files")
                    + " ( *.wav *.voc *.au *.snd *.aiff *.aifc *.w64 *.flac )" }
    };


} // namespace IO
} // namespace MO
