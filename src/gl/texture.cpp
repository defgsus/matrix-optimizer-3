/** @file texture.cpp

    @brief Wrapper for OpenGL Texture

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05/20/2012, pulled-in 8/1/2014</p>
*/

#include <atomic>

#include <QImage>

#include "texture.h"
#include "img/image.h"
#include "io/streamoperators_glbinding.h"
#include "io/error.h"
#include "io/log.h"

using namespace gl;

namespace MO {
namespace GL {

long int Texture::memory_used_ = 0;

namespace { static std::atomic_uint_fast64_t tex_count_(0); }

Texture::Texture(ErrorReporting report)
    :   rep_            (report),
        ptr_			(0),
        ptr_px_         (0),
        ptr_nx_         (0),
        ptr_py_         (0),
        ptr_ny_         (0),
        ptr_pz_         (0),
        ptr_nz_         (0),
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
    MO_DEBUG_IMG("Texture::Texture()");
    name_ = QString("tex%1").arg(tex_count_++);
}

Texture::Texture(gl::GLsizei width, gl::GLsizei height,
                 gl::GLenum format, gl::GLenum input_format,
                 gl::GLenum type, void *ptr_to_data,
                 ErrorReporting report)
    : rep_          (report),
      ptr_			(ptr_to_data),
      ptr_px_         (0),
      ptr_nx_         (0),
      ptr_py_         (0),
      ptr_ny_         (0),
      ptr_pz_         (0),
      ptr_nz_         (0),
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
    MO_DEBUG_IMG("Texture::Texture(" << width << ", " << height
                << ", " << format << ", " << input_format
                << ", " << type << ", " << ptr_to_data << ")");
    name_ = QString("tex%1").arg(tex_count_++);
}

Texture::Texture(gl::GLsizei width, gl::GLsizei height,
                 gl::GLenum format, gl::GLenum input_format,
                 gl::GLenum type,
                 void * ptr_px, void * ptr_nx,
                 void * ptr_py, void * ptr_ny,
                 void * ptr_pz, void * ptr_nz,
                 ErrorReporting report)
    : rep_          (report),
      ptr_			(0),
      ptr_px_       (ptr_px),
      ptr_nx_       (ptr_nx),
      ptr_py_       (ptr_py),
      ptr_ny_       (ptr_ny),
      ptr_pz_       (ptr_pz),
      ptr_nz_       (ptr_nz),
      uploaded_		(false),
      width_		(width),
      height_		(height),
      memory_ 		(0),
      handle_		(invalidGl),
      target_		(GL_TEXTURE_CUBE_MAP),
      format_		(format),
      input_format_	(input_format),
      type_			(type)
{
    MO_DEBUG_IMG("Texture::Texture(" << width << ", " << height
                << ", " << format << ", " << input_format
                << ", " << type << ", " << ptr_px << ", " << ptr_nx << ", "
                << ptr_py << ", " << ptr_ny << ", " << ptr_pz << ", " << ptr_nz << ")");
    name_ = QString("tex%1").arg(tex_count_++);
}

Texture::~Texture()
{
    MO_DEBUG_IMG("Texture::~Texture()");

    if (isCreated())
        MO_GL_WARNING("destructor of bound texture!");
}

Texture * Texture::constructFrom(const Texture * t)
{
    Texture * c;
    if (t->isCube())
        c = new Texture(t->width(), t->height(),
                           t->format(), t->input_format_,
                           t->type(), 0,0,0,0,0,0, t->rep_);
    else
        c = new Texture(t->width(), t->height(),
                           t->format(), t->input_format_,
                           t->type(), 0, t->rep_);

    c->name_ += "_from_" + t->name();
    return c;
}


bool Texture::isCube() const
{
     return target_ == GL_TEXTURE_CUBE_MAP;
}

bool Texture::create(gl::GLsizei width,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void* ptr_to_data)
{
    MO_DEBUG_IMG("Texture::create(" << width
                << ", " << format << ", " << input_format
                << ", " << type << ", " << ptr_to_data << ")");

    releaseTexture_();

    target_ = GL_TEXTURE_1D;
    width_ = width;
    height_ = 0;
    handle_ = genTexture_();
    uploaded_ = false;
    format_ = format;
    input_format_ = input_format;
    type_ = type;
    ptr_ = ptr_to_data;
    ptr_px_ =
    ptr_nx_ =
    ptr_py_ =
    ptr_ny_ =
    ptr_pz_ =
    ptr_nz_ = 0;

    if (!bind())
    {
        MO_GL_WARNING("Could not bind 1D-texture for creation");
        return false;
    }

    return upload_(ptr_, 0);
}

