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
    { "any",
      "scene",
      "object",
      "texture",
      "ntexture",
      "video",
      "model",
      "shapefile",
      "geom-set",
      "interface",
      "interface-p",
      "sound",
      "ir",
      "s3m",
      "projection-set",
      "povray",
      "equpreset",
      "helpexp",
      "ladspa",
      "text",
      "glsl",
      "angelscript",
      "equation",
      "python34",
      "ssw",
      "csg",
      "evo",
      "fmatrix",
      "nifti",
      "neurofox_hex"
    };

    const QStringList fileTypeNames =
    { QObject::tr("Any"),
      QObject::tr("Scene"),
      QObject::tr("Object"),
      QObject::tr("Texture"),
      QObject::tr("Normal-map texture"),
      QObject::tr("3D-Model"),
      QObject::tr("Video"),
      QObject::tr("Shapefile"),
      QObject::tr("Geometry settings"),
      QObject::tr("Interface layout"),
      QObject::tr("Interface preset"),
      QObject::tr("Audio"),
      QObject::tr("Impulse response"),
      QObject::tr("Tracker file"),
      QObject::tr("Projection settings"),
      QObject::tr("Povray script"),
      QObject::tr("Equation preset"),
      QObject::tr("Help export"),
      QObject::tr("Ladspa directory"),
      QObject::tr("Text"),
      QObject::tr("Glsl"),
      QObject::tr("Angelscript"),
      QObject::tr("Equation"),
      QObject::tr("Python 3.4"),
      QObject::tr("SSW Project"),
      QObject::tr("CSG Tree"),
      QObject::tr("Evolution"),
      QObject::tr("Float Matrix"),
      QObject::tr("Nifti"),
      QObject::tr("Neurofox (hexadecimal)")
    };

    const QList<QStringList> fileTypeExtensions =
    {
        { "*" },
        { "mo3" },
        { "mo3-obj" },
        { "png", "jpg", "jpeg", "bmp", "tif", "tiff", "pbm", "pgm", "xbm", "xpm" },
        { "png", "jpg", "jpeg", "bmp", "tif", "tiff", "pbm", "pgm", "xbm", "xpm" },
        { "avi", "mp4", "mov" },
        { "obj" },
        { "shp" },
        { "mo3-geom" },
        { "xml-iface" },
        { "xml-iface-p" },
        { "wav", "voc", "au", "snd", "aif", "aiff", "aifc", "w64", "flac" },
        { "wav", "voc", "au", "snd", "aif", "aiff", "aifc", "w64", "flac" },
        { "mod", "s3m", "xm", "it" },
        { "xml-proj" },
        { "pov" },
        { "xml-equ" },
        { "" },
        { "so" },
        { "txt", "asc" },
        { "glsl", "vert", "frag", "txt", "asc" },
        { "as", "txt", "asc" },
        { "txt", "asc" },
        { "py" },
        { "uifm" },
        { "xml-csg" },
        { "json" },
        { "json" },
        { "nii", "nii.gz", "img", "hdr" },
        { "csv" }
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
        { QObject::tr("Video files") + " ( *.* )" },
        { QObject::tr("Wavefront Object") + " ( *.obj )" },
        { QObject::tr("Shapefile") + " ( *.shp )" },
        { QObject::tr("geometry presets") + " ( *.mo3-geom )" },
        { QObject::tr("Interface xml") + " ( *.xml *.xml-iface )" },
        { QObject::tr("Interface presets xml") + " ( *.xml *.xml-iface-p )" },
        { QObject::tr("all audio files")
                    + " ( *.wav *.voc *.au *.snd *.aif *.aiff *.aifc *.w64 *.flac )" },
        { QObject::tr("all audio files")
                    + " ( *.wav *.voc *.au *.snd *.aif *.aiff *.aifc *.w64 *.flac )" },
        { QObject::tr("tracker files s3m/mod/xm/it")
                    + " ( *.S3M *.mod *.xm *.it )" },
        { QObject::tr("projector xml files") + " ( *.xml *.xml-proj )" },
        { QObject::tr("povray files") + " ( *.pov )" },
        { QObject::tr("equation xml files") + " ( *.xml *.xml-equ )" },
        { QObject::tr("* (*)") },
        { QObject::tr("* (*)") },
        { QObject::tr("text files") + " ( *.txt *.asc )",
          QObject::tr("all files (*)") },
        { QObject::tr("glsl source") + " ( *.glsl *.frag *.vert *.txt *.asc )",
          QObject::tr("all files (*)") },
        { QObject::tr("angelscript source") + " ( *.as *.txt *.asc )",
          QObject::tr("all files (*)") },
        { QObject::tr("equations") + " ( *.txt *.asc )",
          QObject::tr("all files (*)") },
        { QObject::tr("python") + " ( *.py )",
          QObject::tr("all files (*)") },
        { QObject::tr("SSW-JSON-Project (*.uifm)") },
        { QObject::tr("xml files ( *.xml *.xml-csg )") },
        { QObject::tr("json files ( *.json )"),
          QObject::tr("all files (*)") },
        { QObject::tr("json files ( *.json )"),
          QObject::tr("all files (*)") },
        { QObject::tr("Nifti files ( *.nii *.nii.gz *.img *.hdr )"),
          QObject::tr("all files (*)") },
        { QObject::tr("Neurofox files ( *.csv * )") }

    };


    FileType guessFiletype(const QString& fn)
    {
        for (int i = 1; i < fileTypeExtensions.size(); ++i)
        {
            for (const auto & ext : fileTypeExtensions[i])
            if (!ext.isEmpty())
            {
                if (fn.endsWith(ext))
                    return FileType(i);
            }
        }
        return FT_ANY;
    }

} // namespace IO
} // namespace MO
