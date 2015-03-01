/** @file dumbfile.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.12.2014</p>
*/

#ifndef MO_DISABLE_DUMB

#include <dumb.h>

#include <QFile>
#include <QBuffer>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include "dumbfile.h"
#include "audiobuffer.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace AUDIO {


class DumbFile::Private
{
public:
    Private(DumbFile * dumb)
        : dumb  (dumb),
          conf  (),
          duh   (0),
          render(0),
          nextRender(0),
          buffer(0),

          tempo(0),
          speed(0)
    { }

    void open(const QString& filename);
    void createRenderer();

    static bool static_init;
    static const F32 conversion;

    DumbFile * dumb;

    AUDIO::Configuration conf;

    QString filename;

    DUMBFILE * file;
    DUH * duh;
    DUH_SIGRENDERER * render;
    DUH_SIGRENDERER * nextRender;
    sample_t ** buffer;

    int tempo, speed;

    long lastPos;

    QReadWriteLock mutex;
};

bool DumbFile::Private::static_init = false;
#if DUMB_VERSION > 902
    const F32 DumbFile::Private::conversion = 1.f / F32(0x800000);
#else
    const F32 DumbFile::Private::conversion = 1.f / F32(0x8000);
#endif

DumbFile::DumbFile()
    : p_    (new Private(this))
{
}

DumbFile::DumbFile(const Configuration & c)
    : DumbFile()
{
    setConfig(c);
}


DumbFile::~DumbFile()
{
    close();
    delete p_;
}

bool DumbFile::isOpen() const
{
    return p_->duh;
}

bool DumbFile::isReady() const
{
    return p_->render && p_->buffer;
}

const QString& DumbFile::filename() const
{
    return p_->filename;
}

const AUDIO::Configuration& DumbFile::config() const
{
    return p_->conf;
}


void DumbFile::open(const QString &filename)
{
    MO_DEBUG("DumbFile::open(" << filename << ")");
    p_->open(filename);
}

void DumbFile::Private::open(const QString &filename)
{
    // open file
    // (rely on Qt here)

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
    {
        MO_IO_ERROR(READ, "Can't open dumbfile '" << filename << "'\n"
                    << file.errorString());
    }

    QByteArray data = file.readAll();

    DUMBFILE * dfile = dumbfile_open_memory(data.constData(), data.size());

    // get the DUH

    DUH * duh = dumb_read_s3m(dfile);
    if (!duh)
        duh = dumb_read_it(dfile);
    if (!duh)
        duh = dumb_read_mod(dfile);
    if (!duh)
        duh = dumb_read_xm(dfile);

    if (!duh)
    {
        MO_IO_ERROR(READ, "Dumb can't comprehent '" << filename << "'");
    }


    dumb->close();

    this->duh = duh;
    this->filename = filename;
    lastPos = 0;

    if (conf.isValid())
        createRenderer();
}

void DumbFile::close()
{
    if (p_->render)
        duh_end_sigrenderer(p_->render);
    p_->render = 0;

    // XXX deinit?
    p_->duh = 0;

    if (p_->buffer)
        destroy_sample_buffer(p_->buffer);
    p_->buffer = 0;

    p_->filename.clear();
}

long DumbFile::position() const
{
    return p_->render ? duh_sigrenderer_get_position(p_->render) : 0;
}


void DumbFile::setConfig(const AUDIO::Configuration& c)
{
    MO_DEBUG("DumbFile::setConfig(" << c << ")");

    p_->conf = c;

    p_->createRenderer();
}

void DumbFile::Private::createRenderer()
{
    MO_DEBUG("DumbFile::createRenderer()");

    if (render)
        duh_end_sigrenderer(render);

    if (!duh)
    {
        MO_DEBUG("DumbFile::createRenderer() no file loaded");
        render = 0;
        return;
    }

    render = duh_start_sigrenderer(duh, 0, conf.numChannelsOut(), lastPos);
    if (!render)
    {
        MO_WARNING("Can't create dumb renderer for '" << filename << "'");
        return;
    }
/*
    duh_sigrenderer_set_sample_analyser_callback(render,
                        [=](void *data, const sample_t *const *samples, int n_channels, long length)
    {
        MO_DEBUG(n_channels << " " << length);
    }, 0);
*/
    DUMB_IT_SIGRENDERER * itrender = duh_get_it_sigrenderer(render);

    tempo = dumb_it_sr_get_tempo(itrender);
    speed = dumb_it_sr_get_speed(itrender);


    buffer = allocate_sample_buffer(conf.numChannelsOut(), conf.bufferSize());
    if (!buffer)
    {
        MO_WARNING("Can't create dumb buffer for '" << filename << "'");
        return;
    }

    MO_DEBUG("DumbFile::createRenderer() okay, channels == "
             << duh_sigrenderer_get_n_channels(render));
}


void DumbFile::setPosition(long pos)
{
    if (p_->render && p_->duh)
    {
        QWriteLocker lock(&p_->mutex);

        duh_end_sigrenderer(p_->nextRender);

        p_->render = duh_start_sigrenderer(p_->duh, 0, p_->conf.numChannelsOut(), pos);

        if (!p_->render)
        {
            MO_WARNING("DumbFile::setPosition(): Can't create dumb renderer for '" << p_->filename << "'");
            return;
        }

        p_->lastPos = pos;
    }
}

void DumbFile::setPositionThreadsafe(long pos)
{
    QWriteLocker lock(&p_->mutex);

    setPosition(pos);
}



/* from http://dumb.sourceforge.net/index.php?page=docs&doc=dumb

      n_channels == 1: samples[0][sample_position]
      n_channels == 2: samples[0][sample_position*2+channel_number]
      n_channels > 2:
         channel_number < n_channels & ~1:
            samples[channel_number>>1][sample_position*2+(channel_number&1)]
         channel_number == n_channels & ~1:
            samples[channel_number>>1][sample_position]

   where 0 <= channel_number < n_channels,
     and 0 <= sample_position < length.
*/

void DumbFile::process(const QList<AudioBuffer*>& outs, F32 amp)
{
    if (!isReady())
    {
#ifdef MO_DO_DEBUG
        MO_WARNING("DumbFile::process() but !isReady()");
#endif
        return;
    }

    // clear buffer
    for (uint i=0; i<p_->conf.numChannelsOut(); ++i)
        dumb_silence(p_->buffer[0], p_->conf.bufferSize() * p_->conf.numChannelsOut());

    // render song
    uint ret;
    {
        // lock access to renderer
        QReadLocker lock(&p_->mutex);

        ret = duh_sigrenderer_generate_samples(
                    p_->render,
                    amp,
                    65536.f * p_->conf.sampleRateInv(),
                    p_->conf.bufferSize(),
                    p_->buffer );
    }

    // store the last position
    p_->lastPos = position();

    // copy to float buffer
    // XXX Does not work for >2 channels !!
    for (int j=0; j<outs.size(); ++j)
    if (outs[j])
    {
        for (uint i=0; i<ret; ++i)
            outs[j]->write(i, F32(p_->buffer[0][i * 2 + j]) * p_->conversion);

        // clear rest
        for (uint i=ret; i<p_->conf.bufferSize(); ++i)
            outs[j]->write(i, 0.f);
    }

}


} // namespace AUDIO
} // namespace MO

#endif // #ifndef MO_DISABLE_DUMB
