/** @file model.h

    @brief Geometry container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRY_H
#define MOSRC_GEOM_GEOMETRY_H

#include <map>

#include "gl/opengl.h"
#include "types/vector.h"

namespace MO {
namespace GEOM {

/** XXX This is not very generic
    - but let's get started and see later.. */
class Geometry
{
public:

    // ------- types ---------

    typedef gl::GLfloat VertexType;
    typedef gl::GLfloat NormalType;
    typedef gl::GLfloat ColorType;
    typedef gl::GLfloat TextureCoordType;
    typedef gl::GLuint  IndexType;

    static const IndexType invalidIndex = (IndexType)-1;

    /** Smallest possible threshold for vertex sharing */
    static const VertexType minimumThreshold;

    // ------ enums ----------

    static const gl::GLenum vertexEnum;
    static const gl::GLenum normalEnum;
    static const gl::GLenum colorEnum;
    static const gl::GLenum textureCoordEnum;
    static const gl::GLenum indexEnum;

    // -------- ctor ---------

    Geometry();

    // -------- file io ------

    /** Throws IoException on errors */
//    void loadOBJ(const QString& filename);

    // ------- query ---------

    /** Returns the progress during certain intense functions [0,100] */
    int progress() const { return progress_; }

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

    IndexType triangleIndex(uint triangleIndex, uint cornerIndex) const
        { return triIndex_[triangleIndex * numTriangleIndexComponents() + cornerIndex]; }

    /** Returns memory usage in bytes */
    long unsigned int memory() const;

    /** Returns the shared-mode */
    bool sharedVertices() const { return sharedVertices_; }
    VertexType sharedVerticesThreshold() const { return threshold_; }

    /** Returns the point of the vertex */
    Vec3 getVertex(IndexType vertexIndex) const;
    Vec4 getColor(IndexType vertexIndex) const;
    Vec3 getNormal(IndexType vertexIndex) const;
    Vec2 getTexCoord(IndexType vertexIndex) const;

    const VertexType * triangle(IndexType triangeleIndex, IndexType cornerIndex) const;
    const VertexType * line(IndexType lineIndex, IndexType endIndex) const;

    /** Returns the minimum and maximum vertex coordinates */
    void getExtent(VertexType * minX, VertexType * maxX,
                   VertexType * minY, VertexType * maxY,
                   VertexType * minZ, VertexType * maxZ) const;

    /** Returns the minimum and maximum vertex coordinates */
    void getExtent(Vec3 * minimum, Vec3 * maximum) const;

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
        If setSharedVertices() was enabled, the vertex might
        be reused and not added.
        @returns the index of the vertex. */
    IndexType addVertex(VertexType x, VertexType y, VertexType z,
                  NormalType nx, NormalType ny, NormalType nz,
                  ColorType r, ColorType g, ColorType b, ColorType a,
                  TextureCoordType u, TextureCoordType v);

    /** Adds a vertex (point) with currently set normal and color.
        Does not check for setSharedVertices().
        @see setColor(), setNormal() */
    IndexType addVertexAlways(VertexType x, VertexType y, VertexType z)
    { return addVertexAlways(x,y,z, curNx_,curNy_,curNz_, curR_, curG_, curB_, curA_, curU_, curV_); }

    /** Adds a vertex (point) with normal and color.
        Does not check for setSharedVertices().
        @returns the index of the vertex. */
    IndexType addVertexAlways(VertexType x, VertexType y, VertexType z,
                  NormalType nx, NormalType ny, NormalType nz,
                  ColorType r, ColorType g, ColorType b, ColorType a,
                  TextureCoordType u, TextureCoordType v);

    /** Connects three previously created indices to form a triangle. */
    void addTriangle(IndexType p1, IndexType p2, IndexType p3);

    /** Connects two previously created indices to form a line */
    void addLine(IndexType p1, IndexType p2);

    // ------- shared vertices -------------

    /** Enables or disables shared vertices.
        This mode is off by default.
        When enabled, addVertex() will reuse existing vertices and
        normals, colors, etc.. will be averaged.
        @note This setting must be made prior to any call of addVertex() and
        @p threshold must not changed afterwards.
        @p threshold will be clipped to minimally 0.001 and vertex coordinates
        can maximally be +/-8350 for the smallest threshold. */
    void setSharedVertices(bool enable, VertexType threshold = 0.001);

    /** Returns the vertex index for the given position.
        If there is no vertex at the given position,
        or setSharedVertices() was not enabled prior to adding vertices,
        this call will return invalidIndex */
    IndexType findVertex(VertexType x, VertexType y, VertexType z) const;

