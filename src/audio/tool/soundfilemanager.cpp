/** @file soundfilemanager.cpp

    @brief Singleton manager for all sound files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#include <QMap>

#include "soundfilemanager.h"
#include "soundfile.h"

namespace MO {
namespace AUDIO {

class SoundFileManager::Private
{
public:
    QMap<QString, SoundFile*> soundFiles_;
};

SoundFileManager * SoundFileManager::instance_ = 0;


SoundFileManager::SoundFileManager()
    : p_    (new Private())
{
}

SoundFileManager::~SoundFileManager()
{
    for (auto s : p_->soundFiles_)
        delete s;
    delete p_;
}

SoundFileManager * SoundFileManager::getInstance_()
{
    if (!instance_)
        instance_ = new SoundFileManager();
    return instance_;
}


SoundFile * SoundFileManager::getSoundFile(const QString &filename)
{
    auto sfm = getInstance_();

    auto i = sfm->p_->soundFiles_.find(filename);
    // already loaded
    if (i != sfm->p_->soundFiles_.end())
        return i.value();

    // create new
    SoundFile * sf = new SoundFile();
    sfm->p_->soundFiles_.insert(filename, sf);

    return sf;
}


} // namespace AUDIO
} // namespace MO
