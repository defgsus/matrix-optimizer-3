/** @file filemanager.h

    @brief Filename and files manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/20/2014</p>
*/

#ifndef MOSRC_IO_FILEMANAGER_H
#define MOSRC_IO_FILEMANAGER_H

#include <QObject>

#include "filetypes.h"

namespace MO {
namespace IO {

class FileManager;

/** Returns a singleton instance */
FileManager & fileManager();

class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = 0);
    ~FileManager();

    // ------------------ getter --------------------

    /** Returns the local filename for the given @p filename.
        Resource filenames (starting with ':') are returned as-is. */
    QString localFilename(const QString& filename);

    // ------------------ setter --------------------

    /** Clears the list of known filenames */
    void reset();

    /** Adds a filename to the list if files that are needed. */
    void addFilename(FileType ft, const QString& filename);

    /** Starts looking for files.
        On clients, the files will be transferred if needed. */
    void acquireFiles();

    // ----------------- client mode ----------------

#ifdef MO_CLIENT

    /** Gets the file */
    void getFile(const QString& filename, IO::FileType ft);

#endif

signals:

public slots:

private:

    class Private;
    Private * p_;
};


} // namespace IO
} // namespace MO


#endif // MOSRC_IO_FILEMANAGER_H
