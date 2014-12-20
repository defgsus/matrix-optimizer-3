/** @file dumpfile.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.12.2014</p>
*/

#include <dumb.h>

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
          buffer(0)
    { }

    void createRenderer();

    static bool static_init;
    static const F32 conversion;

    DumbFile * dumb;

    AUDIO::Configuration conf;

    QString filename;

    DUMBFILE * file;
    DUH * duh;
    DUH_SIGRENDERER * render;
    sample_t ** buffer;
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

    // initialize dumb
    if (!p_->static_init)
    {
        MO_DEBUG("DumbFile: initializing dump filesystem");

        p_->static_init = true;
        dumb_register_stdfiles();
    }

    const char * fn = filename.toStdString().c_str();

    DUH * duh = load_duh(fn);
    if (!duh)
    {
        duh = dumb_load_it(fn);
        if (!duh)
        {
            duh = dumb_load_xm(fn);
            if (!duh)
            {
                duh = dumb_load_s3m(fn);
                if (!duh)
                {
                    duh = dumb_load_mod(fn);
                    if (!duh)
                    {
                        MO_IO_ERROR(READ, "Can't open dumbfile '" << filename << "'");
                    }
                }
            }
        }
    }

    close();

    p_->duh = duh;
    p_->filename = filename;

    if (p_->conf.isValid())
        p_->createRenderer();
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

    render = duh_start_sigrenderer(duh, 0, conf.numChannelsOut(), 0);
    if (!render)
    {
        MO_WARNING("Can't create dumb renderer for '" << filename << "'");
        return;
    }

    buffer = allocate_sample_buffer(conf.numChannelsOut(), conf.bufferSize());
    if (!buffer)
    {
        MO_WARNING("Can't create dumb buffer for '" << filename << "'");
        return;
    }

    MO_DEBUG("DumbFile::createRenderer() okay, channels == "
             << duh_sigrenderer_get_n_channels(render));
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
        MO_WARNING("DumbFile::process() but !isReady()");
        return;
    }

    // clear buffer
    for (uint i=0; i<p_->conf.numChannelsOut(); ++i)
        dumb_silence(p_->buffer[0], p_->conf.bufferSize() * p_->conf.numChannelsOut());

    // render song
    uint ret =
        duh_sigrenderer_generate_samples(
                p_->render,
                amp,
                65536.f * p_->conf.sampleRateInv(),
                p_->conf.bufferSize(),
                p_->buffer );

    // copy to float buffer
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

