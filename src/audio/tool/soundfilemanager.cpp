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

SoundFileManager * SoundFileManager::p_instance_ = 0;


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

SoundFileManager * SoundFileManager::p_getInstance_()
{
    if (!p_instance_)
        p_instance_ = new SoundFileManager();
    return p_instance_;
}


SoundFile * SoundFileManager::getSoundFile(const QString &filename_, bool loadToMemory)
{
    QString filename = IO::fileManager().localFilename(filename_),
            key = filename + (loadToMemory ? "_mem" : "_stream");

    MO_DEBUG_SND("SoundFileManager::getSoundFile('"
                 << filename_ << "', " << loadToMemory);

    auto sfm = p_getInstance_();

    {
        QReadLocker lock(&sfm->p_->lock);

        auto i = sfm->p_->soundFiles_.find(key);
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
    sfm->p_->soundFiles_.insert(key, file);

    // try to load
    try
    {
        if (loadToMemory)
            sf->p_loadFile_(filename);
        else
            sf->p_openStream_(filename);
    }
    catch (Exception & e)
    {
        // XXX how to signal the error?
        MO_IO_WARNING(READ, "loading soundfile failed: \n"
                      << e.what());

        // SoundFile::isOk() will be false
    }

    return sf;
}

SoundFile * SoundFileManager::createSoundFile(uint channels, uint samplerate)
{
    SoundFile * sf = new SoundFile();
    sf->p_create_(channels, samplerate, 32);

    auto sfm = p_getInstance_();

    // install
    QWriteLocker lock(&sfm->p_->lock);
    Private::File file;
    file.sf = sf;
    file.count = 1;
    sfm->p_->soundFiles_.insert("audio_" + QString::number(ulong(sf), 16) + "_mem", file);

    return sf;
}

void SoundFileManager::releaseSoundFile(SoundFile * sf)
{
    MO_DEBUG_SND("SoundFileManager::releaseSoundFile(" << sf << ")");

    QString key = sf->filename() + (sf->isStream() ? "_stream" : "_mem");

    auto sfm = p_getInstance_();

    QWriteLocker lock(&sfm->p_->lock);

    auto i = sfm->p_->soundFiles_.find(key);

    // check for existence
    // (unlikely this would fail)
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
