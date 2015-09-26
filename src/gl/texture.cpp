/** @file texture.cpp

    @brief Wrapper for OpenGL Texture

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05/20/2012, pulled-in 8/1/2014</p>
*/

#include <atomic>
#include <memory>

#include <QImage>

#include "texture.h"
#include "io/imagereader.h"
#include "io/streamoperators_glbinding.h"
#include "io/error.h"
#include "io/log.h"

using namespace gl;

namespace MO {
namespace GL {

long int Texture::memory_used_ = 0;

namespace { static std::atomic_uint_fast64_t tex_count_(0); }

Texture::Texture()
    :   ptr_			(0),
        ptr_px_         (0),
        ptr_nx_         (0),
        ptr_py_         (0),
        ptr_ny_         (0),
        ptr_pz_         (0),
        ptr_nz_         (0),
        uploaded_		(false),
        width_			(0),
        height_			(0),
        depth_          (0),
        multiSamples_   (0),
        memory_ 		(0),
        handle_			(invalidGl),
        target_			(GL_NONE),
        format_			(GL_NONE),
        input_format_	(GL_NONE),
        type_			(GL_NONE),
        hash_           (-1)
{
    MO_DEBUG_IMG("Texture::Texture()");
    name_ = QString("tex%1").arg(tex_count_++);
}


Texture::Texture(gl::GLsizei width, gl::GLsizei height,
                 gl::GLenum format, gl::GLenum input_format,
                 gl::GLenum type, void *ptr_to_data,
                 gl::GLsizei multiSamples)
    : ptr_			(ptr_to_data),
      ptr_px_         (0),
      ptr_nx_         (0),
      ptr_py_         (0),
      ptr_ny_         (0),
      ptr_pz_         (0),
      ptr_nz_         (0),
      uploaded_		(false),
      width_		(width),
      height_         (height),
      depth_          (0),
      multiSamples_ (multiSamples),
      memory_ 		(0),
      handle_		(invalidGl),
      target_		(multiSamples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D),
      format_		(format),
      input_format_	(input_format),
      type_			(type),
      hash_           (-1)
{
    MO_DEBUG_IMG("Texture::Texture(" << width << "x" << height
                << ", " << format << ", " << input_format
                << ", " << type << ", " << ptr_to_data << ")");
    name_ = QString("tex%1").arg(tex_count_++);
}

Texture::Texture(gl::GLsizei width, gl::GLsizei height, gl::GLsizei depth,
                 gl::GLenum format, gl::GLenum input_format,
                 gl::GLenum type, void *ptr_to_data)
    : ptr_			(ptr_to_data),
      ptr_px_         (0),
      ptr_nx_         (0),
      ptr_py_         (0),
      ptr_ny_         (0),
      ptr_pz_         (0),
      ptr_nz_         (0),
      uploaded_		(false),
      width_		(width),
      height_		(height),
      depth_        (depth),
      multiSamples_ (0),
      memory_ 		(0),
      handle_		(invalidGl),
      target_		(GL_TEXTURE_3D),
      format_		(format),
      input_format_	(input_format),
      type_			(type),
      hash_           (-1)
{
    MO_DEBUG_IMG("Texture::Texture(" << width << "x" << height << "x" << depth
                << ", " << format << ", " << input_format
                << ", " << type << ", " << ptr_to_data << ")");
    name_ = QString("tex%1").arg(tex_count_++);
}

Texture::Texture(gl::GLsizei width, gl::GLsizei height,
                 gl::GLenum format, gl::GLenum input_format,
                 gl::GLenum type,
                 void * ptr_px, void * ptr_nx,
                 void * ptr_py, void * ptr_ny,
                 void * ptr_pz, void * ptr_nz)
    : ptr_			(0),
      ptr_px_       (ptr_px),
      ptr_nx_       (ptr_nx),
      ptr_py_       (ptr_py),
      ptr_ny_       (ptr_ny),
      ptr_pz_       (ptr_pz),
      ptr_nz_       (ptr_nz),
      uploaded_		(false),
      width_		(width),
      height_		(height),
      depth_        (0),
      multiSamples_ (0),
      memory_ 		(0),
      handle_		(invalidGl),
      target_		(GL_TEXTURE_CUBE_MAP),
      format_		(format),
      input_format_	(input_format),
      type_			(type),
      hash_           (-1)
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

