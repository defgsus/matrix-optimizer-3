/** @file model.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_GL_MODEL_H
#define MOSRC_GL_MODEL_H

#include "openglfunctions.h"
#include "types/vector.h"

// XXX no,no,no this shouldn't be done in a hurry...

namespace MO {
namespace GL {

class Model //: protected MO_QOPENGL_FUNCTIONS_CLASS
{
public:

    // ------- types ---------

    typedef GLfloat VertexType;
    typedef GLfloat NormalType;
    typedef GLfloat ColorType;
    typedef GLfloat TextureCoordType;
    typedef GLuint  IndexType;

    // ------ enums ----------

    static const GLenum VertexEnum       = GL_FLOAT;
    static const GLenum NormalEnum       = GL_FLOAT;
    static const GLenum ColorEnum        = GL_FLOAT;
    static const GLenum TextureCoordEnum = GL_FLOAT;
    static const GLenum IndexEnum        = GL_UNSIGNED_INT;

    // -------- ctor ---------

    Model();

    // ------- query ---------

    /** Returns number of vertices in the Model */
    int numVertices() const { return vertex_.size() / 3; }

    /** Returns number of triangles in the Model */
    int numTriangles() const { return index_.size() / 3; }

    /** Returns if a vertex array object has been initialized for this model. */
    //bool isVAO() const { return isVAO_; }

    // --------- state -----------------------

    /** Sets the current color. Any subsequent call to the
        easy form of addVertex() will use this color. */
    void setColor(ColorType r, ColorType g, ColorType b, ColorType a)
        { curR_ = r; curG_ = g; curB_ = b; curA_ = a; }

    /** Sets the current normal. Any subsequent call to the
        easy form of addVertex() will use this normal. */
    void setNormal(NormalType nx, NormalType ny, NormalType nz)
        { curNx_ = nx; curNy_ = ny; curNz_ = nz; }

    /** Sets the current texture coordinates.
        Any subsequent call to the easy form of
        addVertex() will use this coords. */
    void setTexCoord(TextureCoordType u, TextureCoordType v)
        { curU_ = u; curV_ = v; }

    // -------- vertex/triangle handling -----

    /** Clear ALL contents */
    void clear();

    /** Adds a vertex (point) with currently set normal and color.
        @returns the index of the vertex.
        @see setColor(), setNormal() */
    IndexType addVertex(VertexType x, VertexType y, VertexType z)
        { return addVertex(x,y,z, curNx_,curNy_,curNz_, curR_, curG_, curB_, curA_, curU_, curV_); }

    /** Adds a vertex (point) with normal and color.
        @returns the index of the vertex. */
    IndexType addVertex(VertexType x, VertexType y, VertexType z,
                  NormalType nx, NormalType ny, NormalType nz,
                  ColorType r, ColorType g, ColorType b, ColorType a,
                  TextureCoordType u, TextureCoordType v);

    /** Connects three previously created indices to form a triangle. */
    void addTriangle(IndexType p1, IndexType p2, IndexType p3);

    // ------- convenience functions -------

    /** Automatically calculates all normals for each triangle.
        Normals that share multiple triangles will be averaged. */
    void calculateTriangleNormals();

    /** Makes every vertex in the model unique.
        After this call, every triangle will have it's unique vertices. */
    void unGroupVertices();

    // ------------- opengl / drawing ---------------

    /** See if the OpenGL functions have been initialized. */
    bool isGlInitialized() const { return isGlFuncInitialized_; }

    /** @{ */
    /** All these functions need to be called from within an opengl context! */

    /** Must be called once to initialize the OpenGLFunctions of this class. */
    bool initGl() { return isGlFuncInitialized_ = initQtGl_(); }

    /** Release existing opengl resources.
        Call this before deleting the model. */
    void releaseGl();

    /** Transmits the vertex attribute locations from the shader.
        This needs to be called for Model to create it's vertex array object */
    //void setShaderLocations(const ShaderLocations&);

    /** Draws the vertex array object.
        @note This needs a shader working with the vertex attributes. */
    virtual void draw() = 0;

    /** Draws the model through oldschool opengl arrays */
    //void drawOldschool();

    /** @} */

protected:

    virtual bool initQtGl_() = 0;

    std::vector<VertexType>       vertex_;
    std::vector<NormalType>       normal_;
    std::vector<ColorType>        color_;
    std::vector<TextureCoordType> texcoord_;
    std::vector<IndexType>        index_;

private:

    /** Creates the vertexArrayObject from the initialized data. */
    //void createVAO_();

    bool isGlFuncInitialized_;

    ColorType
        curR_, curG_, curB_, curA_;
    NormalType
        curNx_, curNy_, curNz_;
    TextureCoordType
        curU_, curV_;

#if (0)
    ShaderLocations attribs_;

    /** vertex array object */
    GLuint buffers_[4], vao_;
    bool isVAO_;
#endif

};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_MODEL_H
