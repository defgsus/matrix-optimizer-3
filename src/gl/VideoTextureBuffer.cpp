/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#include "VideoTextureBuffer.h"
#include "video/ffm/VideoStream.h"
#include "video/DecoderFrame.h"
#include "gl/Texture.h"
#include "io/log.h"

namespace MO {
namespace GL {

struct VideoTextureBuffer::Private
{
    Private(VideoTextureBuffer*p)
        : p         (p)
        , stream    (nullptr)
    { }

    typedef long Key;

    static Key toKey(double t) { return t * 512; }
    static double fromKey(Key k) { return double(k) / 512; }

    struct Frame
    {
        Frame(double time = 0, GL::Texture* tex = 0)
            : key(toKey(time)), time(time), tex(tex)
        { }
        ~Frame() { if (tex) tex->release(); delete tex; }

        Key key;
        double time;
        GL::Texture* tex;
    };

    GL::Texture* createTexture(DecoderFrame* f);
    Frame* createFrame(DecoderFrame* f);
    void freeGl();

    VideoTextureBuffer* p;
    FFM::VideoStream* stream;

    std::map<Key, Frame*> frames;
};

VideoTextureBuffer::VideoTextureBuffer()
    : p_        (new Private(this))
{

}

VideoTextureBuffer::~VideoTextureBuffer()
{
    delete p_;
}

void VideoTextureBuffer::setStream(FFM::VideoStream* s)
{
    p_->stream = s;
}

void VideoTextureBuffer::releaseGl() { p_->freeGl(); }

void VideoTextureBuffer::Private::freeGl()
{
    for (auto i : frames)
        delete i.second;
    frames.clear();
}

GL::Texture* VideoTextureBuffer::Private::createTexture(DecoderFrame *f)
{
    auto tex = new GL::Texture(f->width(), f->height(), gl::GL_RGBA,
                               gl::GL_RED, gl::GL_UNSIGNED_BYTE, nullptr);
    try
    {
        tex->create();
        tex->upload(f->planeY());
    }
    catch (...)
    {
        tex->release();
        delete tex;
        throw;
    }
    return tex;
}

VideoTextureBuffer::Private::Frame* VideoTextureBuffer::Private::createFrame(DecoderFrame *f)
{
    auto tex = createTexture(f);
    auto frame = new Frame(f->presentationTime(), tex);
    return frame;
}

GL::Texture* VideoTextureBuffer::getTexture(double time)
{
    if (!p_->stream || !p_->stream->isReady())
        return nullptr;

    p_->stream->seekSecond(time);
    auto f = p_->stream->getVideoFrame();

    if (!f)
        return nullptr;
    return p_->createTexture(f);
}


} // namespace GL
} // namespace MO
