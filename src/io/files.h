/** @file directories.h

    @brief default / configured directories and filetypes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/5/2014</p>
*/

#ifndef MOSRC_IO_FILES_H
#define MOSRC_IO_FILES_H

#include <QStringList>

#include "filetypes.h"

namespace MO {
namespace IO {

class Files
{
public:
    Files() { }

    /** Returns directory for given filetype */
    static QString directory(FileType);

    /** Returns the last filename for given filetype */
    static QString filename(FileType);

    /** Sets the current directory+name for filetype */
    static void setFilename(FileType, const QString& absolute_filename);

    /** Sets the current directory for filetype. */
    static void setDirectory(FileType ft, const QString& path);

    /** Gets a filename from an open-file-dialog */
    static QString getOpenFileName(FileType ft, QWidget * parent = 0,
                        bool updateDirectory = true, bool updateFile = true);

    /** Gets a filename from an save-file-dialog */
    static QString getSaveFileName(FileType ft, QWidget * parent = 0,
                        bool updateDirectory = true, bool updateFile = true);

};



} // namespace IO
} // namespace MO


#endif // MOSRC_IO_FILES_H
