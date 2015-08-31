/** @file geometry.h

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
#include "types/refcounted.h"

#ifndef MO_DISABLE_ANGELSCRIPT
class asIScriptEngine;
#endif


namespace MO {
namespace GEOM {

/** A mesh container with lots of functionality.
    This is not totally generic (e.g. types are fixed). */
class Geometry : public RefCounted
{
public:

    // ------- types ---------

    typedef gl::GLfloat VertexType;
    typedef gl::GLfloat NormalType;
    typedef gl::GLfloat ColorType;
    typedef gl::GLfloat TextureCoordType;
    typedef gl::GLuint  IndexType;
    typedef gl::GLfloat AttributeType;

    static const IndexType invalidIndex = (IndexType)-1;

    /** Smallest possible threshold for vertex sharing */
    static const VertexType minimumThreshold;

    struct UserAttribute
    {
        /** Creates an attribute with shader name @p attributeName and @p numComponents per vertex */
        UserAttribute(const QString& attributeName, unsigned int numComponents)
            : attributeName     (attributeName),
              numComponents     (numComponents)
        { curValue.resize(numComponents, 0.f); }

        unsigned long int numBytes() const { return data.size() * sizeof(AttributeType); }
        /** Returns the type as string */
        QString typeName() const;
        /** Returns the glsl declaration */
        QString declaration() const;

        QString attributeName;
        unsigned int numComponents;
        std::vector<AttributeType>
        /** size == numComponents() * numVertices() */
            data,
        /** size == numComponents() */
        curValue;
    };

    // ------ enums ----------

    static const gl::GLenum vertexEnum;
    static const gl::GLenum normalEnum;
    static const gl::GLenum colorEnum;
    static const gl::GLenum textureCoordEnum;
    static const gl::GLenum indexEnum;
    static const gl::GLenum attributeEnum;

    // -------- ctor ---------

    Geometry();
private:
    ~Geometry();
public:

    Geometry(const Geometry& other) { copyFrom(other); }
    Geometry& operator = (const Geometry& other) { copyFrom(other); return *this; }

    void copyFrom(const Geometry& other);

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

    /** Returns number of points in the Model */
    unsigned int numPoints() const { return pointIndex_.size(); }

    /** Returns the number of user attributes */
    unsigned int numAttributes() const { return attributes_.size(); }

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
    /** Returns a pointer to numTriangles() * 3 indices */
    const IndexType * pointIndices() const { return &pointIndex_[0]; }

    int numVertexBytes() const { return vertex_.size() * sizeof(VertexType); }
    int numNormalBytes() const { return normal_.size() * sizeof(NormalType); }
    int numColorBytes() const { return color_.size() * sizeof(ColorType); }
    int numTextureCoordBytes() const { return texcoord_.size() * sizeof(TextureCoordType); }
    int numTriangleIndexBytes() const { return triIndex_.size() * sizeof(IndexType); }
    int numLineIndexBytes() const { return lineIndex_.size() * sizeof(IndexType); }
    int numPointIndexBytes() const { return pointIndex_.size() * sizeof(IndexType); }

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

    /** Returns true when the ray intersects with a triangle of the geometry.
        If @p pos is given, it will be set to the intersection position.
        @note Not terribly efficient */
    bool intersects_any(const Vec3& ray_origin, const Vec3& ray_direction, Vec3 * pos = 0) const;

    /** Returns the closest intersection point of the ray with a triangle of the geometry.
        If @p pos is given, it will be set to the intersection position.
        @note Not terribly efficient */
    bool intersects(const Vec3& ray_origin, const Vec3& ray_direction, Vec3 * pos = 0) const;

    /** Creates some JavaScript code snippet defining some arrays. */
    QString toJavaScriptArray(const QString& baseName,
                              bool withNormals = true, bool withTexCoords = true) const;

    // ----------- user attributes -----------

