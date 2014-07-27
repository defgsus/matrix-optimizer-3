/** @file model.h

    @brief Geometry container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_GL_GEOMETRY_H
#define MOSRC_GL_GEOMETRY_H

#include "opengl.h"
#include "types/vector.h"

namespace MO {
namespace GL {

/** XXX This is not very generic
    - but let's get started and see later.. */
class Geometry
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

    Geometry();

    // ------- query ---------

    /** Returns number of vertices in the Model */
    unsigned int numVertices() const { return vertex_.size() / 3; }

    /** Returns number of triangles in the Model */
    unsigned int numTriangles() const { return triIndex_.size() / 3; }

    /** Returns a pointer to numVertices() * 3 coordinates */
    const VertexType * vertices() const { return &vertex_[0]; }
    /** Returns a pointer to numVertices() * 3 coordinates */
    const NormalType * normals() const { return &normal_[0]; }
    /** Returns a pointer to numVertices() * 4 entries */
    const ColorType * colors() const { return &color_[0]; }
    /** Returns a pointer to numVertices() * 2 coordinates */
    const TextureCoordType * textureCoords() const { return &texcoord_[0]; }
    /** Returns a pointer to numTriangles() * 3 indices */
    const IndexType * triangleIndices() const { return &triIndex_[0]; }

    int numVertexBytes() const { return numVertices() * 3 * sizeof(VertexType); }
    int numNormalBytes() const { return numVertices() * 3 * sizeof(NormalType); }
    int numColorBytes() const { return numVertices() * 4 * sizeof(ColorType); }
    int numTextureCoordBytes() const { return numVertices() * 2 * sizeof(TextureCoordType); }
    int numTriangleIndexBytes() const { return numTriangles() * 3 * sizeof(IndexType); }

    const VertexType * triangle(IndexType triangeleIndex, IndexType cornerIndex) const;

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

protected:

    std::vector<VertexType>       vertex_;
    std::vector<NormalType>       normal_;
    std::vector<ColorType>        color_;
    std::vector<TextureCoordType> texcoord_;
    std::vector<IndexType>        triIndex_;

private:

    ColorType
        curR_, curG_, curB_, curA_;
    NormalType
        curNx_, curNy_, curNz_;
    TextureCoordType
        curU_, curV_;

};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_GEOMETRY_H
