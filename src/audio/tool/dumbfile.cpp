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


namespace MO {
namespace AUDIO {


class DumbFile::Private
{
public:
    Private(DumbFile * dumb)
        : dumb  (dumb),
          file  (0),
          duh   (0),
          render(0),
          conf  ()
    { }

    static bool static_init;

    DumbFile * dumb;

    DUMBFILE * file;
    DUH * duh;
    DUH_SIGRENDERER * render;

    AUDIO::Configuration conf;

    sample_t ** buffer;
};

bool DumbFile::Private::static_init = false;


DumbFile::DumbFile()
    : p_    (new Private(this))
{
    if (!p_->static_init)
    {
        p_->static_init = true;
        dumb_register_stdfiles();
    }
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


bool DumbFile::isReady() const
{
    return p_->render;
}

const AUDIO::Configuration& DumbFile::config() const
{
    return p_->conf;
}


void DumbFile::open(const QString &filename)
{
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

    p_->render = duh_start_sigrenderer(duh, 0, p_->conf.numChannelsOut(), 0);
    if (!p_->render)
    {
        close();
        MO_ERROR("Cant' create dumb renderer for '" << filename << "'");
    }
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
}

long DumbFile::position() const
{
    return p_->render ? duh_sigrenderer_get_position(p_->render) : 0;
}


void DumbFile::setConfig(const AUDIO::Configuration& c)
{
    p_->conf = c;

    allocate_sample_buffer(p_->conf.numChannelsOut(), p_->conf.bufferSize());
}


void DumbFile::process(const QList<AudioBuffer*>& outs)
{
    if (!isReady())
        return;

    // clear buffer
    for (uint i=0; i<p_->conf.numChannelsOut(); ++i)
        dumb_silence(p_->buffer[0], p_->conf.bufferSize());

    // render song
    uint ret =
        duh_sigrenderer_generate_samples(
                p_->render,
                1.0,
                65536.f * p_->conf.sampleRateInv(),
                p_->conf.bufferSize(),
                p_->buffer );

    // copy to float buffer
    for (int j=0; j<outs.size(); ++j)
    if (outs[j])
    {
        for (uint i=0; i<ret; ++i)
            outs[j]->write(i, F32(p_->buffer[j][i]) / 0x8000);

        // clear rest
        for (uint i=ret; i<p_->conf.bufferSize(); ++i)
            outs[j]->write(i, 0.f);
    }

}


} // namespace AUDIO
} // namespace MO