    /** Returns access to the user attribute, or NULL */
    UserAttribute * getAttribute(const QString& name);
    const UserAttribute * getAttribute(const QString& name) const;

    /** Returns a reference to the specific attribute data, or NULL */
    const AttributeType * attributes(const QString& name) const;

    /** Returns the list of installed attributes */
    QStringList getAttributeNames() const;

    // -------------- setter -----------------

    /** Returns a pointer to numVertices() * 3 coordinates */
    VertexType * vertices() { return &vertex_[0]; }
    /** Returns a pointer to numVertices() * 3 coordinates */
    NormalType * normals() { return &normal_[0]; }
    /** Returns a pointer to numVertices() * 4 entries */
    ColorType * colors() { return &color_[0]; }
    /** Returns a pointer to numVertices() * 2 coordinates */
    TextureCoordType * textureCoords() { return &texcoord_[0]; }
    /** Returns a pointer to numTriangles() * 3 indices */
    IndexType * triangleIndices() { return &triIndex_[0]; }
    /** Returns a pointer to numTriangles() * 3 indices */
    IndexType * lineIndices() { return &lineIndex_[0]; }

    /** Adds a float attribute with @p numComponents per vertex. */
    UserAttribute * addAttribute(const QString& name, unsigned int numComponents);

    /** Creates an attribute containing the vertex indices.
        <point, line, tri, ...> */
    UserAttribute * addEnumerationAttribute(const QString& name);

    /** Returns false if the triangle is degenerate (e.g. on of the edges is too small) */
    static bool checkTriangle(const Vec3&, const Vec3&, const Vec3&);

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

    /** Sets the current attribute. Any subsequent call to addVertex() will use this attribute. */
    void setAttribute(const QString& name, AttributeType x);
    void setAttribute(const QString& name, AttributeType x, AttributeType y);
    void setAttribute(const QString& name, AttributeType x, AttributeType y, AttributeType z);
    void setAttribute(const QString& name, AttributeType x, AttributeType y, AttributeType z, AttributeType w);

    ColorType currentRed() const { return curR_; }
    ColorType currentGreen() const { return curG_; }
    ColorType currentBlue() const { return curB_; }
    ColorType currentAlpha() const { return curA_; }

    NormalType currentNormalX() const { return curNx_; }
    NormalType currentNormalY() const { return curNy_; }
    NormalType currentNormalZ() const { return curNz_; }

    TextureCoordType currentTexCoordX() const { return curU_; }
    TextureCoordType currentTexCoordY() const { return curV_; }

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
    void addTriangle(const Vec3& p1, const Vec3& p2, const Vec3& p3);

    /** Connects three previously created indices to form a triangle.
        Uses checkTriangle() to discard degenerate triangles. */
    void addTriangleChecked(IndexType p1, IndexType p2, IndexType p3);

    /** Connects two previously created indices to form a line */
    void addLine(IndexType p1, IndexType p2);

    /** Create a point sprite for the vertex */
    void addPoint(IndexType idx);

    /** Changes the vertex point */
    void setVertex(uint i, const Vec3& v) { auto p = &vertices()[i * numVertexComponents()]; *p++ = v.x; *p++ = v.y; *p = v.z; }
    /** Changes the texcoord point */
    void setTexCoord(uint i, const Vec2& v) { auto p = &texcoord_[i * numTextureCoordComponents()]; *p++ = v.x; *p = v.y; }
    /** Changes the normal */
    void setNormal(uint i, const Vec3& v) { auto p = &normal_[i * numNormalComponents()]; *p++ = v.x; *p++ = v.y; *p = v.z; }
    /** Changes the color */
    void setColor(uint i, const Vec4& v) { auto p = &color_[i * numColorComponents()]; *p++ = v.x; *p++ = v.y; *p++ = v.z; *p = v.w; }

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

