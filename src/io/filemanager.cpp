/** @file filemanager.cpp

    @brief Filename and files manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/20/2014</p>
*/

#include "filemanager.h"
#include "application.h"

namespace MO {
namespace IO {

FileManager & fileManager()
{
    static FileManager * instance_ = 0;
    if (!instance_)
        instance_ = new FileManager(application);

    return *instance_;
}


FileManager::FileManager(QObject *parent) :
    QObject(parent)
{
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

} // namespace IO
} // namespace MO