    // --------- manipulation --------------

    /** Copies all data, step by step.
        If vertex sharing is enabled, the vertices get shared even
        if they are not in @p other */
    void copyFrom(const Geometry& other);

    /** Scale all geometry */
    void scale(VertexType x, VertexType y, VertexType z);

    /** Translate all geometry */
    void translate(VertexType x, VertexType y, VertexType z);

    /** Apply the matrix to all vertices */
    void applyMatrix(const Mat4& transformation);

    /** Converts all triangles to lines */
    void convertToLines();

    /** Automatically calculates all normals for each triangle.
        Normals that share multiple triangles will be averaged. */
    void calculateTriangleNormals();

    /** Inverts all normals (pointing inwards) */
    void invertNormals();

    /** Invertes (1-uv) the texture coordinates for u and v axis. */
    void invertTextureCoords(bool invX, bool invY);

    /** Offsets the texture coords */
    void shiftTextureCoords(TextureCoordType offsetX, TextureCoordType offsetY);

    /** Multiplies the texture coords */
    void scaleTextureCoords(TextureCoordType scaleX, TextureCoordType scaleY);

    /** Makes every vertex in the model unique.
        After this call, every triangle or line vertex will be unique. */
    void unGroupVertices();

    /** Split all triangles into smaller ones. */
    void tesselate(uint level = 1);

    /** Normalize all vertex positions.
        @p normalization is the amount of normalization [0,1] */
    void normalizeSphere(VertexType scale = 1, VertexType normalization = 1.0);

    /** Removes triangles or lines with a probability [0,1] */
    void removePrimitivesRandomly(float probability, int seed);

    void transformWithNoise(VertexType modX, VertexType modY, VertexType modZ,
                            VertexType scaleX, VertexType scaleY, VertexType scaleZ,
                            int seedX, int seedY, int seedZ);

    /** Applies the equation to the each vertex.
        Variables are x, y, z and i (for vertex index). */
    bool transformWithEquation(const QString& equationX,
                               const QString& equationY,
                               const QString& equationZ);

    /** Applies the equation to the each vertex of each primitive.
        Variables are x, y, z (position), nx, ny, nz (normal), s, t (tex-coords),
        x1, y1, z1, x2, y2, z2, x3, y3, z3 (position of each primitive vertex),
        nx1, ny1, nz1, nx2, ny2, nz2, nx3, ny3, nz3 (normal of each primitive vertex),
        s1, t1, s2, t2, s3, t3 (tex-coord of each primitive),
        i (primitive index) and p (for index of vertex in primitive). */
    bool transformPrimitivesWithEquation(
                               const QString& equationX,
                               const QString& equationY,
                               const QString& equationZ);

    /** Applies the equation to the texture coordinates.
        Variables are x, y, z, s, t and i (for vertex index). */
    bool transformTexCoordsWithEquation(
                                const QString& equationS,
                                const QString& equationT);

    /** Extrudes all triangles along their normals -> into @p geom.
        If @p createNewFaces is true, the orthogonal or orthonormals side faces
        are created.
        If also @p recognizeEdges is true, then only those side faces will be created
        that don't circumvent original triangles with the same normal. E.g.
        a quad will have 4 side faces and the inner triangle edge face is not created. */
    void extrudeTriangles(Geometry & geom, VertexType constant, VertexType factor,
                          bool createNewFaces, bool recognizeEdges) const;

    // ------------- opengl -----------------

    /** Fills the vertex array object with the data from the geometry.
        The shader provides the locations of the vertex attributes.
        The Shader must be compiled.
        The VAO will be released if it was created previously. */
    void getVertexArrayObject(GL::VertexArrayObject *, GL::Shader *, bool triangles = true);

private:

    std::vector<VertexType>       vertex_;
    std::vector<NormalType>       normal_;
    std::vector<ColorType>        color_;
    std::vector<TextureCoordType> texcoord_;
    std::vector<IndexType>        triIndex_, lineIndex_;

    ColorType
        curR_, curG_, curB_, curA_;
    NormalType
        curNx_, curNy_, curNz_;
    TextureCoordType
        curU_, curV_;

    volatile int progress_;

    // ------- vertex sharing ---------

    typedef quint64 Key_;

    struct MapStruct_
    {
        IndexType idx;
        uint count;
        MapStruct_(IndexType idx) : idx(idx), count(1) { }
    };

    std::map<Key_, MapStruct_> indexMap_;

    bool sharedVertices_;
    VertexType threshold_;
};

} // namespace GEOM
} // namespace MO

#endif // MOSRC_GL_GEOMETRY_H
