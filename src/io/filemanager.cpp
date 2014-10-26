/** @file filemanager.cpp

    @brief Filename and files manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/20/2014</p>
*/

#include <QMap>
#include <QFileInfo>

#include "filemanager.h"
#include "application.h"
#include "io/error.h"
#ifndef MO_CLIENT
//#   include "engine/serverengine.h"
#else
#   include "engine/clientengine.h"
#   include "io/clientfiles.h"
#   include "network/netlog.h"
#endif

namespace MO {
namespace IO {

FileManager & fileManager()
{
    static FileManager * instance_ = 0;
    if (!instance_)
        instance_ = new FileManager(application);

    return *instance_;
}


class FileManager::Private
{
public:

    struct File
    {
        bool present;
        bool queried;
        // we tried to fetch, but there was an error
        bool error;
        FileType type;
        QString filename, localFilename;
    };

    QMap<QString, File> files;
};


FileManager::FileManager(QObject *parent)
    : QObject   (parent),
      p_        (new Private())
{
#ifdef MO_CLIENT
    connect(&clientFiles(), SIGNAL(fileReady(QString,QString)),
            this, SLOT(onFileReady_(QString,QString)));
    connect(&clientFiles(), SIGNAL(fileNotReady(QString)),
            this, SLOT(onFileNotReady_(QString)));
#endif
}

FileManager::~FileManager()
{
    delete p_;
}

QString FileManager::localFilename(const QString &filename)
{
    if (filename.startsWith(":"))
        return filename;

#ifndef MO_CLIENT
    return filename;
#else

    // XXX
    auto it = p_->files.find(filename);
    if (it == p_->files.end())
    {
        MO_WARNING("FileManager::localFilename() request of unknown file " << filename);
        return filename;
    }

    return it.value().localFilename;

#endif
}


void FileManager::clear()
{
    p_->files.clear();
}

void FileManager::addFilenames(const FileList & list)
{
    for (const FileListEntry& f : list)
    {
        addFilename(f.second, f.first);
    }
}

void FileManager::addFilename(FileType ft, const QString &filename)
{
    if (p_->files.contains(filename))
        return;

    Private::File f;
    f.type = ft;
    f.filename = filename;
    f.present = false;
    f.queried = false;
    f.error = false;

    p_->files.insert(filename, f);
}

void FileManager::acquireFiles()
{
    bool allpresent = true;

    for (Private::File & f : p_->files)
    {
        if (!f.present)
        {
            // suppose resource-files are always present
            if (f.filename.startsWith(":"))
            {
                f.present =
                f.queried = true;
#ifdef MO_CLIENT
                onFileReady_(f.filename, f.filename);
#endif
                continue;
            }

#ifndef MO_CLIENT

            // XXX currently not really used

            QFileInfo inf(f.filename);
            f.present = inf.exists();
            f.localFilename = f.filename;
            if (!f.present)
            {
                f.error = true;
                emit fileNotReady(f.filename);
                allpresent = false;
            }

#else // MO_CLIENT

            clientFiles().fetchFile(f.filename);
            f.queried = true;
            allpresent = false;
#endif
        }
    }

    if (allpresent)
        emit filesReady();
#ifdef MO_CLIENT
    else
        checkReadyOrFinished_();
#endif

#ifndef MO_CLIENT
    // XXX not used either
    if (!allpresent)
        emit finished();
#endif

}


#ifdef MO_CLIENT

void FileManager::onFileReady_(const QString &serverName, const QString &localName)
{
    auto it = p_->files.find(serverName);
    if (it == p_->files.end())
    {
        MO_NETLOG(WARNING, "Received file-ready for unrequested file '" << serverName << "'");
        return;
    }

    it.value().present = true;
    it.value().error = false;
    it.value().localFilename = localName;

    checkReadyOrFinished_();
}

void FileManager::onFileNotReady_(const QString & fn)
{
    auto it = p_->files.find(fn);
    if (it == p_->files.end())
    {
        MO_NETLOG(WARNING, "Received file-not-ready for unrequested file '" << fn << "'");
        return;
    }

    it.value().error = true;

    emit fileNotReady(fn);

    checkReadyOrFinished_();
}

void FileManager::checkReadyOrFinished_()
{
    // check for complete readyness
    int ready = 0;
    for (const Private::File& f : p_->files)
    {
        if (f.present)
            ++ready;
    }

    MO_NETLOG(DEBUG, "FileManager: files ready " << ready << "/" << p_->files.size());
    dumpStatus();

    if (ready == p_->files.size())
    {
        MO_NETLOG(DEBUG, "FileManager: all files ready");
        emit filesReady();
        return;
    }

    // check if all are queried and all queries have been answered
    for (const Private::File& f : p_->files)
    {
        if (!f.queried)
            return;
        else
            if (!f.error && !f.present)
                return;
    }

    MO_NETLOG(DEBUG, "FileManager: finished but not all ready ready");

    emit finished();
}

#endif


void FileManager::dumpStatus() const
{
    for (const Private::File& f : p_->files)
    {
        std::cout << (f.present ? "P" : ".")
                  << (f.queried ? "Q" : ".")
                  << (f.error   ? "E" : ".")
                  << " " << f.filename.toStdString()
                  << " (" << f.localFilename.toStdString() << ")"
                  << std::endl;
    }
}


} // namespace IO
} // namespace MO
