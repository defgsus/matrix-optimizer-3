/** @file clientfiles.cpp

    @brief Client file manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/21/2014</p>
*/

#include <QMap>
#include <QFileInfo>
#include <QDateTime>

#include "clientfiles.h"
#include "io/xmlstream.h"
#include "io/settings.h"
#include "io/error.h"
#include "network/netevent.h"
#include "engine/clientengine.h"
#include "io/application.h"
#include "network/netlog.h"

namespace MO {
namespace IO {

class ClientFiles::Private
{
public:

    Private() : saveCachePending(false) { }

    struct FileInfo
    {
        QString serverFilename, clientFilename;
        QDateTime serverTime, clientTime;
        bool serverPresent, clientPresent;
    };

    void loadCache();
    void saveCache();
    void checkLocalPresense();

    /** serverFilename, FileInfo */
    QMap<QString, FileInfo> files;

    bool saveCachePending;
};



ClientFiles & clientFiles()
{
    static ClientFiles * instance_ = 0;
    if (!instance_)
        instance_ = new ClientFiles(application);

    return *instance_;
}


ClientFiles::ClientFiles(QObject *parent)
    : QObject   (parent),
      p_        (new Private())
{
    p_->loadCache();
}

ClientFiles::~ClientFiles()
{
    delete p_;
}


void ClientFiles::fetchFile(const QString &serverFilename)
{
    auto it = p_->files.find(serverFilename);

    // have it in cache-file?
    if (it != p_->files.end())
    {
        // is actually here?
        if (it.value().clientPresent)
        {
            // matches server-time?
            // XXX server should be querried
            if (it.value().clientTime == it.value().serverTime)
            {
                emit fileReady(serverFilename, it.value().clientFilename);
            }
        }
    }

    // request file

    auto event = new NetEventRequest();
    event->setRequest(NetEventRequest::GET_SERVER_FILE);
    event->setData(serverFilename);

    if (!clientEngine().sendEvent(event))
        emit fileNotReady(serverFilename);
}

void ClientFiles::receiveFileInfo(NetEventFileInfo * e)
{
    auto it = p_->files.find(e->filename());

    // update existing cache info
    if (it == p_->files.end())
    {
        it.value().serverTime = e->time();
        it.value().serverPresent = e->isPresent();

        saveCache();
        return;
    }

    // create new entry
    Private::FileInfo f;
    f.serverFilename = e->filename();
    f.serverTime = e->time();
    f.serverPresent = e->isPresent();
    f.clientPresent = false;
    p_->files.insert(f.serverFilename, f);

    saveCache();
}

void ClientFiles::receiveFile(NetEventFile * e)
{

}


void ClientFiles::saveCache()
{
    if (p_->saveCachePending)
        return;

    p_->saveCachePending = true;

    metaObject()->invokeMethod(
                this, SLOT(saveCacheNow()), Qt::QueuedConnection);
}

void ClientFiles::saveCacheNow()
{
    p_->saveCache();
}

void ClientFiles::Private::saveCache()
{
    saveCachePending = false;

    try
    {
        IO::XmlStream xml;
        xml.startWriting("mo-file-cache");

        for (const Private::FileInfo& f : files)
        {
            xml.newSection("file");
            xml.write("name", f.serverFilename);
            xml.write("time", f.serverTime.toString());
            xml.write("local-name", f.clientFilename);
            xml.write("local-time", f.clientTime);
            xml.endSection();
        }

        xml.stopWriting();
        xml.save(settings->getValue("File/filecache").toString());
    }
    catch (const Exception& e)
    {
        MO_IO_WARNING(WRITE, "Could not save file-cache\n" << e.what());
    }
}

void ClientFiles::Private::loadCache()
{
    files.clear();

    IO::XmlStream xml;

    QString fn = settings->getValue("File/filecache").toString();
    if (!QFileInfo(fn).exists())
        return;

    try
    {
        xml.load(fn);
        xml.startReading("mo-file-cache");

        while (xml.nextSubSection())
        {
            if (xml.isSection("file"))
            {
                FileInfo f;
                f.serverFilename = xml.expectString("name");
                f.serverTime = xml.expectDateTime("time");
                f.clientFilename = xml.expectString("local-name");
                f.clientTime = xml.expectDateTime("local-time");
                f.serverPresent = false;
                f.clientPresent = false;

                files.insert(f.serverFilename, f);
            }
            xml.leaveSection();
        }
    }
    catch (const Exception& e)
    {
        MO_IO_WARNING(READ, "Could not load file-cache\n" << e.what());
    }

    checkLocalPresense();
}

void ClientFiles::Private::checkLocalPresense()
{
    for (FileInfo& f : files)
    {
        QFileInfo info(f.clientFilename);

        f.clientPresent = info.exists();
        if (f.clientPresent)
            f.clientTime = info.lastModified();
    }
}


} // namespace IO
} // namespace MO
