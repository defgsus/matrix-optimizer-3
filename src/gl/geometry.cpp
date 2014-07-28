/** @file model.cpp

    @brief Geometry container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include <QSet>

#include "geometry.h"
#include "shadersource.h"
#include "shader.h"
#include "vertexarrayobject.h"
#include "math/hash.h"

namespace MO {
namespace GL {

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
    auto index = triIndex_;

    vertex_.clear();
    normal_.clear();
    color_.clear();
    triIndex_.clear();
    texcoord_.clear();

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

        Hash h1 = MATH::getHash<Hash>(t1, t2),
             h2 = MATH::getHash<Hash>(t1, t3),
             h3 = MATH::getHash<Hash>(t2, t3);

        if (!hash.contains(h1))
        {
            addLine(t1, t2);
            hash.insert(h1);
        }
        if (!hash.contains(h2))
        {
            addLine(t1, t3);
            hash.insert(h2);
        }
        if (!hash.contains(h3))
        {
            addLine(t2, t3);
            hash.insert(h3);
        }
    }

    triIndex_.clear();
}


void Geometry::getVertexArrayObject(VertexArrayObject * vao, Shader * s, bool triangles)
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




} // namespace GL
} // namespace MO
