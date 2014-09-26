/** @file soundfilemanager.cpp

    @brief Singleton manager for all sound files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#include <QMap>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include "soundfilemanager.h"
#include "soundfile.h"
#include "io/error.h"
#include "io/log.h"
#include "io/filemanager.h"

namespace MO {
namespace AUDIO {

class SoundFileManager::Private
{
public:
    struct File
    {
        SoundFile * sf;
        int count;
    };

    QMap<QString, File> soundFiles_;

    QReadWriteLock lock;
};

SoundFileManager * SoundFileManager::instance_ = 0;


SoundFileManager::SoundFileManager()
    : p_    (new Private())
{
    MO_DEBUG_SND("SoundFileManager::SoundFileManager()");
}

SoundFileManager::~SoundFileManager()
{
    MO_DEBUG_SND("SoundFileManager::~SoundFileManager()");

    for (auto s : p_->soundFiles_)
        delete s.sf;
    delete p_;
}

SoundFileManager * SoundFileManager::getInstance_()
{
    if (!instance_)
        instance_ = new SoundFileManager();
    return instance_;
}


SoundFile * SoundFileManager::getSoundFile(const QString &filename_)
{
    QString filename = IO::fileManager().localFilename(filename_);

    MO_DEBUG_SND("SoundFileManager::getSoundFile('" << filename_ << "'");

    auto sfm = getInstance_();

    {
        QReadLocker lock(&sfm->p_->lock);

        auto i = sfm->p_->soundFiles_.find(filename);
        // already loaded
        if (i != sfm->p_->soundFiles_.end())
        {
            i.value().count++;
            return i.value().sf;
        }
    }

    QWriteLocker lock(&sfm->p_->lock);

    // create new
    SoundFile * sf = new SoundFile();
    Private::File file;
    file.sf = sf;
    file.count = 1;
    sfm->p_->soundFiles_.insert(filename, file);

    // try to load
    try
    {
        sf->loadFile_(filename);
    }
    catch (Exception & e)
    {
        // XXX how to signal the error?
        MO_IO_WARNING(READ, "loading soundfile failed: \n"
                      << e.what());

        // SoundFile::ok() will be false
    }

    return sf;
}

void SoundFileManager::releaseSoundFile(SoundFile * sf)
{
    MO_DEBUG_SND("SoundFileManager::releaseSoundFile(" << sf << ")");

    auto sfm = getInstance_();

    QWriteLocker lock(&sfm->p_->lock);

    auto i = sfm->p_->soundFiles_.find(sf->filename());

    // check for existence
    // (very unlikely this fails)
    if (i == sfm->p_->soundFiles_.end())
    {
        MO_WARNING("SoundFileManager::releaseSoundFile() called for unknown SoundFile\n"
                   "'" << sf->filename() << "'");
        return;
    }

    // count down
    i.value().count--;

    // destroy if not needed anymore
    if (!i.value().count)
    {
        delete i.value().sf;
        sfm->p_->soundFiles_.erase(i);
    }
}

} // namespace AUDIO
} // namespace MO
