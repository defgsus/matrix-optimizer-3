/** @file texture.cpp

    @brief Wrapper for OpenGL Texture

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05/20/2012, pulled-in 8/1/2014</p>
*/

#include "texture.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GL {

long int Texture::memory_used_ = 0;

Texture::Texture(ErrorReporting report)
    :   rep_            (report),
        ptr_			(0),
        uploaded_		(false),
        width_			(0),
        height_			(0),
        memory_ 		(0),
        handle_			(invalidGl),
        target_			(GL_NONE),
        format_			(GL_NONE),
        input_format_	(GL_NONE),
        type_			(GL_NONE)
{
    MO_DEBUG_GL("Texture::Texture()");
}

Texture::Texture(GLsizei width, GLsizei height,
                 GLenum format, GLenum input_format,
                 GLenum type, void *ptr_to_data,
                 ErrorReporting report)
    : rep_          (report),
      ptr_			(ptr_to_data),
      uploaded_		(false),
      width_		(width),
      height_		(height),
      memory_ 		(0),
      handle_		(invalidGl),
      target_		(GL_TEXTURE_2D),
      format_		(format),
      input_format_	(input_format),
      type_			(type)
{
    MO_DEBUG_GL("Texture::Texture(" << width << ", " << height
                << ", " << format << ", " << input_format << ", "
                << ", " << type << ", " << ptr_to_data << ")");
}


Texture::~Texture()
{
    MO_DEBUG_GL("Texture::~Texture()");

    if (isCreated())
        MO_GL_WARNING("destructor of bound texture!");
}


bool Texture::create(GLsizei width, GLsizei height,
                GLenum format, GLenum input_format,
                GLenum type,
                void* ptr_to_data)
{
    MO_DEBUG_GL("Texture::create(" << width << ", " << height
                << ", " << format << ", " << input_format << ", "
                << ", " << type << ", " << ptr_to_data << ")");

    releaseTexture_();

    target_ = GL_TEXTURE_2D;
    width_ = width;
    height_ = height;
    handle_ = genTexture_();
    uploaded_ = false;
    format_ = format;
    input_format_ = input_format;
    type_ = type;
    ptr_ = ptr_to_data;

    if (!bind())
        return false;

    return upload_(ptr_, 0);
}



GLuint Texture::genTexture_() const
{
    GLuint tex;

    GLenum err;
    MO_CHECK_GL_RET_COND( rep_, glGenTextures(1, &tex), err );

    return err ? invalidGl : tex;
}

void Texture::releaseTexture_()
{
    if (!isCreated()) return;

    MO_CHECK_GL_COND( rep_, glDeleteTextures(1, &handle_) );

    handle_ = invalidGl;

    if (uploaded_)
        memory_used_ -= memory_;
}



// ---------------------- public interface ------------------------

bool Texture::bind() const
{
    GLint err;
    MO_CHECK_GL_RET_COND( rep_, glBindTexture(target_, handle_), err );
    if (err) return false;

    MO_CHECK_GL_RET_COND( rep_, glEnable(target_), err );
    return !err;
}

void Texture::unbind() const
{
    MO_CHECK_GL_COND( rep_, glDisable(target_) );
}

void Texture::release()
{
    releaseTexture_();
}

void Texture::texParameter(GLenum param, GLenum value) const
{
    MO_CHECK_GL_COND( rep_, glTexParameteri(target_, param, value) );
}

bool Texture::create()
{
    handle_ = genTexture_();
    if (handle_ == invalidGl)
        return false;

    if (!bind())
        return false;

    return upload_(ptr_, 0);
}


bool Texture::upload(GLint mipmap_level)
{
    return upload_(ptr_, mipmap_level);
}

bool Texture::upload(void * ptr, GLint mipmap_level)
{
    return upload_(ptr, mipmap_level);
}

bool Texture::upload_(void * ptr, GLint mipmap_level)
{
    MO_DEBUG_GL("Texture::upload_(" << ptr << ", " << mipmap_level << ")");

    if (!isCreated())
    {
        if (rep_ == ER_THROW)
            MO_GL_ERROR("Texture::upload() on uninitialized Texture");
        return false;
    }

    GLint err;
    switch (target_)
    {
        case GL_TEXTURE_1D:
            MO_CHECK_GL_RET_COND( rep_,
            glTexImage1D(
                target_,
                // mipmap level
                mipmap_level,
                // color components
                format_,
                // size
                width_,
                // boarder
                0,
                // input format and type
                input_format_,
                // data type
                type_,
                // pixel data
                ptr)
            , err);
            if (err) return false;

            // count memory
            if (!uploaded_)
            {
                uploaded_ = true;
                memory_ = width_ * GL::channelSize(format_) * GL::typeSize(type_);
                memory_used_ += memory_;
            }
        break;

        case GL_TEXTURE_2D:

            MO_CHECK_GL_RET_COND( rep_,
            glTexImage2D(
                target_,
                // mipmap level
                mipmap_level,
                // color components
                format_,
                // size
                width_, height_,
                // boarder
                0,
                // input format
                input_format_,
                // data type
                type_,
                // ptr
                ptr);
            , err)
            if (err) return false;

            if (!uploaded_)
            {
                uploaded_ = true;
                memory_ = width_ * GL::channelSize(format_) * GL::typeSize(type_);
                memory_used_ += memory_;
            }
        break;
    }


    texParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    bool repeat = false;
    texParameter(GL_TEXTURE_WRAP_S, (repeat)? GL_REPEAT : GL_CLAMP );
    texParameter(GL_TEXTURE_WRAP_T, (repeat)? GL_REPEAT : GL_CLAMP );

    // bind again to check for errors
    return bind();
}


bool Texture::download(void * ptr) const
{
    MO_DEBUG_GL("Texture::download(" << ptr << ")");

    if (ptr) ptr = ptr_;

    MO_ASSERT(ptr, "download from texture to NULL");

    GLint err;
    MO_CHECK_GL_RET_COND( rep_,
        glGetTexImage(
            target_,
            // mipmap level
            0,
            // color components
            input_format_,
            // format
            type_,
            // data
            ptr)
        , err);

    return (!err);
}

/*
QString Texture::info_str() const
{
    std::stringstream s;
    s 	<< width_ << "x" << height_
        << ", target:" << GL::Enum::name(target_)
        << ", format:" << GL::Enum::name(format_)
        << ", host_format:" << GL::Enum::name(input_format_)
        << ", handle:" << handle_
        ;
    return s.str();
}
*/




} // namespace GL
} // namespace MO
