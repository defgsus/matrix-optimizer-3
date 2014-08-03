/** @file model.cpp

    @brief Geometry container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include <random>

#include <QSet>

#include "geometry.h"
#include "gl/shadersource.h"
#include "gl/shader.h"
#include "gl/vertexarrayobject.h"
#include "math/hash.h"
#include "math/noiseperlin.h"

namespace MO {
namespace GEOM {

Geometry::Geometry()
    :
        curR_   (1.f),
        curG_   (1.f),
        curB_   (1.f),
        curA_   (1.f),
        curNx_  (0.f),
        curNy_  (0.f),
        curNz_  (1.f),
        curU_   (0.f),
        curV_   (0.f)
{
}

long unsigned int Geometry::memory() const
{
    return numVertexBytes()
            + numNormalBytes()
            + numColorBytes()
            + numTextureCoordBytes()
            + numTriangleIndexBytes()
            + numLineIndexBytes();
}

void Geometry::clear()
{
    vertex_.clear();
    normal_.clear();
    color_.clear();
    texcoord_.clear();
    triIndex_.clear();
}

Geometry::IndexType Geometry::addVertex(
                VertexType x, VertexType y, VertexType z,
                NormalType nx, NormalType ny, NormalType nz,
                ColorType r, ColorType g, ColorType b, ColorType a,
                TextureCoordType u, TextureCoordType v)
{
    vertex_.push_back(x);
    vertex_.push_back(y);
    vertex_.push_back(z);

    normal_.push_back(nx);
    normal_.push_back(ny);
    normal_.push_back(nz);

    color_.push_back(r);
    color_.push_back(g);
    color_.push_back(b);
    color_.push_back(a);

    texcoord_.push_back(u);
    texcoord_.push_back(v);

    return numVertices() - 1;
}

void Geometry::addTriangle(IndexType p1, IndexType p2, IndexType p3)
{
    triIndex_.push_back(p1);
    triIndex_.push_back(p2);
    triIndex_.push_back(p3);
}

void Geometry::addLine(IndexType p1, IndexType p2)
{
    lineIndex_.push_back(p1);
    lineIndex_.push_back(p2);
}


Vec3 Geometry::getVertex(const IndexType i) const
{
    return Vec3(vertex_[i*3], vertex_[i*3+1], vertex_[i*3+2]);
}

Vec3 Geometry::getNormal(const IndexType i) const
{
    return Vec3(normal_[i*3], normal_[i*3+1], normal_[i*3+2]);
}

Vec2 Geometry::getTexCoord(const IndexType i) const
{
    return Vec2(texcoord_[i*2], texcoord_[i*2+1]);
}

Vec4 Geometry::getColor(const IndexType i) const
{
    return Vec4(color_[i*4], color_[i*4+1], color_[i*4+2], color_[i*4+3]);
}

const Geometry::VertexType * Geometry::triangle(
        IndexType triangleIndex, IndexType cornerIndex) const
{
    return &vertex_[triIndex_[triangleIndex * numTriangleIndexComponents() + cornerIndex]
            * numVertexComponents()];
}

const Geometry::VertexType * Geometry::line(
        IndexType lineIndex, IndexType endIndex) const
{
    return &vertex_[lineIndex_[lineIndex * numLineIndexComponents() + endIndex]
            * numVertexComponents()];
}

void Geometry::scale(VertexType x, VertexType y, VertexType z)
{
    for (uint i=0; i<numVertices(); ++i)
    {
        vertex_[i*numVertexComponents()] *= x;
        vertex_[i*numVertexComponents()+1] *= y;
        vertex_[i*numVertexComponents()+2] *= z;
    }
}



void Geometry::calculateTriangleNormals()
{
    // first clear the normal array
    normal_.resize(vertex_.size());
    for (size_t i=0; i<normal_.size(); ++i)
        normal_[i] = 0;

    // an array to memorize the number of
    // adjacent triangles for each vertex
    std::vector<size_t> nr_adds(numVertices());

    // for each triangle
    for (size_t i=0; i<numTriangles(); ++i)
    {
        // index of each triangle corner
        size_t
            v1 = triIndex_[i*3],
            v2 = triIndex_[i*3+1],
            v3 = triIndex_[i*3+2];
        // vector of each triangle corner
        Vec3
            p1 = Vec3(vertex_[v1*3], vertex_[v1*3+1], vertex_[v1*3+2]),
            p2 = Vec3(vertex_[v2*3], vertex_[v2*3+1], vertex_[v2*3+2]),
            p3 = Vec3(vertex_[v3*3], vertex_[v3*3+1], vertex_[v3*3+2]);

        // calculate the normal of the triangle
        Vec3 n = glm::normalize( glm::cross( p2-p1, p3-p1 ) );

        // copy/add to normals array
        normal_[v1*3  ] += n[0];
        normal_[v1*3+1] += n[1];
        normal_[v1*3+2] += n[2];
        normal_[v2*3  ] += n[0];
        normal_[v2*3+1] += n[1];
        normal_[v2*3+2] += n[2];
        normal_[v3*3  ] += n[0];
        normal_[v3*3+1] += n[1];
        normal_[v3*3+2] += n[2];

        // memorize the count
        nr_adds[v1]++;
        nr_adds[v2]++;
        nr_adds[v3]++;
    }

    // finally normalize the normals :)
    for (size_t i=0; i<normal_.size(); ++i)
    {
        if (nr_adds[i/3])
            normal_[i] /= nr_adds[i/3];
    }

}

void Geometry::unGroupVertices()
{
    // backup data
    auto vertex = vertex_;
    auto normal = normal_;
    auto color = color_;
    auto texcoord = texcoord_;

    vertex_.clear();
    normal_.clear();
    color_.clear();
    texcoord_.clear();

    if (numTriangles())
    {
        auto index = triIndex_;
        triIndex_.clear();

        // for each previous triangle ..
        for (uint i=0; i<index.size() / 3; ++i)
        {
            IndexType
                i1 = index[i*3],
                i2 = index[i*3+1],
                i3 = index[i*3+2],

                t1 = addVertex(vertex[i1*3], vertex[i1*3+1], vertex[i1*3+2],
                               normal[i1*3], normal[i1*3+1], normal[i1*3+2],
                               color[i1*4], color[i1*4+1], color[i1*4+2], color[i1*4+3],
                               texcoord[i1*2], texcoord[i1*2+1]),
                t2 = addVertex(vertex[i2*3], vertex[i2*3+1], vertex[i2*3+2],
                               normal[i2*3], normal[i2*3+1], normal[i2*3+2],
                               color[i2*4], color[i2*4+1], color[i2*4+2], color[i2*4+3],
                               texcoord[i2*2], texcoord[i2*2+1]),
                t3 = addVertex(vertex[i3*3], vertex[i3*3+1], vertex[i3*3+2],
                               normal[i3*3], normal[i3*3+1], normal[i3*3+2],
                               color[i3*4], color[i3*4+1], color[i3*4+2], color[i3*4+3],
                               texcoord[i3*2], texcoord[i3*2+1]);

            // .. create a new unique triangle
            addTriangle(t1, t2, t3);
        }
    }
    else
    {
        auto index = lineIndex_;
        lineIndex_.clear();

        // for each previous line ..
        for (uint i=0; i<index.size() / 2; ++i)
        {
            IndexType
                i1 = index[i*2],
                i2 = index[i*2+1],

                t1 = addVertex(vertex[i1*3], vertex[i1*3+1], vertex[i1*3+2],
                               normal[i1*3], normal[i1*3+1], normal[i1*3+2],
                               color[i1*4], color[i1*4+1], color[i1*4+2], color[i1*4+3],
                               texcoord[i1*2], texcoord[i1*2+1]),
                t2 = addVertex(vertex[i2*3], vertex[i2*3+1], vertex[i2*3+2],
                               normal[i2*3], normal[i2*3+1], normal[i2*3+2],
                               color[i2*4], color[i2*4+1], color[i2*4+2], color[i2*4+3],
                               texcoord[i2*2], texcoord[i2*2+1]);

            // .. create a new unique triangle
            addLine(t1, t2);
        }
    }
}

void Geometry::convertToLines()
{
    if (!numTriangles())
        return;

    lineIndex_.clear();

    // test for already-connected
    typedef long unsigned int Hash;
    QSet<Hash> hash;

    for (uint i=0; i<numTriangles(); ++i)
    {
        const IndexType
                  t1 = triIndex_[i*3],
                  t2 = triIndex_[i*3+1],
                  t3 = triIndex_[i*3+2];

        const Hash
                h1  = Hash(t1) | (Hash(t2) << 32),
                h1i = Hash(t2) | (Hash(t1) << 32),
                h2  = Hash(t1) | (Hash(t3) << 32),
                h2i = Hash(t3) | (Hash(t1) << 32),
                h3  = Hash(t2) | (Hash(t3) << 32),
                h3i = Hash(t3) | (Hash(t2) << 32);
        //Hash h1 = MATH::getHashUnordered<Hash>(t1, t2),
        //     h2 = MATH::getHashUnordered<Hash>(t1, t3),
        //     h3 = MATH::getHashUnordered<Hash>(t2, t3);

        if (!hash.contains(h1) && !hash.contains(h1i))
        {
            addLine(t1, t2);
            hash.insert(h1);
        }
        if (!hash.contains(h2) && !hash.contains(h2i))
        {
            addLine(t1, t3);
            hash.insert(h2);
        }
        if (!hash.contains(h3) && !hash.contains(h3i))
        {
            addLine(t2, t3);
            hash.insert(h3);
        }
    }

    triIndex_.clear();
}

void Geometry::normalizeSphere(VertexType scale, VertexType normalization)
{
    for (uint i=0; i<numVertices(); ++i)
    {
        const VertexType
            v1 = vertex_[i*3],
            v2 = vertex_[i*3+1],
            v3 = vertex_[i*3+2],
            mag = std::sqrt(v1*v1 + v2*v2 + v3*v3) / scale;

        if (mag>0 && normalization >= 1)
        {
            vertex_[i*3] /= mag;
            vertex_[i*3+1] /= mag;
            vertex_[i*3+2] /= mag;
        }
        else
        {
            vertex_[i*3] += normalization * (vertex_[i*3] / mag - vertex_[i*3]);
            vertex_[i*3+1] += normalization * (vertex_[i*3+1] / mag - vertex_[i*3+1]);
            vertex_[i*3+2] += normalization * (vertex_[i*3+2] / mag - vertex_[i*3+2]);
        }
    }
}

void Geometry::tesselate(uint level)
{
    // XXX TODO: vertex reuse and color/texcoord handling

    if (!numTriangles())
    {
        Geometry tess;

        level = std::pow(2,level);

        for (uint i=0; i<numLines(); ++i)
        {
            const IndexType
                    t1 = lineIndex_[i*2],
                    t2 = lineIndex_[i*2+1];

            const Vec3
                    p1 = getVertex(t1),
                    p2 = getVertex(t2);

            std::vector<IndexType> n;
            n.push_back(tess.addVertex(p1[0], p1[1], p1[2]));
            for (uint l = 0; l<level; ++l)
            {
                Vec3 p12 = p1 + (p2 - p1) * (float(l+1) / (level+1));
                n.push_back(tess.addVertex(p12[0], p12[1], p12[2]));
            }
            n.push_back(tess.addVertex(p2[0], p2[1], p2[2]));

            for (uint l = 1; l<n.size(); ++l)
                tess.addLine(n[l-1], n[l]);
        }

        *this = tess;
    }

    else
    // tesselate triangles
    for (uint l = 0; l<level; ++l)
    {
        Geometry tess;

        for (uint i=0; i<numTriangles(); ++i)
        {
            const IndexType
                    t1 = triIndex_[i*3],
                    t2 = triIndex_[i*3+1],
                    t3 = triIndex_[i*3+2];

            const Vec3
                    p1 = getVertex(t1),
                    p2 = getVertex(t2),
                    p3 = getVertex(t3),
                    p12 = 0.5f * (p1 + p2),
                    p13 = 0.5f * (p1 + p3),
                    p23 = 0.5f * (p2 + p3),

                    pn1 = getNormal(t1),
                    pn2 = getNormal(t2),
                    pn3 = getNormal(t3),
                    pn12 = 0.5f * (pn1 + pn2),
                    pn13 = 0.5f * (pn1 + pn3),
                    pn23 = 0.5f * (pn2 + pn3);
            const Vec4
                    pc1 = getColor(t1),
                    pc2 = getColor(t2),
                    pc3 = getColor(t3),
                    pc12 = 0.5f * (pc1 + pc2),
                    pc13 = 0.5f * (pc1 + pc3),
                    pc23 = 0.5f * (pc2 + pc3);
            const Vec2
                    pt1 = getTexCoord(t1),
                    pt2 = getTexCoord(t2),
                    pt3 = getTexCoord(t3),
                    pt12 = 0.5f * (pt1 + pt2),
                    pt13 = 0.5f * (pt1 + pt3),
                    pt23 = 0.5f * (pt2 + pt3);


            const IndexType
                    n1 = tess.addVertex(p1[0], p1[1], p1[2], pn1[0], pn1[1], pn1[2], pc1[0], pc1[1], pc1[2], pc1[3], pt1[0], pt1[1]),
                    n2 = tess.addVertex(p2[0], p2[1], p2[2], pn2[0], pn2[1], pn2[2], pc2[0], pc2[1], pc2[2], pc2[3], pt2[0], pt2[1]),
                    n3 = tess.addVertex(p3[0], p3[1], p3[2], pn3[0], pn3[1], pn3[2], pc3[0], pc3[1], pc3[2], pc3[3], pt3[0], pt3[1]),
                    n12 = tess.addVertex(p12[0], p12[1], p12[2], pn12[0], pn12[1], pn12[2], pc12[0], pc12[1], pc12[2], pc12[3], pt12[0], pt12[1]),
                    n13 = tess.addVertex(p13[0], p13[1], p13[2], pn13[0], pn13[1], pn13[2], pc13[0], pc13[1], pc13[2], pc13[3], pt13[0], pt13[1]),
                    n23 = tess.addVertex(p23[0], p23[1], p23[2], pn23[0], pn23[1], pn23[2], pc23[0], pc23[1], pc23[2], pc23[3], pt23[0], pt23[1]);

            tess.addTriangle(n1, n12, n13);
            tess.addTriangle(n12, n2, n23);
            tess.addTriangle(n12, n23, n13);
            tess.addTriangle(n13, n23, n3);
        }

        *this = tess;
    }
}

void Geometry::removePrimitivesRandomly(float probability, int seed)
{
    std::mt19937 rnd(seed);

    if (numTriangles())
    {
        std::vector<IndexType> index;

        for (uint i=0; i<triIndex_.size(); i += 3)
        {
            if ((float)rnd() / rnd.max() >= probability)
            {
                index.push_back(triIndex_[i]);
                index.push_back(triIndex_[i+1]);
                index.push_back(triIndex_[i+2]);
            }
        }

        triIndex_ = index;
    }
    else
    {
        std::vector<IndexType> index;

        for (uint i=0; i<lineIndex_.size(); i += 2)
        {
            if ((float)rnd() / rnd.max() >= probability)
            {
                index.push_back(lineIndex_[i]);
                index.push_back(lineIndex_[i+1]);
            }
        }

        lineIndex_ = index;
    }
}

void Geometry::transformWithNoise(
        VertexType modX, VertexType modY, VertexType modZ,
        VertexType scaleX, VertexType scaleY, VertexType scaleZ,
        int seedX, int seedY, int seedZ)
{
    MATH::NoisePerlin nx(seedX), ny(seedY), nz(seedZ);

    for (uint i=0; i<vertex_.size(); i+=numVertexComponents())
    {
        VertexType
                x = vertex_[i] * scaleX,
                y = vertex_[i+1] * scaleY,
                z = vertex_[i+2] * scaleZ;

        vertex_[i] += modX * nx.noise(x);
        vertex_[i+1] += modY * nx.noise(y);
        vertex_[i+2] += modZ * nx.noise(z);
    }
}








void Geometry::getVertexArrayObject(GL::VertexArrayObject * vao, GL::Shader * s, bool triangles)
{
    if (vao->isCreated())
        vao->release();

    vao->create();
    vao->bind();

    if (!s->getAttribute(s->source()->attribNamePosition()))
        MO_GL_ERROR("No position attribute in shader");

    // --- position ---
    if (auto a = s->getAttribute(s->source()->attribNamePosition()))
    {
        vao->createAttribBuffer(a->location(),
                                vertexEnum, numVertexComponents(),
                                numVertexBytes(),
                                vertices());
    }

    // --- color ---
    if (auto a = s->getAttribute(s->source()->attribNameColor()))
    {
        vao->createAttribBuffer(a->location(),
                                colorEnum, numColorComponents(),
                                numColorBytes(),
                                colors());
    }

    // --- normal ---
    if (auto a = s->getAttribute(s->source()->attribNameNormal()))
    {
        vao->createAttribBuffer(a->location(),
                                normalEnum, numNormalComponents(),
                                numNormalBytes(),
                                normals());
    }

    // --- texcoord ---
    if (auto a = s->getAttribute(s->source()->attribNameTexCoord()))
    {
        vao->createAttribBuffer(a->location(),
                                textureCoordEnum, numTextureCoordComponents(),
                                numTextureCoordBytes(),
                                textureCoords());
    }

    // --- indices ---
    if (triangles)
        vao->createIndexBuffer(indexEnum,
                               numTriangles() * numTriangleIndexComponents(),
                               triangleIndices());
    else
        vao->createIndexBuffer(indexEnum,
                               numLines() * numLineIndexComponents(),
                               lineIndices());

    vao->unbind();
}




} // namespace GEOM
} // namespace MO
