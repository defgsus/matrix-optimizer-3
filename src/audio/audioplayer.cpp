/** @file audioplayer.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.04.2015</p>
*/

#include <QThread>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QList>
#include <QMessageBox>


#include "audioplayer.h"
#include "audiodevice.h"
#include "audioplayerdata.h"
#include "tool/audiobuffer.h"
#include "tool/soundfile.h"
#include "tool/soundfilemanager.h"
#include "tool/locklessqueue.h"
#include "io/currentthread.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace AUDIO {




namespace {

    class AudioPlayerThread : public QThread
    {
    public:

        AudioPlayerThread(AudioPlayerPrivate * p) : p(p) { }

        AudioPlayerPrivate * p;
        bool doStop;

        void run() Q_DECL_OVERRIDE;
    };

}



class AudioPlayerPrivate
{
public:
    AudioPlayerPrivate()
        : thread(this)
    { }

    bool open();
    bool close();

    void addData(AudioPlayerData*); //! locked
    void removeData(AudioPlayerData*); //! locked
    bool removeAllData();

    void mixBlock(F32 * ); //! interlaced blockSize * channels

    AudioDevice device;
    QReadWriteLock dataLock;
    QList<AudioPlayerData*> dataList;
    LocklessQueue<const F32*> audioOutQueue;
    std::vector<F32> buffer;
    AudioPlayerThread thread;
};

AudioPlayerPrivate * AudioPlayer::p_()
{
    static AudioPlayerPrivate * p = 0;
    return p ? p : p = new AudioPlayerPrivate();
}

bool AudioPlayerPrivate::open()
{
    if (device.isPlaying())
        return true;

    if (!device.initFromSettings())
        return false;

    device.setCallback([this](const F32 * , F32 * out)
    {
        // get output from AudioPlayerThread
        const F32 * buf;
        if (audioOutQueue.consume(buf))
            memcpy(out, buf, device.bufferSize()
                                * device.numOutputChannels()
                                * sizeof(F32));
        else
        {
            memset(out, 0, device.bufferSize()
                                * device.numOutputChannels()
                                * sizeof(F32));
            //MO_WARNING("audio-out buffer underrun");
        }
    });

    // init communication stuff
    audioOutQueue.reset();

    // start threads
    thread.start();

    // start device
    try
    {
        device.start();
        return true;
    }
    catch (const Exception& e)
    {
        thread.doStop = true;

        QMessageBox::critical(0, QMessageBox::tr("Audio device"),
                              QMessageBox::tr("Could initialized but not start audio playback.\n%1")
                              .arg(e.what()));
        return false;
    }
}

bool AudioPlayerPrivate::close()
{
    if (!device.isPlaying())
        return false;
    device.close();
    return true;
}


void AudioPlayerPrivate::addData(AudioPlayerData * d)
{
    QWriteLocker lock(&dataLock);

    dataList.append(d);
    d->addRef();
    MO_PRINT("AudioPlayer::addData(" << d << "): " << d->lengthSamples() << "x"
             << d->numChannels() << " @ " << d->sampleRate() << "hz");
}

void AudioPlayerPrivate::removeData(AudioPlayerData * d)
{
    QWriteLocker lock(&dataLock);

    int num = dataList.removeAll(d);
    for (int i=0; i<num; ++i)
        d->releaseRef();
}

bool AudioPlayerPrivate::removeAllData()
{
    QWriteLocker lock(&dataLock);

    bool r = !dataList.isEmpty();

    for (auto d : dataList)
        d->releaseRef();
    dataList.clear();

    return r;
}

void AudioPlayerPrivate::mixBlock(F32 * dst1)
{
    MO_ASSERT(dst1, "");

    // clear buffer
    const size_t bs = device.bufferSize() * device.numOutputChannels();
    for (size_t i=0; i<bs; ++i)
        dst1[i] = 0.f;

    if (buffer.size() != bs)
        buffer.resize(bs);

    QReadLocker lock(&dataLock);

    for (AudioPlayerData *& data : dataList)
    {
        // get each buffer
        F32 * src = &buffer[0], * dst = dst1;
        size_t r = data->get(src, device.numOutputChannels(), device.bufferSize());
        size_t chan = data->numChannels();

        // finished?
        if (r < device.bufferSize())
        {
            MO_PRINT("AudioPlayer::data finished (" << data
                     << "): " << data->lengthSamples());
            data->releaseRef();
            data = 0;
            continue;
        }

        // add + interlace
        F32 * b = dst;
        for (size_t i=0; i<chan; ++i)
        {
            for (size_t j=0; j<r; ++j, b += device.numOutputChannels())
                *b += *src++;
            // advance channel
            ++dst;
        }
    }

    dataList.removeAll(0);
}


void AudioPlayerThread::run()
{
    setCurrentThreadName("AUPLAY");
    MO_DEBUG_AUDIO("AudioPlayerThread::run()");

    uint    bufferSize = p->device.bufferSize(),
            numChannelsOut = p->device.numOutputChannels(),
            numAhead = 4;

    AUDIO::AudioBuffer
            bufferForDevice(bufferSize * numChannelsOut, numAhead);

    // length of a buffer in microseconds
    unsigned long bufferTimeU =
        Double(1000000 * bufferSize) / p->device.sampleRate();

    // tiredlessly..
    while (!doStop)
    {
        // calc buffers for next system-out callback
        if (p->audioOutQueue.count() < numAhead)
        {
            auto kkk = bufferForDevice.writePointer();

            p->mixBlock( kkk );

            // publish
            bufferForDevice.nextBlock();
            p->audioOutQueue.produce( bufferForDevice.readPointer() );
        }
        else
            usleep(bufferTimeU);
    }
}












size_t AudioPlayer::sampleRate()
{
    return p_()->device.isOk()
            ? p_()->device.sampleRate()
            : 44100;
}

size_t AudioPlayer::numChannels()
{
    return p_()->device.isOk()
            ? p_()->device.numOutputChannels()
            : 2;
}

bool AudioPlayer::open()
{
    return p_()->open();
}

bool AudioPlayer::close()
{
    return p_()->close();
}

bool AudioPlayer::play(AudioPlayerData * d)
{
    if (!p_()->device.isPlaying())
        if (!open())
            return false;

    p_()->addData(d);
    return true;
}

bool AudioPlayer::stop() { return p_()->removeAllData(); }


} // namespace AUDIO
} // namespace MO
