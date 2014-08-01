/** @file shader.h

    @brief GLSL shader wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created about 6/2014 as part of github.com/defgsus/scheeder</p>
    <p>reworked for MO3 7/27/2014</p>
*/

#ifndef MOSRC_GL_SHADER_H
#define MOSRC_GL_SHADER_H

#include <vector>
#include <memory>

#include <QString>

#include "opengl.h"

namespace MO {
namespace GL {

class ShaderSource;

/** Container for a GLSL uniform. */
class Uniform
{
public:

    // ------- public member ---------

    union
    {
        /** A vector of floats,
            for types GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3 and GL_FLOAT_VEC4. */
        GLfloat floats[4];

        /** A vector of ints,
            for types GL_INT, GL_INT_VEC2, GL_INT_VEC3 and GL_INT_VEC4 */
        GLint ints[4];
    };

    // ----- getter -----

    /** Name as in the shader source */
    const QString& name() const { return name_; }
    /** Type of the uniform (as OpenGL enum) */
    GLenum type() const { return type_; }
    /** Number of instances (for arrays) */
    GLint size() const { return size_; }
    /** Uniform location, to send the stuff over */
    GLint location() const { return location_; }

    // ----- setter -----

    void setFloats(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
    { floats[0] = x; floats[1] = y; floats[2] = z; floats[3] = w; }


    friend class Shader;
    friend void privateUniformDeleter(Uniform*);

    // ----------- private area -----------
private:

    /** Constructor (initializes all to zero) */
    Uniform();
    /** Private destructor to avoid stupid things. */
    ~Uniform() { }

    void copyValuesFrom_(Uniform*);

    QString name_;
    GLenum type_;
    GLint size_;
    GLint location_;

};


/** Container for a GLSL attribute. */
class Attribute
{
public:

    // ----- getter -----

    /** Name as in the shader source */
    const QString& name() const { return name_; }
    /** Type of the uniform (as OpenGL enum) */
    GLenum type() const { return type_; }
    /** Number of instances (for arrays) */
    GLint size() const { return size_; }
    /** Attribute location, to send the stuff over */
    GLint location() const { return location_; }

    friend class Shader;
    friend void privateAttributeDeleter(Attribute*);

    // ----------- private area -----------
private:

    /** Constructor (initializes all to zero) */
    Attribute();
    /** Private destructor to avoid stupid things. */
    ~Attribute() { }

    QString name_;
    GLenum type_;
    GLint size_;
    GLint location_;
};


class Shader
{
public:

    // ---------------- ctor -----------------

    /** @p name is a user defined name, mainly for help with debugging */
    Shader(const QString& name);
    ~Shader();

    // ----------- query ---------------------

    /** Return the log from the last compilation */
    const QString& log() const { return log_; }

    /** Has the source changed and shader needs recompilation? */
    bool sourceChanged() const { return sourceChanged_; }

    /** Is the shader ready to use? */
    bool ready() const { return ready_; }

    /** Returns if the shader has been activated.
        @note If, after activation, activate() or deactivate() is called on a
        different shader, this value will not reflect the GPU state! */
    bool activated() const { return activated_; }

    /** Returns the internal source object */
    const ShaderSource * source() const { return source_; }

    /** Returns the number of used uniforms of this shader.
        Can be called after succesful compilation. */
    size_t numUniforms() const { return uniforms_.size(); }

    /** Returns a pointer to a uniform attached to this shader.
        The public members of the Uniform class can be manipulated.
        Can be called after succesful compilation.
        @p index must be < numUniforms() */
    Uniform * getUniform(size_t index);

    /** Returns the program index in OpenGL space after successful compilation. */
    GLuint programId() const { return prog_; }

    /** Returns the list of all attached uniforms.
        The public members of the Uniform classes can be manipulated.
        Can be called after succesful compilation. */
    const QList<Uniform*> getUniforms() const { return uniformList_; }

    /** Returns a pointer to the Uniform with the given name, or NULL.
        Can be called after succesful compilation.
        If @p expect is true, an GlException will be thrown if the uniform
        is not defined. */
    Uniform * getUniform(const QString& name, bool expect = false);

    /** Returns the number of used attributes of this shader.
        Can be called after succesful compilation. */
    size_t numAttributes() const { return attribs_.size(); }

    /** Returns a pointer to an Attribute attached to this shader.
        Can be called after succesful compilation.
        @p index must be < numUniforms() */
    const Attribute * getAttribute(size_t index) const;

    /** Returns a pointer to the Attribute with the given name, or NULL.
        Can be called after succesful compilation. */
    const Attribute *getAttribute(const QString& name) const;

    /** Returns the list of all attached attributes.
        Can be called after succesful compilation. */
    const QList<const Attribute*> getAttributes() const { return attributeList_; }

    // ---------- source/compiler ------------

    /** Sets all the source code from the ShaderSource class,
        Previous contents will be overwritten.
        The attribute names of the ShaderSource object are expected
        to match the ones in the source. */
    void setSource(const ShaderSource *);

    /** Tries to compile the shader.
        Any previous program will be destroyed but the values of uniforms are kept.
        @returns true on success, also sets ready() to true. */
    bool compile();

    // ------------ usage --------------------

    /** Activates the shader. Subsequent OpenGL calls will
        be affected by the shader's workings. */
    void activate();

    /** Turns the shader off. */
    void deactivate();

    /** Sets the GPU-value of the uniform to the contents provided by
        the @p uniform parameter.
        @note The shader must be activated. */
    void sendUniform(const Uniform * uniform);

    /** Sends all uniform values to the GPU.
        @note The shader must be activated. */
    void sendUniforms();

    /** Releases GPU resources. */
    void releaseGL();

private:

    /** Gets the standardized attributes and uniforms. */
    void getSpecialLocations_();

    /** Gets all used uniforms and populates the uniforms_ list */
    void getUniforms_();

    /** Gets all used attributes and populates the attribs_ list */
    void getAttributes_();

    /** Compiles one of the vertex/fragment shaders and attaches to current programObject */
    bool compileShader_(GLenum type, const QString& typeName, const QString& source);

    ShaderSource * source_;

    QString name_, log_;

    GLuint prog_;

    bool sourceChanged_, ready_, activated_;

    // --- uniforms ---

    std::vector<std::shared_ptr<Uniform>>
        uniforms_,
        oldUniforms_;

    QList<Uniform*> uniformList_;

    // --- attributes ---

    std::vector<std::shared_ptr<Attribute>>
        attribs_;
    QList<const Attribute*> attributeList_;
};


} // namespace GL
} // namespace MO


#endif // MOSRC_GL_SHADER_H
