/** @file texture.h

    @brief Wrapper for OpenGL Texture

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05/20/2012, pulled-in 8/1/2014</p>
*/

#ifndef MOSRC_GL_TEXTURE_H
#define MOSRC_GL_TEXTURE_H

#include "gl/opengl.h"

namespace MO {
namespace GL {

class Texture
{
public:
    explicit Texture(ErrorReporting reporting = ER_IGNORE);

    /** 2d, explicit format, input_format, type */
    explicit Texture(
                GLsizei width, GLsizei height,
                GLenum format, GLenum input_format,
                GLenum type,
                void* ptr_to_data,
                ErrorReporting reporting = ER_IGNORE);

    virtual ~Texture();

    // --------------- getter ---------------------

    /** Returns whether the texture can be bound/created/uploaded. */
    bool isCreated() const { return handle_ != invalidGl; }
    /** Returns whether the texture is allocated on the device. */
    bool isAllocated() const { return uploaded_; }

    uint width() const { return width_; }
    uint height() const { return height_; }
    GLenum format() const { return format_; }
    GLenum type() const { return type_; }
    GLenum target() const { return target_; }
    GLuint handle() const { return handle_; }

    /** Returns the assigned pointer. */
    void * pointer() const { return ptr_; }


    // -------- openGL interface ---------------

    /** bind texture handle and enable texture target */
    bool bind() const;

    /** disable corresponding texture target */
    void unbind() const;

    /** Convenience function for glTexParameteri(target, param, value) */
    void texParameter(GLenum param, GLenum value) const;

    /** @{ */
    /** create() (re-)defines the piece of data that Texture should work with.
        The data is created or uploaded depending if @p ptr_to_data is not NULL. */

    /** assign 2d, explicit format, input_format, type */
    bool create(GLsizei width, GLsizei height,
                GLenum format, GLenum input_format,
                GLenum type,
                void* ptr_to_data);

    /** assign 2d, explicit format and input_format, type */
    bool create(GLsizei width, GLsizei height,
                GLenum format,
                GLenum type,
                void* ptr_to_data)
    { return create(width, height, format, format, type, ptr_to_data); }

    /** @} */ // create

    /** Creates the texture on the device as defined in constructor. */
    bool create();

    /** Uploads the data specified previously.
        <br>if ptr_to_data in constructor was NULL,
        the call is equivalent to create().
        <br><b>Texture must be bound()</b>
        */
    bool upload(GLint mipmap_level = 0);

    /** Uploads the data specified by ptr_to_data.
        <br><b>Texture must be bound()</b>
        */
    bool upload(void * ptr_to_data, GLint mipmap_level = 0);

    /** Downloads the texture from device to host. <br/>
        If the pointer is zero, the previously assigned pointer is used.
        Eventually, the pointer must be non-zero and capable of storing the required memory.
        <br><b>Texture must be bound()</b>
        */
    bool download(void * ptr_to_data = 0) const;

    /** free the device data and release handle */
    void release();


private:

    /** device memory counter */
    static long int memory_used_;

    ErrorReporting rep_;

    /** Generates a texture handle */
    GLuint genTexture_() const;

    /** Releases the texture handle (and device data) */
    void releaseTexture_();

    /** Creates or uploads the texture */
    bool upload_(void * ptr, GLint mipmap_level);

    void
    /** pointer to data */
        *ptr_;
    bool
    /** already uploaded? */
        uploaded_;

    GLsizei
        width_,
        height_,
    /** calculated memory of texture in bytes */
        memory_;
    GLuint
    /** texture identifier */
        handle_;
    GLenum
    /** 1d/2d/3d */
        target_,
    /** device channel format */
        format_,
    /** data channel format */
        input_format_,
    /** data type */
        type_;
};

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_TEXTURE_H
