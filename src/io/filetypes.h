/** @file filetypes.h

    @brief Filetypes and extensions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/13/2014</p>
*/

#ifndef MOSRC_IO_FILETYPES_H
#define MOSRC_IO_FILETYPES_H


#include <QStringList>
#include <QPair>

namespace MO {
namespace IO {

    enum FileType
    {
        FT_ANY,
        FT_SCENE,
        FT_OBJECT_TEMPLATE,
        FT_TEXTURE,
        FT_NORMAL_MAP,
        FT_MODEL,
        FT_GEOMETRY_SETTINGS,
        FT_INTERFACE_XML,
        FT_SOUND_FILE,
        /** Like .mod and .s3m */
        FT_TRACKER,
        FT_PROJECTION_SETTINGS,
        FT_POVRAY,
        FT_EQUATION_PRESET,
        FT_HELP_EXPORT
    };

    extern const QStringList fileTypeIds;
    extern const QStringList fileTypeNames;
    extern const QList<QStringList> fileTypeExtensions;
    extern const QList<QStringList> fileTypeDialogFilters;

    typedef QPair<QString, FileType> FileListEntry;
    typedef QList<FileListEntry> FileList;

} // namespace IO
} // namespace MO

#endif // MOSRC_IO_FILETYPES_H