    if (isHandle())
        MO_GL_WARNING("destructor of bound texture '" << name() << "'");
}

Texture * Texture::constructFrom(const Texture * t)
{
    Texture * c;
    if (t->isCube())
        c = new Texture(t->width(), t->height(),
                           t->format(), t->input_format_,
                           t->type(), 0,0,0,0,0,0);
    else if (t->is3d())
        c = new Texture(t->width(), t->height(), t->depth(),
                           t->format(), t->input_format_,
                           t->type(), 0);
    else
        c = new Texture(t->width(), t->height(),
                           t->format(), t->input_format_,
                           t->type(), 0, t->isMultiSample());

    c->name_ += t->name() + "(copy)";
    return c;
}


bool Texture::isCube() const
{
     return target_ == GL_TEXTURE_CUBE_MAP;
}

bool Texture::is3d() const
{
    return target_ == gl::GL_TEXTURE_3D;
}

bool Texture::isMultiSample() const
{
    return target_ == gl::GL_TEXTURE_2D_MULTISAMPLE;
}

void Texture::create(gl::GLsizei width,
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
    depth_ = 0;
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

    MO_EXTEND_EXCEPTION(bind(), "Could not bind 1D-texture for creation");

    upload_(ptr_, 0);
}


void Texture::create(gl::GLsizei width, gl::GLsizei height,
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
    depth_ = 0;
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

    MO_EXTEND_EXCEPTION(bind(), "Could not bind 2D-texture for creation");

    upload_(ptr_, 0);
}

void Texture::create(gl::GLsizei width, gl::GLsizei height, gl::GLsizei depth,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void* ptr_to_data)
{
    MO_DEBUG_IMG("Texture::create(" << width << "x" << height << "x" << depth
                << ", " << format << ", " << input_format
                << ", " << type << ", " << ptr_to_data << ")");

    releaseTexture_();

    target_ = GL_TEXTURE_3D;
    width_ = width;
    height_ = height;
    depth_ = depth;
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

    MO_EXTEND_EXCEPTION(bind(), "Could not bind 3D-texture for creation");

    upload_(ptr_, 0);
}


void Texture::create(gl::GLsizei width, gl::GLsizei height,
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
    depth_ = 0;
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

    MO_EXTEND_EXCEPTION(bind(), "Could not bind texture for creation");

    upload_(ptr_, 0);
}



GLuint Texture::genTexture_() const
{
    MO_DEBUG_IMG("Texture::genTexture_()");

    GLuint tex;

    MO_CHECK_GL_THROW( glGenTextures(1, &tex) );

    return tex;
}

void Texture::releaseTexture_()
{
    MO_DEBUG_IMG("Texture::releaseTexture_()");

    if (!isHandle()) return;

    MO_CHECK_GL_THROW( glDeleteTextures(1, &handle_) );

    handle_ = invalidGl;

    if (uploaded_)
        memory_used_ -= memory_;

    uploaded_ = false;
}



// ---------------------- public interface ------------------------

void Texture::bind() const
{
    //MO_DEBUG_IMG("Texture::bind(" << target_ << ", " << handle_ << ")");

    MO_CHECK_GL_THROW( glBindTexture(target_, handle_) );
}


void Texture::release()
{
    releaseTexture_();
}

void Texture::setTexParameter(GLenum param, GLint value) const
{
    MO_CHECK_GL_THROW_TEXT( glTexParameteri(target_, param, value),
                            "target_=" << target_
                            << ", param=" << param
                            << ", value=" << value);
}

void Texture::create()
{
    MO_DEBUG_IMG("Texture::create()");

    // delete previous
    if (handle_ != invalidGl)
        releaseTexture_();

    handle_ = genTexture_();
    bind();

    if (target_ != GL_TEXTURE_CUBE_MAP)
        upload_(ptr_, 0);
    else
    {
        upload_(ptr_px_, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
        upload_(ptr_nx_, 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
        upload_(ptr_py_, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
        upload_(ptr_ny_, 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
        upload_(ptr_pz_, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
        upload_(ptr_nz_, 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    }
}


void Texture::upload(gl::GLint mipmap_level)
{
    MO_DEBUG_IMG("Texture::upload(mipmap=" << mipmap_level << ")");

    if (target_ != GL_TEXTURE_CUBE_MAP)
        upload_(ptr_, mipmap_level);
    else
    {
        upload_(ptr_px_, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
        upload_(ptr_nx_, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
        upload_(ptr_py_, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
        upload_(ptr_ny_, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
        upload_(ptr_pz_, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
        upload_(ptr_nz_, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    }
}

void Texture::upload(void * ptr, gl::GLint mipmap_level)
{
    MO_DEBUG_IMG("Texture::upload(" << ptr << ", mipmap=" << mipmap_level << ")");

    if (target_ != GL_TEXTURE_CUBE_MAP)
        upload_(ptr, mipmap_level);
    else
    {
        upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
        upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
        upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
        upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
        upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
        upload_(ptr, mipmap_level, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    };
}

void Texture::upload_(const void * ptr, GLint mipmap_level, GLenum cube_target)
{
    MO_DEBUG_IMG("Texture::upload_(" << ptr << ", mipmap=" << mipmap_level
                << ", cubetgt=" << cube_target << ")");

    if (!isHandle())
        MO_GL_ERROR("Texture::upload() on uninitialized Texture");

    switch (target_)
    {
        case GL_TEXTURE_1D:

            MO_CHECK_GL_THROW(
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
                ptr));

            // count memory
            if (!uploaded_)
            {
                uploaded_ = true;
                memory_ = width_ * GL::channelSize(format_) * GL::typeSize(type_);
                memory_used_ += memory_;
            }
        break;


        case GL_TEXTURE_2D:

            MO_CHECK_GL_THROW(
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
                ptr));

            if (!uploaded_)
            {
                uploaded_ = true;
                memory_ = width_ * height_ * GL::channelSize(format_) * GL::typeSize(type_);
                memory_used_ += memory_;
            }

        break;

        case GL_TEXTURE_2D_MULTISAMPLE:

            MO_CHECK_GL_THROW(
            gl::glTexImage2DMultisample(
                target_,
                // samples
                multiSamples_,
                // color components
                format_,
                // size
                width_, height_,
                // fixed-samples?
                GL_TRUE));

            if (!uploaded_)
            {
                uploaded_ = true;
                memory_ = width_ * height_ * GL::channelSize(format_) * GL::typeSize(type_)
                                * 4 * 4;
                memory_used_ += memory_;
            }

        break;

        case GL_TEXTURE_3D:

            MO_CHECK_GL_THROW(
            gl::glTexImage3D(
                target_,
                // mipmap level
                mipmap_level,
                // color components
                GLint(format_),
                // size
                width_, height_, depth_,
                // boarder
                0,
                // input format
                input_format_,
                // data type
                type_,
                // ptr
                ptr));

            if (!uploaded_)
            {
                uploaded_ = true;
                memory_ = width_ * height_ * depth_
                        * GL::channelSize(format_) * GL::typeSize(type_);
                memory_used_ += memory_;
            }

        break;

        case GL_TEXTURE_CUBE_MAP:

            MO_CHECK_GL_THROW(
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
                ptr));

            /** @todo fix memory count for cubemap */
            if (!uploaded_)
            {
                uploaded_ = true;
                memory_ = width_ * GL::channelSize(format_) * GL::typeSize(type_);
                memory_used_ += memory_;
            }
        break;

        default:
            MO_GL_ERROR("Unhandled texture target " << (int)target_ << " in Texture::create_()");
        break;
    }

    // those parameters are not applicable to multisample textures
    if (!isMultiSample())
    {
        setTexParameter(GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
        setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));

        // XXX needs to be true for TEXTURE_CUBE_MAP
        bool repeat = true;
        setTexParameter(GL_TEXTURE_WRAP_S, GLint( (repeat)? GL_REPEAT : GL_CLAMP) );
        setTexParameter(GL_TEXTURE_WRAP_T, GLint( (repeat)? GL_REPEAT : GL_CLAMP) );
    }

    MO_DEBUG_IMG("Texture::upload_() finished");

    // bind again to check for errors
    bind();
}


void Texture::download(void * ptr, GLuint mipmap) const
{
    MO_DEBUG_IMG("Texture::download(" << ptr << ")");

    MO_ASSERT(ptr && target_ != GL_TEXTURE_CUBE_MAP,
              "download of cubemap to single goal undefined");

    if (target_ != GL_TEXTURE_CUBE_MAP)
    {
        if (ptr) ptr = ptr_;
        download_(ptr, mipmap, target_, type_);
    }

    download_(ptr_px_, mipmap, GL_TEXTURE_CUBE_MAP_POSITIVE_X, type_);
    download_(ptr_nx_, mipmap, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, type_);
    download_(ptr_py_, mipmap, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, type_);
    download_(ptr_ny_, mipmap, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, type_);
    download_(ptr_pz_, mipmap, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, type_);
    download_(ptr_nz_, mipmap, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, type_);
}

void Texture::download_(void * ptr, GLuint mipmap, GLenum target, GLenum type) const
{
    MO_DEBUG_IMG("Texture::download_(" << ptr << ", " << mipmap
                 << ", " << target << ", " << type << ")");

    MO_ASSERT(ptr, "download from texture to NULL");

    MO_CHECK_GL_THROW(
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
        );
}

void Texture::download(void * ptr, GLenum format, GLenum type, GLuint mipmap) const
{
    MO_DEBUG_IMG("Texture::download(" << ptr << ", " << format
                 << ", " << type << ", " << mipmap << ")");

    MO_ASSERT(ptr, "download from texture to NULL");
    MO_ASSERT(target_ != GL_TEXTURE_CUBE_MAP, "download of cubemap to single goal undefined");

    MO_CHECK_GL_THROW(
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
    );
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


QImage Texture::toQImage() const
{
    MO_DEBUG_IMG("Texture::getImage()");

    std::vector<GLfloat> buffer(width() * height() * 4);
    //float * buffer = (float*) aligned_alloc(32, width() * height() * 4);

    MO_CHECK_GL_THROW(
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
        );

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

    return img;
}



// ------------------------ static ----------------------------


Texture * Texture::createFromImage(const QImage & img, gl::GLenum gpu_format)
{
    MO_DEBUG_IMG("Texture::createFromQImage(" << &img << ", " << gpu_format << ")");

    if (img.isNull())
    {
        MO_GL_ERROR("createFromImage() with NULL qimage");
    }

    if (img.width() == 0 || img.height() == 0 )
    {
        MO_GL_ERROR("createFromImage() with empty qimage");
    }

    // determine texture format from image format
#if QT_VERSION >= 0x050200
    GLenum iformat = GL_RGBA;
#else
    GLenum iformat = GL_RGB;
#endif
    GLenum itype = GL_UNSIGNED_BYTE;

    // create and bind
    Texture * tex = new Texture(
                img.width(), img.height(),
                gpu_format, iformat, itype, 0);
    tex->name_ += "_from_img";

    std::unique_ptr<Texture> texDel(tex);

    tex->create();

    // upload image data

    try
    {
        const QImage * pimg = &img;
        QImage cpy;

#if QT_VERSION >= 0x050200
        QImage::Format fmt = QImage::Format_RGBA8888;
#else
        QImage::Format fmt = QImage::Format_RGB888;
#endif
        if (img.format() != fmt)
        {
            cpy = img.convertToFormat(fmt);
            pimg = &cpy;
        }

        // create empty texture
        MO_CHECK_GL_THROW(
        gl::glTexImage2D(
            tex->target_,
            // mipmap level
            0,
            // color components
            GLint(gpu_format),
            // size
            img.width(), img.height(),
            // boarder
            0,
            // input format
            iformat,
            // data type
            itype,
            // ptr
            0)
        );
        
        // upload line-by-line
        // 'cause Qt stores it's images upside-down
        for (int i=0; i<img.height(); ++i)
        {
            MO_CHECK_GL_THROW(
                gl::glTexSubImage2D(tex->target_, 0,
                                0, i, img.width(), 1,
                                iformat, itype,
                                pimg->constScanLine(img.height()-1-i))
                        );
        }

    }
    catch (Exception & e)
    {
        if (tex->isHandle())
            tex->release();
        e << "\non creating texture from image " << img.width() << "x" << img.height();
        throw;
    }

    return texDel.release();
}


Texture * Texture::createFromImage(const QString &filename, gl::GLenum gpu_format)
{
    ImageReader reader;
    reader.setFilename(filename);

    QImage img = reader.read();
    if (img.isNull())
    {
        MO_GL_ERROR("Could not load image file\n'"
                         << filename << "'\n'" << reader.errorString() << "'");
    }

    return createFromImage(img, gpu_format);
}

} // namespace GL
} // namespace MO
