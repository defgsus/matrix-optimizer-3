/** @file clientfiles.h

    @brief Client file manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/21/2014</p>
*/

#ifndef MOSRC_IO_CLIENTFILES_H
#define MOSRC_IO_CLIENTFILES_H

#include <QObject>

#include "network/network_fwd.h"

namespace MO {
namespace IO {

class ClientFiles;

/** Returns singleton instance */
ClientFiles & clientFiles();

class ClientFiles : public QObject
{
    Q_OBJECT
public:
    /** After construction, the local cache-file will be loaded */
    explicit ClientFiles(QObject *parent = 0);
    ~ClientFiles();

signals:

    void fileReady(const QString& serverFilename, const QString& clientFilename);
    void fileNotReady(const QString& serverFilename);

public slots:

    /** Requests saving the current info-cache to disk, in the next event-cycle */
    void saveCache();

    /** Immediately saves the current info-cache to disk */
    void saveCacheNow();

    /** Look out for the given file.
        First the cache is checked and if necessary the server is querried for the file.
        fileReady() or fileNotReady() will be emitted. */
    void fetchFile(const QString& serverFilename);

    /** Called by ClientEngine upon event from server */
    void receiveFileInfo(NetEventFileInfo *);
    /** Called by ClientEngine upon event from server */
    void receiveFile(NetEventFile *);

private:

    class Private;
    Private * p_;
};

} // namespace IO
} // namespace MO


#endif // MOSRC_IO_CLIENTFILES_H
