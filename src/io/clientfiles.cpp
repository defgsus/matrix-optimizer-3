/** @file clientfiles.cpp

    @brief Client file manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/21/2014</p>
*/

//#include <QDebug>

#include <QMap>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>

#include "clientfiles.h"
#include "io/xmlstream.h"
#include "io/settings.h"
#include "io/error.h"
#include "network/netevent.h"
#include "engine/clientengine.h"
#include "io/application.h"
#include "network/netlog.h"
#include "io/log.h"

namespace MO {
namespace IO {

class ClientFiles::Private
{
public:

    Private(ClientFiles * cf) : cf(cf), saveCachePending(false) { }

    struct FileInfo
    {
        QString serverFilename,
                clientFilename;
        QDateTime
        /** If up-to-date serverTime matches clientTime. */
                serverTime,
        /** Set to serverTime on transfer */
                clientTime;
        bool    serverPresent,
                clientPresent,
        /** Server has submitted file time (serverTime) ? */
                timeUpdated,
        /** Server was querried for file time */
                infoRequested,
        /** Server was querried for the file */
                transferRequested;
    };

    void loadCache();
    void saveCache();
    void checkLocalPresense();
    void queryServerTime();
    void requestFileTime(const QString& serverFilename);
    void requestFile(const QString& serverFilename);
    bool checkAllReady();

    ClientFiles * cf;

    /** serverFilename, FileInfo */
    QMap<QString, FileInfo> files;

