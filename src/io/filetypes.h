/** @file filetypes.h

    @brief Filetypes and extensions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/13/2014</p>
*/

#ifndef MOSRC_IO_FILETYPES_H
#define MOSRC_IO_FILETYPES_H


#include <QStringList>

namespace MO {
namespace IO {

    enum FileType
    {
        FT_ANY,
        FT_SCENE,
        FT_TEXTURE,
        FT_MODEL,
        FT_GEOMETRY_SETTINGS,
        FT_SOUND_FILE
    };

    extern const QStringList fileTypeIds;
    extern const QStringList fileTypeNames;
    extern const QList<QStringList> fileTypeExtensions;
    extern const QList<QStringList> fileTypeDialogFilters;

} // namespace IO
} // namespace MO

#endif // MOSRC_IO_FILETYPES_H