    /** Copies all data, step by step from @p other.
        Previous contents stay as they are.
        If vertex sharing is enabled, the vertices area shared regardless
        of the setting in @p other */
    void addGeometry(const Geometry& other, const Vec3& offset = Vec3(0));

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
    void tesselateTriangles(uint level = 1);
    /** Split all triangles into smaller ones. */
    void tesselateTriangles(VertexType minArea, VertexType minLength, uint level = 1);
    /** Split all lines into smaller ones. */
    void tesselateLines(uint level = 1);

    /** Normalize all vertex positions.
        @p normalization is the amount of normalization [0,1] */
    void normalizeSphere(VertexType scale = 1, VertexType normalization = 1.0);

    /** Removes triangles or lines with a probability [0,1] */
    void removePrimitivesRandomly(float probability, int seed);

    void transformWithNoise(VertexType modX, VertexType modY, VertexType modZ,
                            VertexType scaleX, VertexType scaleY, VertexType scaleZ,
                            int seedX, int seedY, int seedZ);

    /** Applies the equation to the each vertex.
        Variables are x, y, z and i (for vertex index),
        red, green, blue, alpha, bright. */
    bool transformWithEquation(const QString& equation,
                               const QStringList &constantNames = QStringList(),
                               const QList<Double> &constantValues = QList<Double>());

    /** Applies the equation to the each vertex.
        Variables are x, y, z and i (for vertex index). */
    bool transformWithEquation(const QString& equationX,
                               const QString& equationY,
                               const QString& equationZ,
                               const QStringList &constantNames = QStringList(),
                               const QList<Double> &constantValues = QList<Double>());

    /** Applies the equation to the each vertex of each primitive.
        Variables are x, y, z (position), nx, ny, nz (normal), s, t (tex-coords),
        red, green, blue, alpha, bright (colors).
        x1, y1, z1, x2, y2, z2, x3, y3, z3 (position of each primitive vertex),
        nx1, ny1, nz1, nx2, ny2, nz2, nx3, ny3, nz3 (normal of each primitive vertex),
        s1, t1, s2, t2, s3, t3 (tex-coord of each primitive),
        i (primitive index) and p (for index of vertex in primitive). */
    bool transformPrimitivesWithEquation(const QString& equation,
                                         const QStringList &constantNames = QStringList(),
                                         const QList<Double> &constantValues = QList<Double>());
    bool transformPrimitivesWithEquation(const QString& equationX,
                                         const QString& equationY,
                                         const QString& equationZ,
                                         const QStringList &constantNames = QStringList(),
                                         const QList<Double> &constantValues = QList<Double>());

    /** Extrudes all triangles along their normals -> into @p geom.
        If @p createNewFaces is true, the orthogonal or orthonormals side faces
        are created.
        If also @p recognizeEdges is true, then only those side faces will be created
        that don't circumvent original triangles with the same normal. E.g.
        a quad will have 4 side faces and the inner triangle edge face is not created.
        @p shift_center is a factor [0,1] of moving the extruded triangled vertices
        into the center of the extruded triangle and can be negative for experimental
        reasons. */
    void extrudeTriangles(Geometry & geom, VertexType constant, VertexType factor,
                          VertexType shift_center,
                          bool createNewFaces, bool recognizeEdges) const;



    // ------------- opengl -----------------

    /** Fills the vertex array object with the data from the geometry.
        Triangles and lines will create a separate element buffer if present.
        The shader provides the locations of the vertex attributes.
        The Shader must be compiled.
        The VAO will be released if it was created previously. */
    void getVertexArrayObject(GL::VertexArrayObject *, GL::Shader *);

private:

    std::vector<VertexType>       vertex_;
    std::vector<NormalType>       normal_;
    std::vector<ColorType>        color_;
    std::vector<TextureCoordType> texcoord_;
    std::vector<IndexType>        triIndex_, lineIndex_, pointIndex_;
    std::map<QString, UserAttribute*> attributes_;

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
