/** @file filetypes.cpp

    @brief Filetypes and extensions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/13/2014</p>
*/

#include "filetypes.h"

namespace MO {
namespace IO {

    const QStringList fileTypeIds =
    { "any", "scene", "texture", "model", "geom-set" };

    const QStringList fileTypeNames =
    { "Any file", "Scene", "Texture", "Model", "Geometry settings" };

    const QList<QStringList> fileTypeExtensions =
    {
        { "*" },
        { "mo3" },
        { "png", "jpg", "jpeg", "bmp" },
        { "obj" },
        { "mo3-geom" }
    };

    const QList<QStringList> fileTypeDialogFilters =
    {
        { "all files ( * )" },
        { "scene files ( *.mo3 )" },
        { "all image files ( *.png *.jpg *.jpeg *.bmp )" },
        { "Wavefront OBJ ( *.obj )" },
        { "geometry presets ( *.mo3-geom )" }
    };


} // namespace IO
} // namespace MO