bool Texture::create(gl::GLsizei width, gl::GLsizei height,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void* ptr_to_data)
{
    MO_DEBUG_IMG("Texture::create(" << width << ", " << height
                << ", " << format << ", " << input_format
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
    ptr_px_ =
    ptr_nx_ =
    ptr_py_ =
    ptr_ny_ =
    ptr_pz_ =
    ptr_nz_ = 0;

    if (!bind())
    {
        MO_GL_WARNING("Could not bind 2D-texture for creation");
        return false;
    }

    return upload_(ptr_, 0);
}


bool Texture::create(gl::GLsizei width, gl::GLsizei height,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void * ptr_px, void * ptr_nx,
                void * ptr_py, void * ptr_ny,
                void * ptr_pz, void * ptr_nz)
{
    MO_DEBUG_IMG("Texture::create(" << width << ", " << height
                << ", " << format << ", " << input_format
                << ", " << type << ", " << ptr_px << ", " << ptr_nx << ", "
                << ptr_py << ", " << ptr_ny << ", " << ptr_pz << ", " << ptr_nz << ")");

    releaseTexture_();

    target_ = GL_TEXTURE_CUBE_MAP;
    width_ = width;
    height_ = height;
    handle_ = genTexture_();
    uploaded_ = false;
    format_ = format;
    input_format_ = input_format;
    type_ = type;
    ptr_ = 0;
    ptr_px_ = ptr_px;
    ptr_py_ = ptr_py;
    ptr_pz_ = ptr_pz;
    ptr_nx_ = ptr_nx;
    ptr_ny_ = ptr_ny;
    ptr_nz_ = ptr_nz;

    if (!bind())
        return false;

    return upload_(ptr_, 0);
}



GLuint Texture::genTexture_() const
{
    MO_DEBUG_IMG("Texture::genTexture_()");

    GLuint tex;

    GLenum err;
    MO_CHECK_GL_RET_COND( rep_, glGenTextures(1, &tex), err );

    return err == GL_NO_ERROR ? tex : invalidGl;
}

void Texture::releaseTexture_()
{
    MO_DEBUG_IMG("Texture::releaseTexture_()");

    if (!isCreated()) return;

    MO_CHECK_GL_COND( rep_, glDeleteTextures(1, &handle_) );

    handle_ = invalidGl;

    if (uploaded_)
        memory_used_ -= memory_;

    uploaded_ = false;
}



// ---------------------- public interface ------------------------

bool Texture::bind() const
{
    //MO_DEBUG_IMG("Texture::bind(" << target_ << ", " << handle_ << ")");

    GLenum err;
    MO_CHECK_GL_RET_COND( rep_, glBindTexture(target_, handle_), err );
    if (err != GL_NO_ERROR)
        return false;

    return err == GL_NO_ERROR;
}


void Texture::release()
{
    releaseTexture_();
}

void Texture::setTexParameter(GLenum param, GLint value) const
{
    MO_CHECK_GL_COND( rep_, glTexParameteri(target_, param, value) );
}

bool Texture::create()
{
    MO_DEBUG_IMG("Texture::create()");

    // delete previous
    if (handle_ != invalidGl)
        releaseTexture_();

    handle_ = genTexture_();
    if (handle_ == invalidGl)
    {
        MO_GL_WARNING("genTexture_() failed");
        return false;
    }

    if (!bind())
    {
        MO_GL_WARNING("bind of texture before creation failed");
        return false;
    }

    if (target_ != GL_TEXTURE_CUBE_MAP)
        return upload_(ptr_, 0);
    else
    {
        if (!upload_(ptr_px_, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X)) return false;
        if (!upload_(ptr_nx_, 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X)) return false;
        if (!upload_(ptr_py_, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y)) return false;
        if (!upload_(ptr_ny_, 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)) return false;
        if (!upload_(ptr_pz_, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z)) return false;
        return upload_(ptr_nz_, 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    }
}


bool Texture::upload(gl::GLint mipmap_level)
{
    MO_DEBUG_IMG("Texture::upload(mipmap=" << mipmap_level << ")");

    if (target_ != GL_TEXTURE_CUBE_MAP)
        return upload_(ptr_, mipmap_level);
    else
    {
        if (!upload_(ptr_px_, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_X)) return false;
        if (!upload_(ptr_nx_, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_X)) return false;
        if (!upload_(ptr_py_, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_Y)) return false;
        if (!upload_(ptr_ny_, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)) return false;
        if (!upload_(ptr_pz_, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_Z)) return false;
        return upload_(ptr_nz_, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    }
}

bool Texture::upload(void * ptr, gl::GLint mipmap_level)
{
    MO_DEBUG_IMG("Texture::upload(" << ptr << ", mipmap=" << mipmap_level << ")");

    if (target_ != GL_TEXTURE_CUBE_MAP)
        return upload_(ptr, mipmap_level);
    else
    {
        if (!upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_X)) return false;
        if (!upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_X)) return false;
        if (!upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_Y)) return false;
        if (!upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)) return false;
        if (!upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_Z)) return false;
        return upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    };
}

