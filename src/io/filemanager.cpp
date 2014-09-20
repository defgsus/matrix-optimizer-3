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
#ifndef MO_CLIENT
#   include "engine/serverengine.h"
#else
#   include "engine/clientengine.h"
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
        FileType type;
        QString filename, localFilename;
    };

    QMap<QString, File> files;
};


FileManager::FileManager(QObject *parent)
    : QObject   (parent),
      p_        (new Private())
{
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
    return filename;

#endif
}


void FileManager::reset()
{
    p_->files.clear();
}

void FileManager::addFilename(FileType ft, const QString &filename)
{
    if (p_->files.contains(filename))
        return;

    Private::File f;
    f.type = ft;
    f.filename = filename;
    f.present = false;

    p_->files.insert(filename, f);
}

void FileManager::acquireFiles()
{
    for (Private::File & f : p_->files)
    {
        if (!f.present)
        {
            // suppose resource-files as always present
            if (f.filename.startsWith(":"))
            {
                f.present = true;
                continue;
            }

#ifndef MO_CLIENT

            // XXX currently not really used

            QFileInfo inf(f.filename);
            f.present = inf.exists();
            f.localFilename = f.filename;

#else // MO_CLIENT

            // TODO

#endif
        }
    }
}



} // namespace IO
} // namespace MO
