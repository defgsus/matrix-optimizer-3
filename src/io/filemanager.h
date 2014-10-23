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

    /** Debug dump */
    void dumpStatus() const;

    // ------------------ setter --------------------

    /** Clears the list of known filenames */
    void clear();

    /** Adds a filename to the list of files that are needed. */
    void addFilename(FileType ft, const QString& filename);

    /** Adds all the filenames to the list of files that are needed. */
    void addFilenames(const FileList&);

    /** Starts looking for files.
        On clients, the files will be transferred if needed. */
    void acquireFiles();

signals:

    /** Emitted when all files are ready */
    void filesReady();

    /** Emitted when a file is not ready */
    void fileNotReady(const QString& filename);

    /** Emitted when all files are fetched for but not all are ready. */
    void finished();

private slots:

#ifdef MO_CLIENT
    void onFileReady_(const QString& serverName, const QString& localName);
    void onFileNotReady_(const QString&);
    void checkReadyOrFinished_();
#endif

private:

    class Private;
    Private * p_;
};


} // namespace IO
} // namespace MO


#endif // MOSRC_IO_FILEMANAGER_H
