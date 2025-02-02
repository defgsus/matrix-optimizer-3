/** @file texture.h

    @brief Wrapper for OpenGL Texture

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05/20/2012, pulled-in 8/1/2014</p>
*/

#ifndef MOSRC_GL_TEXTURE_H
#define MOSRC_GL_TEXTURE_H

#include "gl/opengl.h"

class QImage;

namespace MO {
namespace GL {

class Texture
{
public:
    explicit Texture();

    /** target GL_TEXTURE_2D or GL_TEXTURE_2D_MULTISAMPLE,
        explicit format, input_format, type */
    explicit Texture(
                gl::GLsizei width, gl::GLsizei height,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void* ptr_to_data,
                gl::GLsizei multiSamples = 0);

    /** target GL_TEXTURE_3D, explicit format, input_format, type */
    explicit Texture(
                gl::GLsizei width, gl::GLsizei height, gl::GLsizei depth,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void* ptr_to_data);

    /** target GL_TEXTURE_CUBE_MAP, explicit format, input_format, type */
    explicit Texture(
                gl::GLsizei width, gl::GLsizei height,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void * ptr_px, void * ptr_nx,
                void * ptr_py, void * ptr_ny,
                void * ptr_pz, void * ptr_nz);

    virtual ~Texture();

    /** Create a texture using settings from @p other */
    static Texture * constructFrom(const Texture * other);

    // -------------- static ----------------------

    /** Equal to glActiveTexture(GL_TEXTURE0 + slot).
        @throws GlException */
    static void setActiveTexture(gl::GLuint slot);

    /* Creates a new Texture from an image.
        Returns NULL on fail, or throws exception.
        OpenGL context must be present of course. */
    //static Texture * createFromImage(const Image&, gl::GLenum gpu_format, ErrorReporting = ER_THROW);

    /** Creates a new Texture from an image.
        Returns NULL on fail, or throws exception.
        OpenGL context must be present of course. */
    static Texture * createFromImage(const QImage&, gl::GLenum gpu_format, uint mipmap_levels = 0);

    /** Creates a new Texture from an image file.
        Returns NULL on fail, or throws exception.
        OpenGL context must be present. */
    static Texture * createFromImage(const QString& filename, gl::GLenum gpu_format, uint mipmap_levels = 0);

    // --------------- getter ---------------------

    /** Returns a one-liner with info about the texture */
    QString infoString() const;

    /** Returns whether the texture can be bound/created/uploaded. */
    bool isHandle() const { return handle_ != invalidGl; }
    /** Returns whether the texture is allocated on the device. */
    bool isAllocated() const { return uploaded_; }

    /** When true, target() is GL_TEXTURE_CUBE_MAP */
    bool isCube() const;

    /** Returns true when texture target equals GL_TEXTURE_3D */
    bool is3d() const;

    /** Returns true when texture target equals GL_TEXTURE_2D_MULTISAMPLE */
    bool isMultiSample() const;

    gl::GLsizei numMultiSamples() const { return multiSamples_; }

    /** Returns the set number of mip-map levels, 0 for no mip-mapping.
        @see createMipmaps() */
    uint numMipmapLevels() const { return mipLevels_; }

    /** Returns a number which can be used to check if the texture has changed.
        Used by texture-in objects.
        hash will count upwards on every call to setChanged() */
    int hash() const { return hash_; }

    /** Calculated memory of texture in bytes */
    gl::GLsizei memory() const { return memory_; }

    const QString& name() const { return name_; }
    void setName(const QString& name) { name_ = name; }

    /** Approximated memory usage of all textures */
    static long int memoryAll() { return memory_used_; }

    uint width() const { return width_; }
    uint height() const { return height_; }
    uint depth() const { return depth_; }
    gl::GLenum format() const { return format_; }
    gl::GLenum type() const { return type_; }
    gl::GLenum target() const { return target_; }
    gl::GLuint handle() const { return handle_; }

    /** Returns the assigned pointer. */
    void * pointer() const { return ptr_; }


    // -------- openGL interface ---------------

    /** Sets a change to the texture. Will change the number returned in hash() */
    void setChanged();

    /** Binds the texture handle to the current slot */
    void bind() const;

    /** Convenience function for glTexParameteri(target, param, value) */
    void setTexParameter(gl::GLenum param, gl::GLint value) const;

    /** @{ */
    /** create() (re-)defines the piece of data that Texture should work with.
        The data is created or uploaded depending if @p ptr_to_data is not NULL.
        @throws GlException */