bool Texture::upload_(const void * ptr, GLint mipmap_level, GLenum cube_target)
{
    MO_DEBUG_IMG("Texture::upload_(" << ptr << ", mipmap=" << mipmap_level
                << ", cubetgt=" << cube_target << ")");

    if (!isCreated())
    {
        if (rep_ == ER_THROW)
            MO_GL_ERROR("Texture::upload() on uninitialized Texture");
        return false;
    }

    GLenum err;
    switch (target_)
    {
        case GL_TEXTURE_1D:
            MO_CHECK_GL_RET_COND( rep_,
            gl::glTexImage1D(
                target_,
                // mipmap level
                mipmap_level,
                // color components
                GLint(format_),
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
            if (err != GL_NO_ERROR) return false;

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
            gl::glTexImage2D(
                target_,
                // mipmap level
                mipmap_level,
                // color components
                GLint(format_),
                // size
                width_, height_,
                // boarder
                0,
                // input format
                input_format_,
                // data type
                type_,
                // ptr
                ptr)
            , err);

            if (err != GL_NO_ERROR)
            {
                MO_GL_WARNING("Texture::upload_() failed");
                return false;
            }

            if (!uploaded_)
            {
                uploaded_ = true;
                memory_ = width_ * GL::channelSize(format_) * GL::typeSize(type_);
                memory_used_ += memory_;
            }

        break;

        case GL_TEXTURE_CUBE_MAP:

            MO_CHECK_GL_RET_COND( rep_,
            gl::glTexImage2D(
                cube_target,
                // mipmap level
                mipmap_level,
                // color components
                GLint(format_),
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
            if (err != GL_NO_ERROR) return false;

            // XXX fix memory count for cubemap
            if (!uploaded_)
            {
                uploaded_ = true;
                memory_ = width_ * GL::channelSize(format_) * GL::typeSize(type_);
                memory_used_ += memory_;
            }
        break;

        default:
            MO_GL_WARNING("Unhandled texture target " << (int)target_ << " in Texture::create_()");
        break;
    }

    setTexParameter(GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
    setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));

    // XXX needs to be true for TEXTURE_CUBE_MAP
    bool repeat = true;
    setTexParameter(GL_TEXTURE_WRAP_S, GLint( (repeat)? GL_REPEAT : GL_CLAMP) );
    setTexParameter(GL_TEXTURE_WRAP_T, GLint( (repeat)? GL_REPEAT : GL_CLAMP) );

    MO_DEBUG_IMG("Texture::upload_() finished");

    // bind again to check for errors
    return bind();
}


bool Texture::download(void * ptr, GLuint mipmap) const
{
    MO_DEBUG_IMG("Texture::download(" << ptr << ")");

    MO_ASSERT(ptr && target_ != GL_TEXTURE_CUBE_MAP,
              "download of cubemap to single goal undefined");

    if (target_ != GL_TEXTURE_CUBE_MAP)
    {
        if (ptr) ptr = ptr_;
        return download_(ptr, mipmap, target_, type_);
    }

    if (!download_(ptr_px_, mipmap, GL_TEXTURE_CUBE_MAP_POSITIVE_X, type_)) return false;
    if (!download_(ptr_nx_, mipmap, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, type_)) return false;
    if (!download_(ptr_py_, mipmap, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, type_)) return false;
    if (!download_(ptr_ny_, mipmap, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, type_)) return false;
    if (!download_(ptr_pz_, mipmap, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, type_)) return false;
    return download_(ptr_nz_, mipmap, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, type_);
}

bool Texture::download_(void * ptr, GLuint mipmap, GLenum target, GLenum type) const
{
    MO_DEBUG_IMG("Texture::download_(" << ptr << ", " << mipmap
                 << ", " << target << ", " << type << ")");

    MO_ASSERT(ptr, "download from texture to NULL");

    GLenum err;
    MO_CHECK_GL_RET_COND( rep_,
        glGetTexImage(
            target,
            // mipmap level
            mipmap,
            // color components
            input_format_,
            // format
            type,
            // data
            ptr)
        , err);

    return (err != GL_NO_ERROR);
}

bool Texture::download(void * ptr, GLenum format, GLenum type, GLuint mipmap) const
{
    MO_DEBUG_IMG("Texture::download(" << ptr << ", " << format
                 << ", " << type << ", " << mipmap << ")");

    MO_ASSERT(ptr, "download from texture to NULL");
    MO_ASSERT(target_ != GL_TEXTURE_CUBE_MAP, "download of cubemap to single goal undefined");

    GLenum err;
    MO_CHECK_GL_RET_COND( rep_,
        glGetTexImage(
            target_,
            // mipmap level
            mipmap,
            // color components
            format,
            // format
            type,
            // data
            ptr)
        , err);

    return (err != GL_NO_ERROR);
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


QImage Texture::getImage() const
{
    MO_DEBUG_IMG("Texture::getImage()");

    std::vector<GLfloat> buffer(width() * height() * 4);
    //float * buffer = (float*) aligned_alloc(32, width() * height() * 4);

    GLenum err;
    MO_CHECK_GL_RET_COND( rep_,
        glGetTexImage(
            target_,
            // mipmap level
            0,
            // color components
            GL_RGBA,
            // format
            GL_FLOAT,
            // data
            &buffer[0])
        , err);

    if (err != GL_NO_ERROR)
        return QImage();

    QImage img(width(), height(), QImage::Format_RGB32);

    for (uint y=0; y<height(); ++y)
    for (uint x=0; x<width(); ++x)
    {
        const float * pix = &buffer[((height()-1-y)*width()+x)*4];
        img.setPixel(x, y, qRgb(
                         255 * std::max(0.f, std::min(1.f, pix[0] )),
                         255 * std::max(0.f, std::min(1.f, pix[1] )),
                         255 * std::max(0.f, std::min(1.f, pix[2] ))
                         ));
    }

    //free(buffer);

    return img;
}



// ------------------------ static ----------------------------


Texture * Texture::createFromImage(const Image & img, gl::GLenum gpu_format, ErrorReporting rep)
{
    MO_DEBUG_IMG("Texture::createFromImage(" << img << ", " << gpu_format << ")");

    if (img.isEmpty())
    {
        MO_GL_ERROR_COND(rep, "createFromImage() with NULL image");
        return 0;
    }

    if (img.width() == 0 || img.height() == 0 )
    {
        MO_GL_ERROR_COND(rep, "createFromImage() with empty image");
        return 0;
    }

    // determine texture format from image format

    GLenum format = img.glEnumForFormat();

    // create and bind

    Texture * tex = new Texture(
                img.width(), img.height(),
                gpu_format, format, img.glEnumForType(), 0, rep);

    tex->name_ += "_from_img";

    if (!tex->create())
    {
        delete tex;
        return 0;
    }

    // upload image data

    try
    {
        tex->upload_(img.data(), 0);
    }
    catch (Exception & e)
    {
        delete tex;

        if (rep == ER_THROW)
        {
            e << "\non creating texture from image " << img;
            throw;
        }
        else
            return 0;
    }

    return tex;
}


Texture * Texture::createFromImage(const QImage & input_img, gl::GLenum gpu_format, ErrorReporting rep)
{
    MO_DEBUG_IMG("Texture::createFromImage(QImage&, " << gpu_format << ")");

    Image img;
    if (!img.createFrom(input_img))
    {
        MO_GL_ERROR_COND(rep, "could not convert QImage to Image");
        return 0;
    }

    return createFromImage(img, gpu_format, rep);
}


} // namespace GL
} // namespace MO
