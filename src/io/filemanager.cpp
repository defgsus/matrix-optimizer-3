/** @file filemanager.cpp

    @brief Filename and files manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/20/2014</p>
*/

#include <QMap>
#include <QFileInfo>
#include <QDir>

#include "filemanager.h"
#include "application.h"
#include "io/error.h"
#include "engine/serverengine.h"
#include "engine/clientengine.h"
#include "io/clientfiles.h"
#include "network/netlog.h"

namespace MO {
namespace IO {

FileManager & fileManager()
{
    static FileManager * instance_ = 0;
    if (!instance_)
        instance_ = new FileManager(application());

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

    QStringList searchPaths;

    /** Tries to locate f in local search paths.
        Sets present and localFilename fields. */
    void checkSearchPaths(File& f);
};


FileManager::FileManager(QObject *parent)
    : QObject   (parent),
      p_        (new Private())
{
    addSearchPath(".");

    if (isClient())
    {
        connect(&clientFiles(), SIGNAL(fileReady(QString,QString)),
                this, SLOT(onFileReady_(QString,QString)));
        connect(&clientFiles(), SIGNAL(fileNotReady(QString)),
                this, SLOT(onFileNotReady_(QString)));
        //connect(&clientFiles(), SIGNAL(allFilesReady()),
        //        this, SLOT(checkReadyOrFinished_()));
    }
}

FileManager::~FileManager()
{
    delete p_;
}

QString FileManager::localFilename(const QString &filename)
{
    if (filename.startsWith(":"))
        return filename;

    // XXX
    auto it = p_->files.find(filename);
    if (it == p_->files.end())
    {
        if (isClient())
        {
            MO_WARNING("FileManager::localFilename() request of previously unknown file " << filename);
            return filename;
        }
        else
        {
            // XXX more like a hack
            // on desktops/servers this will
            // look in searchpaths for the unknown files
            // which helps to load waves in sequences
            // unfortunately they are tried to load
            // on scene object construction...

            // create entry
            Private::File f;
            //f.type = XXX unknown here;
            f.filename = f.localFilename = filename;
            f.queried = true;
            f.error = false;
            QFileInfo inf(f.filename);
            f.present = inf.exists();
            if (!f.present)
                // look in local search paths
                p_->checkSearchPaths(f);

            p_->files.insert(filename, f);

            return f.localFilename;
        }
    }

    return it.value().localFilename;

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
    if (isClient())
        aquireFilesClient_();
    else
        aquireFilesServer_();
}

void FileManager::aquireFilesServer_()
{
    bool allpresent = true;

    for (Private::File & f : p_->files)
    {
        if (!f.queried)
        {
            // suppose resource-files are always present
            if (f.filename.startsWith(":"))
            {
                f.present =
                f.queried = true;
                continue;
            }

            // check existence
            QFileInfo inf(f.filename);
            f.queried = true;
            f.present = inf.exists();
            f.localFilename = f.filename;
            if (!f.present)
                // look in local search paths
                p_->checkSearchPaths(f);

            // emit file error
            if (!f.present)
            {
                f.error = true;
                emit fileNotReady(f.filename);
                allpresent = false;
            }
        }
    }

    if (allpresent)
        emit filesReady();
    // XXX currently not used
    else
        emit finished();
}


void FileManager::aquireFilesClient_()
{
    bool allpresent = true;

    for (Private::File & f : p_->files)
    {
        if (!f.queried) // XXX was f.present, should be f.querried but not tested for client
        {
            // suppose resource-files are always present
            if (f.filename.startsWith(":"))
            {
                f.present =
                f.queried = true;
                onFileReady_(f.filename, f.filename);
                continue;
            }

            clientFiles().fetchFile(f.filename);
            f.queried = true;
            allpresent = false;
        }
    }

    if (allpresent)
        emit filesReady();
    else
        checkReadyOrFinished_();
}



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

    if (ready >= p_->files.size())
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

    MO_NETLOG(DEBUG, "FileManager: finished but not all ready");

    emit finished();
}


void FileManager::dumpStatus() const
{
    for (const Private::File& f : p_->files)
    {
        std::cout << (f.queried ? "Q" : ".")
                  << (f.present ? "P" : ".")
                  << (f.error   ? "E" : ".")
                  << " " << f.filename.toStdString()
                  << " (" << f.localFilename.toStdString() << ")"
                  << std::endl;
    }
}


void FileManager::clearSearchPaths()
{
    p_->searchPaths.clear();
}


void FileManager::addSearchPath(const QString &path)
{
    p_->searchPaths << path;
}

void FileManager::Private::checkSearchPaths(File & finf)
{
    /* suppose:
     *  /home/someuser/mo/data/graphic/exciting.png
     * should match:
     *  /home/otheruser/app/mo3/data/graphic/exciting.png
     *
     * it would be really nice to find the common root
     * in this case data/
     * right now, we just assume data/ is it..
     */

    finf.present = false;

    // check if contains data/
    int idx = finf.filename.indexOf("data/");
    if (idx < 0)
        idx = finf.filename.indexOf("data\\");
    if (idx < 0)
        return;

    QString fn = finf.filename.mid(idx);

    for (auto & p : searchPaths)
    {
        // see if searchpath + / + data/... is present
        QFileInfo inf(p + QDir::separator() + fn);
//        std::cout << "------- " << (p + QDir::separator() + fn) << std::endl;
        if (inf.exists())
        {
//            std::cout << "EXISTS" << std::endl;
            finf.present = true;
            finf.localFilename = fn;
            return;
        }
//        std::cout << "EXISTS NOT" << std::endl;
    }
}


} // namespace IO
} // namespace MO