    /** create 1d, explicit format, input_format, type */
    void create(gl::GLsizei width,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void* ptr_to_data);

    /** create 2d, explicit format, input_format, type */
    void create(gl::GLsizei width, gl::GLsizei height,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void* ptr_to_data);

    /** create 3d, explicit format, input_format, type */
    void create(gl::GLsizei width, gl::GLsizei height, gl::GLsizei depth,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void* ptr_to_data);

    /** create 2d, explicit format and input_format, type */
    void create(gl::GLsizei width, gl::GLsizei height,
                gl::GLenum format,
                gl::GLenum type,
                void* ptr_to_data)
    { create(width, height, format, format, type, ptr_to_data); }

    /** create cube, explicit format, input_format, type */
    void create(gl::GLsizei width, gl::GLsizei height,
                gl::GLenum format, gl::GLenum input_format,
                gl::GLenum type,
                void * ptr_px, void * ptr_nx,
                void * ptr_py, void * ptr_ny,
                void * ptr_pz, void * ptr_nz);

    /** @} */ // create

    /** Creates the texture on the device as defined in constructor.
        @throws GlException */
    void create();

    /** Creates mip-map levels for the base texture.
        The texture must already be created on the GPU. */
    void createMipmaps(uint max_level, gl::GLenum mode = gl::GL_LINEAR_MIPMAP_LINEAR);

    /** Uploads the data specified previously.
        <br>if ptr_to_data in constructor or create() was NULL,
        the call is equivalent to create().
        <br><b>Texture must be bound()</b>
        @throws GlException
        */
    void upload(gl::GLint mipmap_level = 0);

    /** Uploads the data specified by ptr_to_data.
        For cube-maps, all textures will be uploaded from the same source.
        <br><b>Texture must be bound()</b>
        @throws GlException
        */
    void upload(void * ptr_to_data, gl::GLint mipmap_level = 0);

    /** Downloads the texture from device to host. <br/>
        If the pointer is zero, the previously assigned pointer is used.
        Eventually, the pointer must be non-zero and capable of storing the required memory.
        If the texture is a cubemap, then assigning a pointer is illegal,
        and the data will be downloaded to the previously defined individual pointers.
        <br><b>Texture must be bound()</b>
        @throws GlException
        */
    void download(void * ptr_to_data = 0, gl::GLuint mipmap_level = 0) const;

    /** Downloads the texture from device to host, converting the format. <br/>
        The pointer must be non-zero and capable of storing the required memory.
        Illegal for cubemaps!
        <br><b>Texture must be bound()</b>
        @throws GlException */
    void download(void * ptr_to_data, gl::GLenum format, gl::GLenum type, gl::GLuint mipmap_level = 0) const;

    /** free the device data and release handle */
    void release();

    // ---------------- QImage ----------------

    /** Creates an QImage from the texture data.
        @note The texture must be bound!
        @throws GlException */
    QImage toQImage() const;

    /** Saves the texture to a file, using toQImage().
        @note The texture must be bound!
        @throws GlException or IoException */
    void saveImageFile(const QString& fn) const;

private:

    Texture(const Texture&);
    void operator=(const Texture&);

    /** Generates a texture handle */
    gl::GLuint genTexture_() const;

    /** Releases the texture handle (and device data) */
    void releaseTexture_();

    /** Creates or uploads the texture */
    void upload_(const void * ptr, gl::GLint mipmap_level,
                 gl::GLenum cube_target = gl::GLenum(0));
    /** Updates the data for already allocated texture */
    void uploadFast_(const void * ptr_to_data, gl::GLint mipmap_level,
                     gl::GLenum cube_target = gl::GLenum(0));
    /** Downloads to ptr (must be non-NULL) */
    void download_(void * ptr, gl::GLuint mipmap, gl::GLenum target, gl::GLenum type) const;

    /** device memory counter */
    static long int memory_used_;

    void
    /** pointer to data for 1d/2d */
        *ptr_,
        *ptr_px_, *ptr_nx_, *ptr_py_, *ptr_ny_, *ptr_pz_, *ptr_nz_;
    bool
    /** already uploaded? */
        uploaded_;

    gl::GLsizei
        width_,
        height_,
        depth_,
        multiSamples_,
        mipLevels_,
    /** calculated memory of texture in bytes */
        memory_;
    gl::GLuint
    /** texture identifier */
        handle_;
    gl::GLenum
    /** 1d/2d/3d */
        target_,
    /** device channel format */
        format_,
    /** data channel format */
        input_format_,
    /** data type */
        type_;

    QString name_;
    int hash_;
};

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_TEXTURE_H