    bool saveCachePending;
};



ClientFiles & clientFiles()
{
    static ClientFiles * instance_ = 0;
    if (!instance_)
        instance_ = new ClientFiles(application());

    return *instance_;
}


ClientFiles::ClientFiles(QObject *parent)
    : QObject   (parent),
      p_        (new Private(this))
{
    // be sure the directory for cached files is present
    const QString cachedir = settings()->getValue("Directory/filecache").toString();

    QDir dir(cachedir);
    if (!dir.exists() && !dir.mkpath("."))
        MO_NETLOG(ERROR, "ClientFiles is unable to access/create filecache directory '"
                  << cachedir << "'");

    p_->loadCache();
}

ClientFiles::~ClientFiles()
{
    saveCacheNow();

    delete p_;
}


void ClientFiles::fetchFile(const QString &serverFilename)
{
    MO_NETLOG(DEBUG, "ClientFiles::fetchFile(" << serverFilename << ")");

    auto it = p_->files.find(serverFilename);

    // have it in cache-file?
    if (it == p_->files.end())
        MO_NETLOG(DEBUG, "ClientFiles: not in cache: '" << serverFilename << "'")
    else
    {
        // is actually here?
        if (!it.value().clientPresent)
            MO_NETLOG(DEBUG, "ClientFiles: in cache but not present: '" << serverFilename << "'")
        else
        {
            // matches server-time?
            if (!(it.value().timeUpdated
                  && it.value().clientTime == it.value().serverTime))
            {
                MO_NETLOG(DEBUG, "ClientFiles: in cache but outdated: '" << serverFilename << "'");
                p_->requestFileTime(it.value().serverFilename);
            }
            else
            {
                MO_NETLOG(DEBUG, "ClientFiles::fileReady(" << serverFilename << ")");
                emit fileReady(serverFilename, it.value().clientFilename);
            }
            return;
        }

    }

    // not there at all?
    p_->requestFile(serverFilename);
}


void ClientFiles::receiveFileInfo(NetEventFileInfo * e)
{
    MO_NETLOG(DEBUG, "ClientFiles::receiveFileInfo('" << e->filename() << "')");

    auto it = p_->files.find(e->filename());

    // update existing cache info
    if (it != p_->files.end())
    {
        MO_NETLOG(DEBUG, "ClientFiles: updating cache for '" << e->filename() << "'");

//        const QString datetime_format = "yyyy.MM.dd-hh:mm:ss.zzz";
//        qDebug() << "----------------" << e->time().toString();
//        qDebug() << "----------++++++" << e->time().toString(datetime_format);

        it.value().serverTime = e->time();
        it.value().serverPresent = e->isPresent();
        it.value().timeUpdated = true;

        saveCache();

        // if file time has changed, request the whole file
        if (it.value().clientTime < it.value().serverTime)
            p_->requestFile(it.value().serverFilename);
        else
        {
            MO_NETLOG(DEBUG, "ClientFiles: file is UP-TO-DATE '" << e->filename() << "'");
            // done?
            if (p_->checkAllReady())
            {
                MO_NETLOG(DEBUG, "READY----------------------");
                emit allFilesReady();
            }
        }
        return;
    }

    // create new entry
    Private::FileInfo f;
    f.serverFilename = e->filename();
    f.serverTime = e->time();
    f.serverPresent = e->isPresent();
    f.clientPresent = false;
    f.timeUpdated = true;
    // clear flag, once the info is there
    f.infoRequested = false;
    p_->files.insert(f.serverFilename, f);

    saveCache();
}

void ClientFiles::receiveFile(NetEventFile * e)
{
    MO_NETLOG(DEBUG, "ClientFiles::receiveFile('" << e->filename() << "')");

    Private::FileInfo * f;

    auto it = p_->files.find(e->filename());

    // create or reuse FileInfo field
    if (it != p_->files.end())
        f = &it.value();
    else
        f = &p_->files.insert(e->filename(), Private::FileInfo()).value();

    f->serverFilename = e->filename();
    f->serverTime = e->time();
    f->serverPresent = e->isPresent();
    // clear flag, once the file is there
    f->transferRequested = false;

    // construct a local filename
    f->clientFilename = f->serverFilename;
    f->clientFilename.replace("/","_");
    f->clientFilename.replace("\\","_");
    f->clientFilename.replace(":","_");
    f->clientFilename.insert(0,
            settings()->getValue("Directory/filecache").toString()
            + QDir::separator());

    f->clientTime = f->serverTime;
    f->timeUpdated = true;
    f->clientPresent = //f->serverPresent &&
                        e->saveFile(f->clientFilename);

    saveCache();

    if (!f->clientPresent)
        emit fileNotReady(f->serverFilename);
    else

    // done?
    if (p_->checkAllReady())
        emit allFilesReady();
}

void ClientFiles::update()
{
    p_->queryServerTime();
}


void ClientFiles::saveCache()
{
    if (p_->saveCachePending)
        return;

    p_->saveCachePending = true;

    metaObject()->invokeMethod(
                this, "saveCacheNow", Qt::QueuedConnection);
}

void ClientFiles::saveCacheNow()
{
    p_->saveCache();
}

void ClientFiles::Private::saveCache()
{
    MO_NETLOG(DEBUG, "ClientFiles::saveCache()");

    saveCachePending = false;

    try
    {
        IO::XmlStream xml;
        xml.startWriting("mo-file-cache");

        for (const Private::FileInfo& f : files)
        {
            xml.newSection("file");
            xml.write("name", f.serverFilename);
            xml.write("time", f.serverTime);
            xml.write("local-name", f.clientFilename);
            xml.write("local-time", f.clientTime);
            xml.endSection();
        }

        xml.stopWriting();
        xml.save(settings()->getValue("File/filecache").toString());
    }
    catch (const Exception& e)
    {
        MO_IO_WARNING(WRITE, "Could not save file-cache\n" << e.what());
    }
}


void ClientFiles::Private::loadCache()
{
    MO_NETLOG(DEBUG, "ClientFiles::loadCache()");

    files.clear();

    IO::XmlStream xml;

    QString fn = settings()->getValue("File/filecache").toString();
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
                f.timeUpdated = false;

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
    queryServerTime();
}

void ClientFiles::Private::checkLocalPresense()
{
    for (FileInfo& f : files)
    {
        QFileInfo info(f.clientFilename);

        f.clientPresent = info.exists();
    }
}

void ClientFiles::Private::queryServerTime()
{
    if (!clientEngine().isRunning())
        return;

    for (FileInfo& f : files)
    {
        auto event = new NetEventRequest();
        event->setRequest(NetEventRequest::GET_SERVER_FILE_TIME);
        event->setData(f.serverFilename);

        clientEngine().sendEvent(event);
    }
}

void ClientFiles::Private::requestFileTime(const QString& serverFilename)
{
    // check if already requested
    auto it = files.find(serverFilename);
    if (it != files.end())
    {
        //if (it.value().infoRequested)
        //    return;
    }

    MO_NETLOG(DEBUG, "ClientFiles::requestFileTime('" << serverFilename << "')");

    auto event = new NetEventRequest();
    event->setRequest(NetEventRequest::GET_SERVER_FILE_TIME);
    event->setData(serverFilename);

    // if sending failed
    if (!clientEngine().sendEvent(event))
        // we definitely won't get a file
        emit cf->fileNotReady(serverFilename);
    else
    {
        // flag as requested
        if (it != files.end())
            it.value().infoRequested = true;
    }
}

void ClientFiles::Private::requestFile(const QString& serverFilename)
{
    // check if already requested
    auto it = files.find(serverFilename);
    if (it != files.end())
    {
        if (it.value().transferRequested)
            return;
    }

    MO_NETLOG(DEBUG, "ClientFiles::requestFile('" << serverFilename << "')");

    auto event = new NetEventRequest();
    event->setRequest(NetEventRequest::GET_SERVER_FILE);
    event->setData(serverFilename);

    // if sending failed
    if (!clientEngine().sendEvent(event))
        // we definitely won't get a file
        emit cf->fileNotReady(serverFilename);
    else
    {
        // flag as requested
        if (it != files.end())
            it.value().transferRequested = true;
    }
}

bool ClientFiles::Private::checkAllReady()
{
    MO_DEBUG("ClientFiles:------")
    for (const FileInfo& f : files)
    {
        MO_DEBUG("serverpresent " << f.serverPresent
                 << " servertime " << f.serverTime.toString()
                 << " clientpresent " << f.clientPresent
                 << " clienttimer " << f.clientTime.toString()
                 << " timeupdate " << f.timeUpdated
                 << " equal " << (f.serverTime == f.clientTime)
                 << " fn " << f.serverFilename);
    }

    for (const FileInfo& f : files)
        if (!(f.serverPresent && f.clientPresent
              && f.timeUpdated
              && f.clientTime == f.serverTime))
            return false;

    MO_DEBUG("YESSSSSSSSSSSS");
    return true;
}

} // namespace IO
} // namespace MO
