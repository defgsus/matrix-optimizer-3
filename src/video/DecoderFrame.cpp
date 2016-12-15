
#include "DecoderFrame.h"
#include "gl/Texture.h"

void DecoderFrame::convertToYUV()
{
    p_converted_.resize(width()*height()*3);

    for (size_t i=0; i<planeYSize(); ++i)
        p_converted_[i*3] = planeY()[i];
    for (size_t i=0; i<planeYSize(); ++i)
    {
        size_t x = i % width(),
               y = i / width();
        const uint8_t *uv = &planeUV()[(y/2*width()/2+x/2)*2];
        p_converted_[i*3+1] = uv[0];
        p_converted_[i*3+2] = uv[1];
    }
}


MO::GL::Texture* DecoderFrame::createTextureY() const
{
    auto tex = new MO::GL::Texture(
                width(), height(),
                gl::GL_R8,
                gl::GL_RED, gl::GL_UNSIGNED_BYTE, nullptr);

    p_upload_(tex, planeY());
    return tex;
}

MO::GL::Texture* DecoderFrame::createTextureYUV()
{

    auto tex = new MO::GL::Texture(
                width(), height(),
                gl::GL_RGB,
                gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr);

    if (!isConverted())
        convertToYUV();
    p_upload_(tex, converted());
    return tex;
}

void DecoderFrame::p_upload_(MO::GL::Texture* tex, const void* data) const
{
    try
    {
        tex->create();
        tex->upload((void*)data);
    }
    catch (...)
    {
        tex->release();
        delete tex;
        throw;
    }
}

