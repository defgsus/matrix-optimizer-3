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

    static const GLenum vertexEnum       = GL_FLOAT;
    static const GLenum normalEnum       = GL_FLOAT;
    static const GLenum colorEnum        = GL_FLOAT;
    static const GLenum textureCoordEnum = GL_FLOAT;
    static const GLenum indexEnum        = GL_UNSIGNED_INT;

    // -------- ctor ---------

    Geometry();

    // ------- query ---------

    unsigned int numVertexComponents() const { return 3; }
    unsigned int numNormalComponents() const { return 3; }
    unsigned int numColorComponents() const { return 4; }
    unsigned int numTextureCoordComponents() const { return 2; }
    unsigned int numTriangleIndexComponents() const { return 3; }
    unsigned int numLineIndexComponents() const { return 2; }

    /** Returns number of vertices in the Model */
    unsigned int numVertices() const { return vertex_.size() / numVertexComponents(); }

    /** Returns number of triangles in the Model */
    unsigned int numTriangles() const { return triIndex_.size() / numTriangleIndexComponents(); }

    /** Returns number of lines in the Model */
    unsigned int numLines() const { return lineIndex_.size() / numLineIndexComponents(); }

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
    /** Returns a pointer to numTriangles() * 3 indices */
    const IndexType * lineIndices() const { return &lineIndex_[0]; }

    int numVertexBytes() const { return vertex_.size() * sizeof(VertexType); }
    int numNormalBytes() const { return normal_.size() * sizeof(NormalType); }
    int numColorBytes() const { return color_.size() * sizeof(ColorType); }
    int numTextureCoordBytes() const { return texcoord_.size() * sizeof(TextureCoordType); }
    int numTriangleIndexBytes() const { return triIndex_.size() * sizeof(IndexType); }
    int numLineIndexBytes() const { return lineIndex_.size() * sizeof(IndexType); }

    /** Returns memory usage in bytes */
    long unsigned int memory() const;

    /** Returns the point of the vertex */
    Vec3 getVertex(IndexType vertexIndex) const;
    /*Vec3 getColor(IndexType vertexIndex) const;
    Vec3 getNormal(IndexType vertexIndex) const;
    Vec3 getTexCoord(IndexType vertexIndex) const;*/

    const VertexType * triangle(IndexType triangeleIndex, IndexType cornerIndex) const;
    const VertexType * line(IndexType lineIndex, IndexType endIndex) const;

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

    /** Connects two previously created indices to form a line */
    void addLine(IndexType p1, IndexType p2);

    // --------- manipulation --------------

    /** Scale all geometry */
    void scale(VertexType x, VertexType y, VertexType z);

    /** Converts all triangles to lines */
    void convertToLines();

    /** Split all triangles into smaller ones. */
    void tesselate(uint level = 1);

    /** Normalize all vertices */
    void normalizeSphere(VertexType scale = 1);

    // ------- convenience functions -------

    /** Automatically calculates all normals for each triangle.
        Normals that share multiple triangles will be averaged. */
    void calculateTriangleNormals();

    /** Makes every vertex in the model unique.
        After this call, every triangle will have it's unique vertices. */
    void unGroupVertices();

    // ------------- opengl -----------------

    /** Fills the vertex array object with the data from the geometry.
        The shader provides the locations of the vertex attributes.
        The Shader must be compiled.
        The VAO will be released if it was created previously. */
    void getVertexArrayObject(VertexArrayObject *, Shader *, bool triangles = true);

protected:

    std::vector<VertexType>       vertex_;
    std::vector<NormalType>       normal_;
    std::vector<ColorType>        color_;
    std::vector<TextureCoordType> texcoord_;
    std::vector<IndexType>        triIndex_, lineIndex_;

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
