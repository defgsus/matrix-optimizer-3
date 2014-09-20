/** @file filemanager.h

    @brief Filename and files manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/20/2014</p>
*/

#ifndef MOSRC_IO_FILEMANAGER_H
#define MOSRC_IO_FILEMANAGER_H

#include <QObject>

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

    /** Returns the local filename for the given @p filename.
        Resource filenames (starting with ':') are returns as-is.
        On clients, the file is actually transferred before return of the function. */
    QString localFilename(const QString& filename);

signals:

public slots:

};


} // namespace IO
} // namespace MO


#endif // MOSRC_IO_FILEMANAGER_H
