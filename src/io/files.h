/** @file files.h

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
    static QString directory(FileType, bool returnCurrentPathIfNotDefined = true);

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

    /** Returns a directory to read from or write into. */
    static QString getDirectory(const QString& title, QWidget * parent)
        { return getDirectory(FT_ANY, parent, false, title); }

    /** Returns a directory to read from or write into.
        If @p title is not empty, it will be used as the title of the dialog. */
    static QString getDirectory(FileType ft, QWidget * parent = 0,
                        bool updateDirectory = true, const QString& title = "");

    static void findFiles(FileType ft, bool recursive,
                          QStringList& entryList, bool include_directory = true);

    static void findFiles(FileType ft, const QString& directory, bool recursive,
                          QStringList& entryList, bool include_directory = true);

    static void getDirectories(const QString& basePath, QStringList& directoryList,
                               bool recursive);
};



} // namespace IO
} // namespace MO


#endif // MOSRC_IO_FILES_H
