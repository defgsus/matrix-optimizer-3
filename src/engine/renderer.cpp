/** @file renderer.cpp

    @brief To-disk renderer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/25/2014</p>
*/

#include <sndfile.h>

#include <QDir>

#include "renderer.h"
#include "io/error.h"
#include "io/log.h"
#include "object/scene.h"


namespace MO {


Renderer::Renderer(QObject *parent) :
    QThread(parent)
{
}


/*
void Renderer::createPaths_()
{
    QDir dir(path_);

    if (!dir.mkpath(path_))
    {
        MO_IO_ERROR(WRITE, "Could not create rendering directory '"
                    << path_ << "'");
    }
}
*/

bool Renderer::prepareRendering()
{
    if (!scene_)
        return false;

    if (path_.isEmpty())
        return false;

    if (!QDir().mkpath(path_))
        return false;

    return true;
}

void Renderer::run()
{
    MO_DEBUG_RENDER("Renderer started for '" << path_ << "'");

    MO_ASSERT(scene_, "no scene to render");
    MO_ASSERT(!path_.isEmpty(), "no path given to render");

    QString wavName = path_ + "/audio.wav";

    const uint
            thread = 1,
            numChannels = scene_->microphones().size(),
            sampleRate = scene_->sampleRate(),
            bufferSize = scene_->bufferSize(thread);

    MO_DEBUG_RENDER("Opening wave files for writing");

    // create libsndfile structures
    SF_INFO info;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
    info.samplerate = scene_->sampleRate();
    info.channels = numChannels;

    // open wav file
    SNDFILE * wav = sf_open(wavName.toStdString().c_str(), SFM_WRITE, &info);

    // create audio buffer
    std::vector<F32> buffer(bufferSize * numChannels);

    MO_DEBUG_RENDER("Start rendering");

    const uint length = 10 * sampleRate / bufferSize;
    SamplePos pos = 0;
    for (uint i = 0; i < length; ++i)
    {
        scene_->calculateAudioBlock(pos, thread);
        scene_->getAudioOutput(numChannels, thread, &buffer[0]);

        sf_writef_float(wav, &buffer[0], bufferSize);

        pos += bufferSize;
    }

    MO_DEBUG_RENDER("Closing wave files");

    sf_close(wav);

    MO_DEBUG_RENDER("Finished");
}


} // namespace MO
