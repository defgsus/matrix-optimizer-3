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
    { "any", "scene", "object", "texture", "ntexture", "model",
      "geom-set", "sound", "s3m", "projection-set",
      "povray", "equpreset", "helpexp" };

    const QStringList fileTypeNames =
    { QObject::tr("Any file"),
      QObject::tr("Scene"),
      QObject::tr("Object"),
      QObject::tr("Texture"),
      QObject::tr("Normal-map texture"),
      QObject::tr("Model"),
      QObject::tr("Geometry settings"),
      QObject::tr("Audio file"),
      QObject::tr("Tracker file"),
      QObject::tr("Projection settings"),
      QObject::tr("Povray script"),
      QObject::tr("Equation preset"),
      QObject::tr("Help export")};

    const QList<QStringList> fileTypeExtensions =
    {
        { "*" },
        { "mo3" },
        { "mo3-obj" },
        { "png", "jpg", "jpeg", "bmp", "tif", "tiff", "pbm", "pgm", "xbm", "xpm" },
        { "png", "jpg", "jpeg", "bmp", "tif", "tiff", "pbm", "pgm", "xbm", "xpm" },
        { "obj" },
        { "mo3-geom" },
        { "wav", "voc", "au", "snd", "aiff", "aifc", "w64", "flac" },
        { "mod", "s3m", "xm", "it" },
        { "xml-proj" },
        { "pov" },
        { "xml-equ" },
        { "" }
    };

    const QList<QStringList> fileTypeDialogFilters =
    {
        { QObject::tr("all files") + " ( * *.* )" },
        { QObject::tr("scene files") + " ( *.mo3 )" },
        { QObject::tr("object files") + " ( *.mo3-obj )" },
        { QObject::tr("all image files") + " ( *.png *.jpg *.jpeg *.bmp *.tif *.tiff *.pbm *.pgm *.xbm *.xpm )",
          QObject::tr("portable network graphics") + " ( *.png )",
          QObject::tr("jpeg") + " ( *.jpg *.jpeg )",
          QObject::tr("windows bitmap") + " ( *.bmp )",
          QObject::tr("Tiff files") + " ( *.tif *.tiff )",
          QObject::tr("Portable bitmap/graymap") + " ( *.pbm *.pgm )",
          QObject::tr("X11 bitmap/pixmap") + " ( *.xbm *.xpm )" },
        { QObject::tr("all image files") + " ( *.png *.jpg *.jpeg *.bmp *.tif *.tiff *.pbm *.pgm *.xbm *.xpm )",
          QObject::tr("portable network graphics") + " ( *.png )",
          QObject::tr("jpeg") + " ( *.jpg *.jpeg )",
          QObject::tr("windows bitmap") + " ( *.bmp )",
          QObject::tr("Tiff files") + " ( *.tif *.tiff )",
          QObject::tr("Portable bitmap/graymap") + " ( *.pbm *.pgm )",
          QObject::tr("X11 bitmap/pixmap") + " ( *.xbm *.xpm )" },
        { QObject::tr("Wavefront Object") + " ( *.obj )" },
        { QObject::tr("geometry presets") + " ( *.mo3-geom )" },
        { QObject::tr("all audio files")
                    + " ( *.wav *.voc *.au *.snd *.aiff *.aifc *.w64 *.flac )" },
        { QObject::tr("tracker files s3m/mod/xm/it")
                    + " ( *.S3M *.mod *.xm *.it )" },
        { QObject::tr("projector xml files") + " ( *.xml-proj )" },
        { QObject::tr("povray files") + " ( *.pov )" },
        { QObject::tr("equation xml files") + " ( *.xml-equ )" },
        { QObject::tr("* (*)") }
    };


} // namespace IO
} // namespace MO
