/** @file geometry.cpp

    @brief Geometry container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include <random>
#include <atomic>

#include <QSet>
#include <QTextStream>

#include "geometry.h"
#include "gl/shadersource.h"
#include "gl/shader.h"
#include "gl/vertexarrayobject.h"
#include "math/hash.h"
#include "math/noiseperlin.h"
#include "math/funcparser/parser.h"
#include "math/constants.h"
#include "math/functions.h"
#include "math/vector.h"
#include "math/intersection.h"
#include "tool/stringmanip.h"
#include "io/log.h"

using namespace gl;

namespace MO {
namespace GEOM {

const Geometry::VertexType Geometry::minimumThreshold = 0.001;

const GLenum Geometry::vertexEnum       = GL_FLOAT;
const GLenum Geometry::normalEnum       = GL_FLOAT;
const GLenum Geometry::colorEnum        = GL_FLOAT;
const GLenum Geometry::textureCoordEnum = GL_FLOAT;
const GLenum Geometry::indexEnum        = GL_UNSIGNED_INT;
const GLenum Geometry::attributeEnum    = GL_FLOAT;


namespace
{
    /** Helper to keep Geometry::hash() unique */
    static std::atomic_int geom_hash_;


    // Some halbausgerorener stuff to build
    // a std::map from vertex positions

    struct Hash2
    {
        Float x,y;
        Hash2(Float x, Float y) : x(x), y(y) { }

        bool operator < (const Hash2& r) const
        {
            if (y == r.y)
                return x < r.x;
            return y < r.y;
        }
    };

    typedef QMap<Hash2, uint> Map2;

    struct Hash3
    {
        Float x,y,z;
        Hash3(Float x, Float y, Float z) : x(x), y(y), z(z) { }

        bool operator < (const Hash3& r) const
        {
            if (z == r.z)
            {
                if (y == r.y)
                    return x < r.x;
                return y < r.y;
            }
            return z < r.z;
        }
    };

    typedef QMap<Hash3, uint> Map3;



    // ---- other hashing stuff -----

    typedef quint64 Key_;

    struct MapStruct_
    {
        Geometry::IndexType idx;
        uint count;
        MapStruct_(Geometry::IndexType idx) : idx(idx), count(1) { }
    };

}

struct Geometry::Private
{
    Private(Geometry* p)
        : p     (p)
        , p_hash_   (geom_hash_++)

    { }

    void clearPrimitiveHash()
    {
        pointMap_.clear();
        lineMap_.clear();
        triMap_.clear();
    }

    Geometry* p;

    std::map<Key_, MapStruct_> indexMap_;
    std::set<Key_> pointMap_, lineMap_, triMap_;

    bool sharedVertices_;

    int p_hash_;
};




Geometry::Geometry()
    : RefCounted("Geometry")
    , curR_   (1.f)
    , curG_   (1.f)
    , curB_   (1.f)
    , curA_   (1.f)
    , curNx_  (0.f)
    , curNy_  (0.f)
    , curNz_  (1.f)
    , curU_   (0.f)
    , curV_   (0.f)
#ifndef MO_DISABLE_EDGEFLAG
    , curEdge_(gl::GL_TRUE)
#endif
    , sharedVertices_ (false)
    , p_shareThreshold_      (minimumThreshold)
    , p_      (new Private(this))
{
    //MO_PRINT("Geometry::Geometry()");
}

Geometry::Geometry(const Geometry& other)
    : RefCounted("Geometry(copy)")
    , p_    (new Private(this))
{
    copyFrom(other);
}


Geometry::~Geometry()
{
    //MO_PRINT("Geometry::~Geometry()");
    clear();
    delete p_;
}

void Geometry::setChanged()
{
    p_->p_hash_ = geom_hash_++;
}

bool Geometry::isEmpty() const
{
    return (!numVertices()
       || !(numPoints() || numLines() || numTriangles()));
}

int Geometry::hash() const
{
    return p_->p_hash_;
}

QString Geometry::infoString() const
{
    QString s = QObject::tr("%1 vertices (%2)")
            .arg(numVertices()).arg(sharedVertices() ? "shared" : "unshared");
    if (numTriangles())
        s += QObject::tr(", %1 triangles").arg(numTriangles());
    if (numLines())
        s += QObject::tr(", %1 lines").arg(numLines());
    if (numPoints())
        s += QObject::tr(", %1 points").arg(numPoints());

    s += QObject::tr(", memory: %1").arg(byte_to_string(memory()));

    return s;
}

void Geometry::clear()
{
    setChanged();

    vertex_.clear();
    normal_.clear();
    color_.clear();
    texcoord_.clear();
    triIndex_.clear();
    lineIndex_.clear();
    pointIndex_.clear();
    for (auto i : attributes_)
        delete i.second;
    attributes_.clear();

    progress_ = 0;
    curR_ = curG_ = curB_ = curA_ = 1.;
    curNx_ = curNy_ = 0.;
    curNz_ = 1.;
    curU_ = curV_ = 0.;
#ifndef MO_DISABLE_EDGEFLAG
    curEdge_ = gl::GL_TRUE;
#endif

    sharedVertices_ = false;
    p_shareThreshold_ = minimumThreshold;
    p_->indexMap_.clear();
    p_->clearPrimitiveHash();
}

void Geometry::copyFrom(const Geometry &o)
{
    clear();

    curR_ = o.curR_;
    curG_ = o.curG_;
    curB_ = o.curB_;
    curA_ = o.curA_;
    curNx_ = o.curNx_;
    curNy_ = o.curNy_;
    curNz_ = o.curNz_;
    curU_ = o.curU_;
    curV_ = o.curV_;
#ifndef MO_DISABLE_EDGEFLAG
    curEdge_ = o.curEdge_;
#endif

    vertex_ = o.vertex_;
    normal_ = o.normal_;
    color_ = o.color_;
    texcoord_ = o.texcoord_;
    triIndex_ = o.triIndex_;
    lineIndex_ = o.lineIndex_;
    pointIndex_ = o.pointIndex_;

    sharedVertices_ = o.sharedVertices_;
    p_shareThreshold_ = o.p_shareThreshold_;
    p_->indexMap_ = o.p_->indexMap_;
    p_->indexMap_ = o.p_->indexMap_;
    p_->pointMap_ = o.p_->pointMap_;
    p_->lineMap_ = o.p_->lineMap_;
    p_->triMap_ = o.p_->triMap_;

    for (auto i : o.attributes_)
        attributes_.insert( std::make_pair(i.first, new UserAttribute(*i.second)) );
}

Geometry * Geometry::createFrom(const QList<const Geometry*>& list)
{
    auto g = new Geometry();

    if (list.isEmpty())
        return g;

    // copy the first
    g->copyFrom(*list.front());

    // add the others
    for (int i = 1; i < list.size(); ++i)
        g->addGeometry(*list[i]);

    return g;
}

// ---------------- attributes --------------

Geometry::UserAttribute * Geometry::getAttribute(const QString &name)
{
    auto i = attributes_.find(name);
    if (i == attributes_.end())
        return 0;
    return i->second;
}

const Geometry::UserAttribute * Geometry::getAttribute(const QString &name) const
{
    const auto i = attributes_.find(name);
    if (i == attributes_.end())
        return 0;
    return i->second;
}

const Geometry::AttributeType * Geometry::attributes(const QString& name) const
{
    auto a = getAttribute(name);
    return a ? &a->data[0] : 0;
}

QString Geometry::UserAttribute::typeName() const
{
    switch (numComponents)
    {
        default: return "float";
        case 2: return "vec2";
        case 3: return "vec3";
        case 4: return "vec4";
    }
}

QString Geometry::UserAttribute::declaration() const
{
    return "in " + typeName() + " " + attributeName + ";\n";
}

Geometry::AttributeType Geometry::UserAttribute::value(
        const IndexType index, const IndexType componentIndex) const
{
    const auto i = index * numComponents + componentIndex;
    return i < data.size() ? data[i] : 0.;
}

QStringList Geometry::getAttributeNames() const
{
    QStringList list;
    for (auto i : attributes_)
        list << i.first;
    return list;
}

Geometry::UserAttribute * Geometry::addAttribute(const QString &name, unsigned int numComponents)
{
    setChanged();

    auto a = getAttribute(name);
    if (!a)
    {
        a = new UserAttribute(name, numComponents);
        attributes_.insert( std::make_pair(name, a) );
    }

    if (a->numComponents != numComponents)
    {
        a->numComponents = numComponents;
        a->curValue.resize(numComponents, 0.f);
    }

    // reserve as much data as currently needed
    a->data.resize(numVertices() * a->numComponents);

    return a;
}

void Geometry::setAttribute(const QString &name, AttributeType x)
{
    setChanged();

    auto a = getAttribute(name);
    if (!a)
        a = addAttribute(name, 1);

    a->curValue[0] = x;
}

void Geometry::setAttribute(const QString &name, AttributeType x, AttributeType y)
{
    setChanged();

    auto a = getAttribute(name);
    if (!a)
        a = addAttribute(name, 2);

    a->curValue[0] = x;
    if (a->curValue.size() >= 2)
        a->curValue[1] = y;
}

void Geometry::setAttribute(const QString &name, AttributeType x, AttributeType y, AttributeType z)
{
    setChanged();

    auto a = getAttribute(name);
    if (!a)
        a = addAttribute(name, 3);

    a->curValue[0] = x;
    if (a->curValue.size() >= 2)
        a->curValue[1] = y;
    if (a->curValue.size() >= 3)
        a->curValue[2] = z;
}

void Geometry::setAttribute(const QString &name, AttributeType x, AttributeType y, AttributeType z, AttributeType w)
{
    setChanged();

    auto a = getAttribute(name);
    if (!a)
        a = addAttribute(name, 4);

    a->curValue[0] = x;
    if (a->curValue.size() >= 2)
        a->curValue[1] = y;
    if (a->curValue.size() >= 3)
        a->curValue[2] = z;
    if (a->curValue.size() >= 4)
        a->curValue[3] = w;
}

Geometry::UserAttribute * Geometry::addEnumerationAttribute(const QString &name)
{
    setChanged();

    auto a = addAttribute(name, 4);

    for (size_t i=0; i<numVertices(); ++i)
    {
        a->data[i * 4] = i;
        a->data[i * 4 + 1] = 0.f;
        a->data[i * 4 + 2] = 0.f;
        a->data[i * 4 + 3] = 0.f;
    }

    for (size_t i=0; i<numLines(); ++i)
    {
        a->data[lineIndex_[i * 2 + 0] * 4 + 1] = i;
        a->data[lineIndex_[i * 2 + 1] * 4 + 1] = i;
    }

    for (size_t i=0; i<numTriangles(); ++i)
    {
        a->data[triIndex_[i * 3 + 0] * 4 + 2] = i;
        a->data[triIndex_[i * 3 + 1] * 4 + 2] = i;
        a->data[triIndex_[i * 3 + 2] * 4 + 2] = i;
    }
    return a;
}



long unsigned int Geometry::memory() const
{
    long unsigned int bytes =
              numVertexBytes()
            + numNormalBytes()
            + numColorBytes()
            + numTextureCoordBytes()
            + numTriangleIndexBytes()
            + numLineIndexBytes()
            + numPointIndexBytes();

    for (auto i : attributes_)
        bytes += i.second->data.size() * sizeof(AttributeType);

    return bytes;
}

void Geometry::getExtent(VertexType * minX, VertexType * maxX,
                         VertexType * minY, VertexType * maxY,
                         VertexType * minZ, VertexType * maxZ) const
{
    Vec3 mi, ma;
    getExtent(&mi, &ma);
    *minX = mi[0];
    *minY = mi[1];
    *minZ = mi[2];
    *maxX = ma[0];
    *maxY = ma[1];
    *maxZ = ma[2];
}

void Geometry::getExtent(Vec3 * minimum, Vec3 * maximum) const
{
    if (numVertices() < 1)
    {
        *minimum = *maximum = Vec3(0,0,0);
        return;
    }

    *minimum = *maximum = getVertex(0);

    for (uint i=1; i<numVertices(); ++i)
    {
        *minimum = glm::min(*minimum, getVertex(i));
        *maximum = glm::max(*maximum, getVertex(i));
    }
}

bool Geometry::intersects_any(const Vec3 &ray_origin, const Vec3 &ray_direction, Vec3 *pos) const
{
    // XXX todo: Test against bounding-box first!

    for (uint i=0; i<numTriangles(); ++i)
    {
        const Vec3 t0 = getVertex(triIndex_[i * numTriangleIndexComponents()]),
                   t1 = getVertex(triIndex_[i * numTriangleIndexComponents() + 1]),
                   t2 = getVertex(triIndex_[i * numTriangleIndexComponents() + 2]);

        if (MATH::intersect_ray_triangle(ray_origin, ray_direction,
                                         t0, t1, t2, pos))
            return true;
    }

    return false;
}

bool Geometry::intersects(const Vec3 &ray_origin, const Vec3 &ray_direction, Vec3 *outpos) const
{
    // XXX todo: Test against bounding-box first!

    Float closest = -1;
    Vec3 pos;

    for (uint i=0; i<numTriangles(); ++i)
    {
        const Vec3 t0 = getVertex(triIndex_[i * numTriangleIndexComponents()]),
                   t1 = getVertex(triIndex_[i * numTriangleIndexComponents() + 1]),
                   t2 = getVertex(triIndex_[i * numTriangleIndexComponents() + 2]);

        if (MATH::intersect_ray_triangle(ray_origin, ray_direction,
                                         t0, t1, t2, &pos))
        {
            // no need to look further, outpos is not used
            if (!outpos)
                return true;

            Float dist = glm::distance(ray_origin, pos);
            if (dist < closest || closest < 0)
            {
                closest = dist;
                *outpos = pos;
            }
        }
    }

    return closest >= 0;
}



void Geometry::setSharedVertices(bool enable, VertexType threshold)
{
    setChanged();

    sharedVertices_ = enable;
    p_shareThreshold_ = std::max(minimumThreshold, threshold);
    if (!enable)
    {
        std::map<Key_, MapStruct_> tmp;
        p_->indexMap_.swap(tmp); // the seriously-clear method
    }

}

bool Geometry::checkTriangle(const Vec3 & a, const Vec3 & b, const Vec3 & c)
{
    const Float small = 0.00001;
    return !(glm::distance(a, b) < small
          || glm::distance(a, c) < small
          || glm::distance(b, c) < small
           );
}

Geometry::IndexType Geometry::addVertex(
                VertexType x, VertexType y, VertexType z,
                NormalType nx, NormalType ny, NormalType nz,
                ColorType r, ColorType g, ColorType b, ColorType a,
                TextureCoordType u, TextureCoordType v)
{    
    if (!sharedVertices_)
    {
        return addVertexAlways(x,y,z,nx,ny,nz,r,g,b,a,u,v);
    }

    setChanged();

#define MO__MAKE_KEY(x__, y__, z__)  \
    (  (Key_((x__)/p_shareThreshold_) & ((1<<23) - 1)) \
    | ((Key_((y__)/p_shareThreshold_) & ((1<<23) - 1)) << 24) \
    | ((Key_((z__)/p_shareThreshold_) & ((1<<23) - 1)) << 48) )

    // find vertex in range
    const Key_ key = MO__MAKE_KEY(x,y,z);
    auto i = p_->indexMap_.find(key);

    // add new
    if (i == p_->indexMap_.end())
    {
        const IndexType idx = addVertexAlways(x,y,z,nx,ny,nz,r,g,b,a,u,v);
        p_->indexMap_.insert(std::make_pair(key, MapStruct_(idx)));
        return idx;
    }

    // reuse
    const IndexType idx = i->second.idx;
    i->second.count++;
    const float
            m2 = 1.f / i->second.count,
            m1 = 1.f - m2;

    // average attributes
    NormalType * norm = &normal_[idx * numNormalComponents()];
    *norm = m1 * *norm + m2 * nx; ++norm;
    *norm = m1 * *norm + m2 * ny; ++norm;
    *norm = m1 * *norm + m2 * nz;

    ColorType * col = &color_[idx * numColorComponents()];
    *col = m1 * *col + m2 * r; ++col;
    *col = m1 * *col + m2 * g; ++col;
    *col = m1 * *col + m2 * b; ++col;
    *col = m1 * *col + m2 * a;

    TextureCoordType * tex = &texcoord_[idx * numTextureCoordComponents()];
    *tex = m1 * *tex + m2 * u; ++tex;
    *tex = m1 * *tex + m2 * v;

    for (auto i : attributes_)
    {
        UserAttribute * ua = i.second;
        AttributeType * a = &ua->data[idx * ua->numComponents];
        for (uint j=0; j<ua->numComponents; ++j)
            a[j] = m1 * a[j] + m2 * ua->curValue[j];
    }

    return idx;
}

Geometry::IndexType Geometry::findVertex(VertexType x, VertexType y, VertexType z) const
{
    const Key_ key = MO__MAKE_KEY(x,y,z);
    auto i = p_->indexMap_.find(key);
    return i == p_->indexMap_.end() ? invalidIndex : i->second.idx;
#undef MO__MAKE_KEY
}


Geometry::IndexType Geometry::addVertexAlways(
                VertexType x, VertexType y, VertexType z,
                NormalType nx, NormalType ny, NormalType nz,
                ColorType r, ColorType g, ColorType b, ColorType a,
                TextureCoordType u, TextureCoordType v)
{
    setChanged();

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

    for (auto i : attributes_)
    {
        UserAttribute * a = i.second;
        for (uint j = 0; j < a->numComponents; ++j)
            a->data.push_back(a->curValue[j]);
    }

    return numVertices() - 1;
}

Geometry::IndexType Geometry::duplicateVertex(IndexType t)
{
    if (sharedVertices())
        return t;

    setChanged();

    const Vec3 v = getVertex(t),
               n = getNormal(t);
    const Vec4 c = getColor(t);
    const Vec2 tx = getTexCoord(t);

    const auto idx = addVertexAlways(
                v.x, v.y, v.z,
                n.x, n.y, n.z,
                c.x, c.y, c.z, c.w,
                tx.x, tx.y);
    // duplicate user attributes
    for (const auto & i : attributes_)
    {
        UserAttribute * a = i.second;
        setAttribute(a->attributeName, idx,
                     a->value(t, 0), a->value(t, 1), a->value(t, 2), a->value(t, 3));
    }

    return idx;
}


void Geometry::setAttribute(const QString &name, IndexType idx,
                            AttributeType x, AttributeType y, AttributeType z, AttributeType w)
{
    setChanged();

    auto i = attributes_.find(name);
    if (i == attributes_.end())
        return;

    UserAttribute * a = i->second;
    if (a->data.size() < idx * a->numComponents)
        a->data.resize(idx * a->numComponents);
    if (a->numComponents >= 1)
        a->data[idx * a->numComponents] = x;
    if (a->numComponents >= 2)
        a->data[idx * a->numComponents + 1] = y;
    if (a->numComponents >= 3)
        a->data[idx * a->numComponents + 2] = z;
    if (a->numComponents >= 4)
        a->data[idx * a->numComponents + 3] = w;
}

void Geometry::addTriangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3)
{
    if (checkTriangle(p1, p2, p3))
    {        
        auto i1 = addVertex(p1.x, p1.y, p1.z),
             i2 = addVertex(p2.x, p2.y, p2.z),
             i3 = addVertex(p3.x, p3.y, p3.z);
        addTriangle(i1, i2, i3);
    }
}

void Geometry::addTriangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
                           const Vec2& tex1, const Vec2& tex2, const Vec2& tex3)
{
    if (checkTriangle(p1, p2, p3))
    {
        setTexCoord(tex1);
        auto i1 = addVertex(p1.x, p1.y, p1.z);
        setTexCoord(tex2);
        auto i2 = addVertex(p2.x, p2.y, p2.z);
        setTexCoord(tex3);
        auto i3 = addVertex(p3.x, p3.y, p3.z);
        addTriangle(i1, i2, i3);
    }
}

#ifndef MO_DISABLE_EDGEFLAG
void Geometry::addTriangle(IndexType p1, IndexType p2, IndexType p3,
                           bool edge1, bool edge2, bool edge3)
{
    addTriangle(p1, p2, p3,
                edge1 ? gl::GL_TRUE : gl::GL_FALSE,
                edge2 ? gl::GL_TRUE : gl::GL_FALSE,
                edge3 ? gl::GL_TRUE : gl::GL_FALSE);
}

void Geometry::addTriangle(IndexType p1, IndexType p2, IndexType p3,
                           EdgeFlagType edge1, EdgeFlagType edge2, EdgeFlagType edge3)
{
    MO_ASSERT(p1 < numVertices(), "triangle index #1 out of range " << p1 << "/" << numVertices());
    MO_ASSERT(p2 < numVertices(), "triangle index #2 out of range " << p2 << "/" << numVertices());
    MO_ASSERT(p3 < numVertices(), "triangle index #3 out of range " << p3 << "/" << numVertices());
    setChanged();
    triIndex_.push_back(p1);
    triIndex_.push_back(p2);
    triIndex_.push_back(p3);
    edgeFlags_.push_back(edge1);
    edgeFlags_.push_back(edge2);
    edgeFlags_.push_back(edge3);
}
#endif

void Geometry::addTriangle(IndexType p1, IndexType p2, IndexType p3)
{
    MO_ASSERT(p1 < numVertices(), "triangle index #1 out of range " << p1 << "/" << numVertices());
    MO_ASSERT(p2 < numVertices(), "triangle index #2 out of range " << p2 << "/" << numVertices());
    MO_ASSERT(p3 < numVertices(), "triangle index #3 out of range " << p3 << "/" << numVertices());

    uint64_t hash = MATH::getHash<uint64_t>(p1, p2, p3);
    if (p_->triMap_.find(hash) != p_->triMap_.end())
    {
        for (size_t i=0; i<triIndex_.size(); i += 3)
            if (p1 == triIndex_[i] && p2 == triIndex_[i+1] && p3 == triIndex_[i+2])
            return;
    }
    p_->triMap_.insert(hash);

    setChanged();
    triIndex_.push_back(p1);
    triIndex_.push_back(p2);
    triIndex_.push_back(p3);
#ifndef MO_DISABLE_EDGEFLAG
        edgeFlags_.push_back(curEdge_);
        edgeFlags_.push_back(curEdge_);
        edgeFlags_.push_back(curEdge_);
#endif
}

void Geometry::addTriangleChecked(IndexType p1, IndexType p2, IndexType p3)
{
    MO_ASSERT(p1 < numVertices(), "triangle index #1 out of range " << p1 << "/" << numVertices());
    MO_ASSERT(p2 < numVertices(), "triangle index #2 out of range " << p2 << "/" << numVertices());
    MO_ASSERT(p3 < numVertices(), "triangle index #3 out of range " << p3 << "/" << numVertices());
    const Vec3
            pos1 = getVertex(p1),
            pos2 = getVertex(p2),
            pos3 = getVertex(p3);
    if (checkTriangle(pos1, pos2, pos3))
    {
        uint64_t hash = MATH::getHash<uint64_t>(p1, p2, p3);
        if (p_->triMap_.find(hash) != p_->triMap_.end())
        {
            for (size_t i=0; i<triIndex_.size(); i += 3)
                if (p1 == triIndex_[i] && p2 == triIndex_[i+1] && p3 == triIndex_[i+2])
                return;
        }
        p_->triMap_.insert(hash);

        setChanged();
        triIndex_.push_back(p1);
        triIndex_.push_back(p2);
        triIndex_.push_back(p3);
#ifndef MO_DISABLE_EDGEFLAG
        edgeFlags_.push_back(curEdge_);
        edgeFlags_.push_back(curEdge_);
        edgeFlags_.push_back(curEdge_);
#endif
    }
}

void Geometry::addQuad(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, const Vec3 &p4)
{
    if (checkTriangle(p1, p2, p3) && checkTriangle(p1, p3, p4))
    {
        auto i1 = addVertex(p1.x, p1.y, p1.z),
             i2 = addVertex(p2.x, p2.y, p2.z),
             i3 = addVertex(p3.x, p3.y, p3.z),
             i4 = addVertex(p4.x, p4.y, p4.z);
        addTriangle(i1, i2, i3);
        addTriangle(i1, i3, i4);
    }
}

void Geometry::addQuad(IndexType p1, IndexType p2, IndexType p3, IndexType p4)
{
    addTriangleChecked(p1, p2, p3);
    addTriangleChecked(p1, p3, p4);
}

void Geometry::addLine(IndexType p1, IndexType p2)
{
    MO_ASSERT(p1 < numVertices(), "line index #1 out of range " << p1 << "/" << numVertices());
    MO_ASSERT(p2 < numVertices(), "line index #2 out of range " << p2 << "/" << numVertices());

    uint64_t hash = MATH::getHashUnordered<uint64_t>(p1, p2);
    if (p_->lineMap_.find(hash) != p_->lineMap_.end())
    {
        for (size_t i=0; i<lineIndex_.size(); i += 2)
            if ((p1 == lineIndex_[i] && p2 == lineIndex_[i+1])
             || (p1 == lineIndex_[i+1] && p2 == lineIndex_[i]))
            return;
    }
    p_->lineMap_.insert(hash);

    setChanged();
    lineIndex_.push_back(p1);
    lineIndex_.push_back(p2);
}

void Geometry::addLine(const Vec3 &p1, const Vec3 &p2)
{
    auto i1 = addVertex(p1.x, p1.y, p1.z),
         i2 = addVertex(p2.x, p2.y, p2.z);
    addLine(i1, i2);
}

void Geometry::addPoint(IndexType p1)
{
    MO_ASSERT(p1 < numVertices(), "point index out of range " << p1 << "/" << numVertices());

    uint64_t hash = p1;
    if (p_->pointMap_.find(hash) != p_->pointMap_.end())
        return;
    p_->pointMap_.insert(hash);

    setChanged();
    pointIndex_.push_back(p1);
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

const Geometry::VertexType * Geometry::point(IndexType pointIndex) const
{
    return &vertex_[pointIndex_[pointIndex] * numVertexComponents()];
}

void Geometry::addGeometry(const Geometry &other, const Vec3& offset)
{
    /** @todo Geometry::addGeometry() doesn't copy attributes! */
    setChanged();

    // copy triangles
    for (uint i=0; i<other.numTriangles(); ++i)
    {
        IndexType
                ot1 = other.triIndex_[i * 3],
                ot2 = other.triIndex_[i * 3 + 1],
                ot3 = other.triIndex_[i * 3 + 2],

                t1 = addVertex(
                    other.vertex_[ot1 * other.numVertexComponents()] + offset.x,
                    other.vertex_[ot1 * other.numVertexComponents() + 1] + offset.y,
                    other.vertex_[ot1 * other.numVertexComponents() + 2] + offset.z,
                    other.normal_[ot1 * other.numNormalComponents()],
                    other.normal_[ot1 * other.numNormalComponents() + 1],
                    other.normal_[ot1 * other.numNormalComponents() + 2],
                    other.color_[ot1 * other.numColorComponents()],
                    other.color_[ot1 * other.numColorComponents() + 1],
                    other.color_[ot1 * other.numColorComponents() + 2],
                    other.color_[ot1 * other.numColorComponents() + 3],
                    other.texcoord_[ot1 * other.numTextureCoordComponents()],
                    other.texcoord_[ot1 * other.numTextureCoordComponents() + 1]),
                t2 = addVertex(
                    other.vertex_[ot2 * other.numVertexComponents()] + offset.x,
                    other.vertex_[ot2 * other.numVertexComponents() + 1] + offset.y,
                    other.vertex_[ot2 * other.numVertexComponents() + 2] + offset.z,
                    other.normal_[ot2 * other.numNormalComponents()],
                    other.normal_[ot2 * other.numNormalComponents() + 1],
                    other.normal_[ot2 * other.numNormalComponents() + 2],
                    other.color_[ot2 * other.numColorComponents()],
                    other.color_[ot2 * other.numColorComponents() + 1],
                    other.color_[ot2 * other.numColorComponents() + 2],
                    other.color_[ot2 * other.numColorComponents() + 3],
                    other.texcoord_[ot2 * other.numTextureCoordComponents()],
                    other.texcoord_[ot2 * other.numTextureCoordComponents() + 1]),
                t3 = addVertex(
                    other.vertex_[ot3 * other.numVertexComponents()] + offset.x,
                    other.vertex_[ot3 * other.numVertexComponents() + 1] + offset.y,
                    other.vertex_[ot3 * other.numVertexComponents() + 2] + offset.z,
                    other.normal_[ot3 * other.numNormalComponents()],
                    other.normal_[ot3 * other.numNormalComponents() + 1],
                    other.normal_[ot3 * other.numNormalComponents() + 2],
                    other.color_[ot3 * other.numColorComponents()],
                    other.color_[ot3 * other.numColorComponents() + 1],
                    other.color_[ot3 * other.numColorComponents() + 2],
                    other.color_[ot3 * other.numColorComponents() + 3],
                    other.texcoord_[ot3 * other.numTextureCoordComponents()],
                    other.texcoord_[ot3 * other.numTextureCoordComponents() + 1]);

        //for (auto a : other.attributes_)
        //    setAttribute();

        addTriangle(t1, t2, t3
#ifndef MO_DISABLE_EDGEFLAG
                    , other.edgeFlags_[i*3]
                    , other.edgeFlags_[i*3+1]
                    , other.edgeFlags_[i*3+2]
#endif
                );
    }
    // copy lines
    for (uint i=0; i<other.numLines(); ++i)
    {
        IndexType
                ol1 = other.lineIndex_[i * 2],
                ol2 = other.lineIndex_[i * 2 + 1],

                l1 = addVertex(
                    other.vertex_[ol1 * other.numVertexComponents()] + offset.x,
                    other.vertex_[ol1 * other.numVertexComponents() + 1] + offset.y,
                    other.vertex_[ol1 * other.numVertexComponents() + 2] + offset.z,
                    other.normal_[ol1 * other.numNormalComponents()],
                    other.normal_[ol1 * other.numNormalComponents() + 1],
                    other.normal_[ol1 * other.numNormalComponents() + 2],
                    other.color_[ol1 * other.numColorComponents()],
                    other.color_[ol1 * other.numColorComponents() + 1],
                    other.color_[ol1 * other.numColorComponents() + 2],
                    other.color_[ol1 * other.numColorComponents() + 3],
                    other.texcoord_[ol1 * other.numTextureCoordComponents()],
                    other.texcoord_[ol1 * other.numTextureCoordComponents() + 1]),
                l2 = addVertex(
                    other.vertex_[ol2 * other.numVertexComponents()] + offset.x,
                    other.vertex_[ol2 * other.numVertexComponents() + 1] + offset.y,
                    other.vertex_[ol2 * other.numVertexComponents() + 2] + offset.z,
                    other.normal_[ol2 * other.numNormalComponents()],
                    other.normal_[ol2 * other.numNormalComponents() + 1],
                    other.normal_[ol2 * other.numNormalComponents() + 2],
                    other.color_[ol2 * other.numColorComponents()],
                    other.color_[ol2 * other.numColorComponents() + 1],
                    other.color_[ol2 * other.numColorComponents() + 2],
                    other.color_[ol2 * other.numColorComponents() + 3],
                    other.texcoord_[ol2 * other.numTextureCoordComponents()],
                    other.texcoord_[ol2 * other.numTextureCoordComponents() + 1]);

        addLine(l1, l2);
    }
}



void Geometry::scale(VertexType x, VertexType y, VertexType z)
{
    setChanged();
    for (uint i=0; i<numVertices(); ++i)
    {
        vertex_[i*numVertexComponents()] *= x;
        vertex_[i*numVertexComponents()+1] *= y;
        vertex_[i*numVertexComponents()+2] *= z;
    }
}

void Geometry::translate(VertexType x, VertexType y, VertexType z)
{
    setChanged();
    for (uint i=0; i<numVertices(); ++i)
    {
        vertex_[i*numVertexComponents()] += x;
        vertex_[i*numVertexComponents()+1] += y;
        vertex_[i*numVertexComponents()+2] += z;
    }
}

void Geometry::applyMatrix(const Mat4 &transformation)
{
    setChanged();
    // transform vertices
    for (uint i=0; i<numVertices(); ++i)
    {
        Vec4 v(
            vertex_[i*numVertexComponents()],
            vertex_[i*numVertexComponents()+1],
            vertex_[i*numVertexComponents()+2], 1.f);
        v = transformation * v;
        vertex_[i*numVertexComponents()] = v[0];
        vertex_[i*numVertexComponents()+1] = v[1];
        vertex_[i*numVertexComponents()+2] = v[2];
    }
    // transform normals
    for (uint i=0; i<numVertices(); ++i)
    {
        Vec4 v(
            normal_[i*numVertexComponents()],
            normal_[i*numVertexComponents()+1],
            normal_[i*numVertexComponents()+2], 0.f);
        Vec3 v3 = glm::normalize(Vec3(transformation * v));
        normal_[i*numVertexComponents()] = v3[0];
        normal_[i*numVertexComponents()+1] = v3[1];
        normal_[i*numVertexComponents()+2] = v3[2];
    }
}


void Geometry::calculateTriangleNormals()
{
    setChanged();
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
        Vec3 n = MATH::normalize_safe( glm::cross( p2-p1, p3-p1 ) );

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
        const size_t num = nr_adds[i/3];
        if (num)
            normal_[i] /= num;
    }
}

void Geometry::invertNormals()
{
    setChanged();
    for (uint i=0; i<normal_.size(); ++i)
    {
        normal_[i] = -normal_[i];
    }
}

void Geometry::invertTextureCoords(bool invX, bool invY)
{
    MO_ASSERT(numTextureCoordComponents() == 2, "code not up-to-date");
    setChanged();

    const uint si = texcoord_.size()/2;
    if (invX)
    {
        TextureCoordType * tex = &texcoord_[0];
        for (uint i=0; i<si; ++i, tex += 2)
            *tex = 1.0 - *tex;
    }
    if (invY)
    {
        TextureCoordType * tex = &texcoord_[1];
        for (uint i=0; i<si; ++i, tex += 2)
            *tex = 1.0 - *tex;
    }
}

void Geometry::shiftTextureCoords(TextureCoordType offsetX, TextureCoordType offsetY)
{
    setChanged();
    MO_ASSERT(numTextureCoordComponents() == 2, "code not up-to-date");

    for (uint i=0; i<texcoord_.size(); i+=2)
    {
        texcoord_[i] += offsetX;
        texcoord_[i+1] += offsetY;
    }
}

void Geometry::scaleTextureCoords(TextureCoordType scaleX, TextureCoordType scaleY)
{
    setChanged();
    MO_ASSERT(numTextureCoordComponents() == 2, "code not up-to-date");

    for (uint i=0; i<texcoord_.size(); i+=2)
    {
        texcoord_[i] *= scaleX;
        texcoord_[i+1] *= scaleY;
    }
}

void Geometry::unGroupVertices()
{
    setChanged();

    // backup data
    auto vertex = vertex_;
    auto normal = normal_;
    auto color = color_;
    auto texcoord = texcoord_;
#ifndef MO_DISABLE_EDGEFLAG
    auto edgeFlags = edgeFlags_;
#endif

    vertex_.clear();
    normal_.clear();
    color_.clear();
    texcoord_.clear();
    p_->indexMap_.clear();
    p_->clearPrimitiveHash();

#ifndef MO_DISABLE_EDGEFLAG
    edgeFlags_.clear();
#endif
    sharedVertices_ = false;

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
            addTriangle(t1, t2, t3
#ifndef MO_DISABLE_EDGEFLAG
                        , edgeFlags[i*3]
                        , edgeFlags[i*3+1]
                        , edgeFlags[i*3+2]
#endif
                    );
        }
    }

    if (numLines())
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

    if (numPoints())
    {
        auto index = pointIndex_;
        pointIndex_.clear();

        // for each previous point..
        for (uint i=0; i<index.size(); ++i)
        {
            IndexType
                i1 = index[i],

                t1 = addVertex(vertex[i1*3], vertex[i1*3+1], vertex[i1*3+2],
                               normal[i1*3], normal[i1*3+1], normal[i1*3+2],
                               color[i1*4], color[i1*4+1], color[i1*4+2], color[i1*4+3],
                               texcoord[i1*2], texcoord[i1*2+1]);

            // .. create a new unique point
            addPoint(t1);
        }
    }
}

void Geometry::convertToLines()
{
    if (!numTriangles())
        return;

    setChanged();

    //lineIndex_.clear();

    // test for already-connected
    typedef quint64 Hash;
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
    setChanged();

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

bool Geometry::transformWithEquation(const QString& equationX,
                                     const QString& equationY,
                                     const QString& equationZ,
                                     const QStringList &constantNames,
                                     const QList<Double> &constantValues)
{
    setChanged();

    Double vx, vy, vz, vindex;

    std::vector<PPP_NAMESPACE::Parser> equ(3);
    for (uint i=0; i<3; ++i)
    {
        equ[i].variables().add("x", &vx, "");
        equ[i].variables().add("y", &vy, "");
        equ[i].variables().add("z", &vz, "");
        equ[i].variables().add("i", &vindex, "");

        for (auto j=0; j<constantNames.size(); ++j)
            equ[i].variables().add(constantNames[j].toStdString(), constantValues[j], "");
    }

    if (!equ[0].parse(equationX.toStdString()))
        return false;
    if (!equ[1].parse(equationY.toStdString()))
        return false;
    if (!equ[2].parse(equationZ.toStdString()))
        return false;

    for (uint i=0; i<numVertices(); ++i)
    {
        // copy input variables
        VertexType * v = &vertex_[i * numVertexComponents()];
        vx = v[0];
        vy = v[1];
        vz = v[2];
        vindex = i;

        // assign result from equation
        if (equationX != "x")
            v[0] = equ[0].eval();
        if (equationY != "y")
            v[1] = equ[1].eval();
        if (equationZ != "z")
            v[2] = equ[2].eval();

        progress_ = (i * 100) / numVertices();
    }

    return true;
}


bool Geometry::transformWithEquation(const QString& equation,
                                     const QStringList &constantNames,
                                     const QList<Double> &constantValues)
{
    setChanged();

    Double vx, vy, vz, vindex, vs, vt,
            red,green,blue,alpha,bright;

    PPP_NAMESPACE::Parser equ;
    equ.variables().add("x", &vx, "");
    equ.variables().add("y", &vy, "");
    equ.variables().add("z", &vz, "");
    equ.variables().add("i", &vindex, "");
    equ.variables().add("s", &vs, "");
    equ.variables().add("t", &vt, "");
    equ.variables().add("red", &red, "");
    equ.variables().add("green", &green, "");
    equ.variables().add("blue", &blue, "");
    equ.variables().add("alpha", &alpha, "");
    equ.variables().add("bright", &bright, "");

    for (auto j=0; j<constantNames.size(); ++j)
        equ.variables().add(constantNames[j].toStdString(), constantValues[j], "");

    if (!equ.parse(equation.toStdString()))
        return false;

    for (uint i=0; i<numVertices(); ++i)
    {
        // copy input variables
        VertexType * v = &vertex_[i * numVertexComponents()];
        vx = v[0];
        vy = v[1];
        vz = v[2];
        vindex = i;
        TextureCoordType * t = &texcoord_[i * numTextureCoordComponents()];
        vs = t[0];
        vt = t[1];
        vindex = i;
        ColorType * c = &color_[i * numColorComponents()];
        red = c[0];
        green = c[1];
        blue = c[2];
        alpha = c[3];
        bright = 1;

        equ.eval();

        // assign result from equation
        v[0] = vx;
        v[1] = vy;
        v[2] = vz;
        t[0] = vs;
        t[1] = vt;
        c[0] = std::max(0.0, std::min(1.0, red * bright ));
        c[1] = std::max(0.0, std::min(1.0, green * bright ));
        c[2] = std::max(0.0, std::min(1.0, blue * bright ));
        c[3] = std::max(0.0, std::min(1.0, alpha ));

        progress_ = (i * 100) / numVertices();
    }

    return true;
}


bool Geometry::transformPrimitivesWithEquation(
                    const QString &equationX,
                    const QString &equationY,
                    const QString &equationZ,
                    const QStringList& constantNames,
                    const QList<Double>& constantValues)
{
    setChanged();

    Double vx, vy, vz, vnx, vny, vnz, vs, vt,
           vpx[3], vpy[3], vpz[3],
           vpnx[3], vpny[3], vpnz[3],
           vps[3], vpt[3],
           vp, vi;

    std::vector<PPP_NAMESPACE::Parser> equ(3);
    for (uint i=0; i<3; ++i)
    {
        equ[i].variables().add("x", &vx, "");
        equ[i].variables().add("y", &vy, "");
        equ[i].variables().add("z", &vz, "");
        equ[i].variables().add("nx", &vnx, "");
        equ[i].variables().add("ny", &vny, "");
        equ[i].variables().add("nz", &vnz, "");
        equ[i].variables().add("s", &vs, "");
        equ[i].variables().add("t", &vt, "");
        equ[i].variables().add("x1", &vpx[0], "");
        equ[i].variables().add("y1", &vpy[0], "");
        equ[i].variables().add("z1", &vpz[0], "");
        equ[i].variables().add("x2", &vpx[1], "");
        equ[i].variables().add("y2", &vpy[1], "");
        equ[i].variables().add("z2", &vpz[1], "");
        equ[i].variables().add("x3", &vpx[2], "");
        equ[i].variables().add("y3", &vpy[2], "");
        equ[i].variables().add("z3", &vpz[2], "");
        equ[i].variables().add("nx1", &vpnx[0], "");
        equ[i].variables().add("ny1", &vpny[0], "");
        equ[i].variables().add("nz1", &vpnz[0], "");
        equ[i].variables().add("nx2", &vpnx[1], "");
        equ[i].variables().add("ny2", &vpny[1], "");
        equ[i].variables().add("nz2", &vpnz[1], "");
        equ[i].variables().add("nx3", &vpnx[2], "");
        equ[i].variables().add("ny3", &vpny[2], "");
        equ[i].variables().add("nz3", &vpnz[2], "");
        equ[i].variables().add("s1", &vps[0], "");
        equ[i].variables().add("t1", &vpt[0], "");
        equ[i].variables().add("s2", &vps[1], "");
        equ[i].variables().add("t2", &vpt[1], "");
        equ[i].variables().add("s3", &vps[2], "");
        equ[i].variables().add("t3", &vpt[2], "");
        equ[i].variables().add("i", &vi, "");
        equ[i].variables().add("p", &vp, "");
        for (auto j=0; j<constantNames.size(); ++j)
            equ[i].variables().add(constantNames[j].toStdString(), constantValues[j], "");
    }

    if (!equ[0].parse(equationX.toStdString()))
        return false;
    if (!equ[1].parse(equationY.toStdString()))
        return false;
    if (!equ[2].parse(equationZ.toStdString()))
        return false;

    for (uint i=0; i<numLines(); ++i)
    {
        // get vertex indices
        int i1 = lineIndex_[i*2],
            i2 = lineIndex_[i*2+1];

        // copy primitive input variables
              VertexType * vert[] = { &vertex_[i1 * numVertexComponents()],
                                      &vertex_[i2 * numVertexComponents()] };
        const NormalType * norm[] = { &normal_[i1 * numNormalComponents()],
                                      &normal_[i2 * numNormalComponents()] };
        const TextureCoordType * tex[] = { &texcoord_[i1 * numTextureCoordComponents()],
                                           &texcoord_[i2 * numTextureCoordComponents()] };
        vi = i;
        for (int j=0; j<2; ++j)
        {
            vpx[j] = vert[j][0];
            vpy[j] = vert[j][1];
            vpz[j] = vert[j][2];
            vpnx[j] = norm[j][0];
            vpny[j] = norm[j][1];
            vpnz[j] = norm[j][2];
            vps[j] = tex[j][0];
            vpt[j] = tex[j][1];
        }

        // execute per primitive vertex
        for (int j=0; j<2; ++j)
        {
            // copy 'current' variables
            vp = j;
            vx = vpx[j];
            vy = vpy[j];
            vz = vpz[j];
            vnx = vpnx[j];
            vny = vpny[j];
            vnz = vpnz[j];
            vs = vps[j];
            vt = vpt[j];

            // assign result from equation
            if (equationX != "x")
                vert[j][0] = equ[0].eval();
            if (equationY != "y")
                vert[j][1] = equ[1].eval();
            if (equationZ != "z")
                vert[j][2] = equ[2].eval();
        }

        progress_ = (i * 100) / numLines();
    }

    for (uint i=0; i<numTriangles(); ++i)
    {
        // get vertex indices
        int i1 = triIndex_[i*3],
            i2 = triIndex_[i*3+1],
            i3 = triIndex_[i*3+2];

        // copy primitive input variables
              VertexType * vert[] = { &vertex_[i1 * numVertexComponents()],
                                      &vertex_[i2 * numVertexComponents()],
                                      &vertex_[i3 * numVertexComponents()] };
        const NormalType * norm[] = { &normal_[i1 * numNormalComponents()],
                                      &normal_[i2 * numNormalComponents()],
                                      &normal_[i3 * numVertexComponents()] };
        const TextureCoordType * tex[] = { &texcoord_[i1 * numTextureCoordComponents()],
                                           &texcoord_[i2 * numTextureCoordComponents()],
                                           &texcoord_[i3 * numTextureCoordComponents()] };

        vi = i;
        for (int j=0; j<3; ++j)
        {
            vpx[j] = vert[j][0];
            vpy[j] = vert[j][1];
            vpz[j] = vert[j][2];
            vpnx[j] = norm[j][0];
            vpny[j] = norm[j][1];
            vpnz[j] = norm[j][2];
            vps[j] = tex[j][0];
            vpt[j] = tex[j][1];
        }

        // execute per primitive vertex
        for (int j=0; j<3; ++j)
        {
            // copy 'current' variables
            vp = j;
            vx = vpx[j];
            vy = vpy[j];
            vz = vpz[j];
            vnx = vpnx[j];
            vny = vpny[j];
            vnz = vpnz[j];
            vs = vps[j];
            vt = vpt[j];

            // assign result from equation
            if (equationX != "x")
                vert[j][0] = equ[0].eval();
            if (equationY != "y")
                vert[j][1] = equ[1].eval();
            if (equationZ != "z")
                vert[j][2] = equ[2].eval();
        }

        progress_ = (i * 100) / numVertices();
    }


    return true;
}


bool Geometry::transformPrimitivesWithEquation(
                    const QString &equation,
                    const QStringList& constantNames,
                    const QList<Double>& constantValues)
{
    setChanged();

    Double vx, vy, vz, vnx, vny, vnz, vs, vt,
           red,green,blue,alpha,bright,
           vpx[3], vpy[3], vpz[3],
           vpnx[3], vpny[3], vpnz[3],
           vps[3], vpt[3],
           vred[3], vgreen[3], vblue[3], valpha[3],
           vp, vi;

    PPP_NAMESPACE::Parser equ;
    equ.variables().add("x", &vx, "");
    equ.variables().add("y", &vy, "");
    equ.variables().add("z", &vz, "");
    equ.variables().add("nx", &vnx, "");
    equ.variables().add("ny", &vny, "");
    equ.variables().add("nz", &vnz, "");
    equ.variables().add("s", &vs, "");
    equ.variables().add("t", &vt, "");
    equ.variables().add("red", &red, "");
    equ.variables().add("green", &green, "");
    equ.variables().add("blue", &blue, "");
    equ.variables().add("alpha", &alpha, "");
    equ.variables().add("bright", &bright, "");
    equ.variables().add("x1", &vpx[0], "");
    equ.variables().add("y1", &vpy[0], "");
    equ.variables().add("z1", &vpz[0], "");
    equ.variables().add("x2", &vpx[1], "");
    equ.variables().add("y2", &vpy[1], "");
    equ.variables().add("z2", &vpz[1], "");
    equ.variables().add("x3", &vpx[2], "");
    equ.variables().add("y3", &vpy[2], "");
    equ.variables().add("z3", &vpz[2], "");
    equ.variables().add("nx1", &vpnx[0], "");
    equ.variables().add("ny1", &vpny[0], "");
    equ.variables().add("nz1", &vpnz[0], "");
    equ.variables().add("nx2", &vpnx[1], "");
    equ.variables().add("ny2", &vpny[1], "");
    equ.variables().add("nz2", &vpnz[1], "");
    equ.variables().add("nx3", &vpnx[2], "");
    equ.variables().add("ny3", &vpny[2], "");
    equ.variables().add("nz3", &vpnz[2], "");
    equ.variables().add("s1", &vps[0], "");
    equ.variables().add("t1", &vpt[0], "");
    equ.variables().add("s2", &vps[1], "");
    equ.variables().add("t2", &vpt[1], "");
    equ.variables().add("s3", &vps[2], "");
    equ.variables().add("t3", &vpt[2], "");
    equ.variables().add("red1", &vred[0], "");
    equ.variables().add("red2", &vred[1], "");
    equ.variables().add("red3", &vred[2], "");
    equ.variables().add("green1", &vgreen[0], "");
    equ.variables().add("green2", &vgreen[1], "");
    equ.variables().add("green3", &vgreen[2], "");
    equ.variables().add("blue1", &vblue[0], "");
    equ.variables().add("blue2", &vblue[1], "");
    equ.variables().add("blue3", &vblue[2], "");
    equ.variables().add("alpha1", &valpha[0], "");
    equ.variables().add("alpha2", &valpha[1], "");
    equ.variables().add("alpha3", &valpha[2], "");
    equ.variables().add("i", &vi, "");
    equ.variables().add("p", &vp, "");
    for (auto j=0; j<constantNames.size(); ++j)
        equ.variables().add(constantNames[j].toStdString(), constantValues[j], "");

    if (!equ.parse(equation.toStdString()))
        return false;

    for (uint i=0; i<numLines(); ++i)
    {
        // get vertex indices
        int i1 = lineIndex_[i*2],
            i2 = lineIndex_[i*2+1];

        // copy primitive input variables
        VertexType * vert[] = { &vertex_[i1 * numVertexComponents()],
                                &vertex_[i2 * numVertexComponents()] };
        NormalType * norm[] = { &normal_[i1 * numNormalComponents()],
                                &normal_[i2 * numNormalComponents()] };
        TextureCoordType * tex[] = { &texcoord_[i1 * numTextureCoordComponents()],
                                     &texcoord_[i2 * numTextureCoordComponents()] };
        ColorType * color[] = { &color_[i1 * numColorComponents()],
                                &color_[i2 * numColorComponents()] };

        // assign all primitive vertex data
        vi = i;
        for (int j=0; j<2; ++j)
        {
            vpx[j] = vert[j][0];
            vpy[j] = vert[j][1];
            vpz[j] = vert[j][2];
            vpnx[j] = norm[j][0];
            vpny[j] = norm[j][1];
            vpnz[j] = norm[j][2];
            vps[j] = tex[j][0];
            vpt[j] = tex[j][1];
            vred[j] = color[j][0];
            vgreen[j] = color[j][1];
            vblue[j] = color[j][2];
            valpha[j] = color[j][3];
        }

        // execute per primitive vertex
        for (int j=0; j<2; ++j)
        {
            // copy 'current' variables
            vp = j;
            vx = vpx[j];
            vy = vpy[j];
            vz = vpz[j];
            vnx = vpnx[j];
            vny = vpny[j];
            vnz = vpnz[j];
            vs = vps[j];
            vt = vpt[j];
            red = vred[j];
            green = vgreen[j];
            blue = vblue[j];
            alpha = valpha[j];
            bright = 1;

            equ.eval();

            // assign result back
            vert[j][0] = vx;
            vert[j][1] = vy;
            vert[j][2] = vz;
            norm[j][0] = vnx;
            norm[j][1] = vny;
            norm[j][2] = vnz;
            tex[j][0] = vs;
            tex[j][1] = vt;
            color[j][0] = std::max(0.0,std::min(1.0, red * bright ));
            color[j][1] = std::max(0.0,std::min(1.0, green * bright ));
            color[j][2] = std::max(0.0,std::min(1.0, blue * bright ));
            color[j][3] = std::max(0.0,std::min(1.0, alpha ));
        }

        progress_ = (i * 100) / numLines();
    }

    for (uint i=0; i<numTriangles(); ++i)
    {
        // get vertex indices
        int i1 = triIndex_[i*3],
            i2 = triIndex_[i*3+1],
            i3 = triIndex_[i*3+2];

        // copy primitive input variables
        VertexType * vert[] = { &vertex_[i1 * numVertexComponents()],
                                &vertex_[i2 * numVertexComponents()],
                                &vertex_[i3 * numVertexComponents()] };
        NormalType * norm[] = { &normal_[i1 * numNormalComponents()],
                                &normal_[i2 * numNormalComponents()],
                                &normal_[i3 * numVertexComponents()] };
        TextureCoordType * tex[] = { &texcoord_[i1 * numTextureCoordComponents()],
                                     &texcoord_[i2 * numTextureCoordComponents()],
                                     &texcoord_[i3 * numTextureCoordComponents()] };
        ColorType * color[] = { &color_[i1 * numColorComponents()],
                                &color_[i2 * numColorComponents()],
                                &color_[i3 * numColorComponents()] };

        vi = i;
        for (int j=0; j<3; ++j)
        {
            vpx[j] = vert[j][0];
            vpy[j] = vert[j][1];
            vpz[j] = vert[j][2];
            vpnx[j] = norm[j][0];
            vpny[j] = norm[j][1];
            vpnz[j] = norm[j][2];
            vps[j] = tex[j][0];
            vpt[j] = tex[j][1];
            vred[j] = color[j][0];
            vgreen[j] = color[j][1];
            vblue[j] = color[j][2];
            valpha[j] = color[j][3];
        }

        // execute per primitive vertex
        for (int j=0; j<3; ++j)
        {
            // copy 'current' variables
            vp = j;
            vx = vpx[j];
            vy = vpy[j];
            vz = vpz[j];
            vnx = vpnx[j];
            vny = vpny[j];
            vnz = vpnz[j];
            vs = vps[j];
            vt = vpt[j];
            red = vred[j];
            green = vgreen[j];
            blue = vblue[j];
            alpha = valpha[j];
            bright = 1;

            equ.eval();

            // assign result from equation
            vert[j][0] = vx;
            vert[j][1] = vy;
            vert[j][2] = vz;
            norm[j][0] = vnx;
            norm[j][1] = vny;
            norm[j][2] = vnz;
            tex[j][0] = vs;
            tex[j][1] = vt;
            color[j][0] = std::max(0.0,std::min(1.0, red * bright ));
            color[j][1] = std::max(0.0,std::min(1.0, green * bright ));
            color[j][2] = std::max(0.0,std::min(1.0, blue * bright ));
            color[j][3] = std::max(0.0,std::min(1.0, alpha ));
        }

        progress_ = (i * 100) / numVertices();
    }


    return true;
}









/** @todo take care of edge flags */
void Geometry::extrudeTriangles(Geometry &geom, VertexType constant, VertexType factor, VertexType eshift,
                                bool createNewFaces, bool recognizeEdges) const
{
    // for edge recognition
    Map3 verts;
    std::vector<uint> extVerts;

#define MO__MAKEHASH(x__, y__, z__) \
    Hash3(MATH::quant(x__, 0.001f), MATH::quant(y__, 0.001f), MATH::quant(z__, 0.001f))

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

                pn1 = getNormal(t1),
                pn2 = getNormal(t2),
                pn3 = getNormal(t3);

        const Float
                dist12 = glm::distance(p1, p2),
                dist13 = glm::distance(p1, p3),
                dist23 = glm::distance(p2, p3);

        const Vec3
                // extruded points
                pe_1 = p1 + pn1 * (constant + factor * (dist12 + dist13)),
                pe_2 = p2 + pn2 * (constant + factor * (dist12 + dist23)),
                pe_3 = p3 + pn3 * (constant + factor * (dist13 + dist23)),
                pe_mid = (pe_1 + pe_2 + pe_3) / 3.f,
                // with shift
                pe1 = pe_1 + eshift * (pe_mid - pe_1),
                pe2 = pe_2 + eshift * (pe_mid - pe_2),
                pe3 = pe_3 + eshift * (pe_mid - pe_3);

        const Vec4
                pc1 = getColor(t1),
                pc2 = getColor(t2),
                pc3 = getColor(t3);

        const Vec2
                pt1 = getTexCoord(t1),
                pt2 = getTexCoord(t2),
                pt3 = getTexCoord(t3);

        // -- create vertices in destination geom --

        const IndexType
                // extruded triangle
                d4 = geom.addVertex(pe1[0], pe1[1], pe1[2], pn1[0], pn1[1], pn1[2], pc1[0], pc1[1], pc1[2], pc1[3], pt1[0], pt1[1]),
                d5 = geom.addVertex(pe2[0], pe2[1], pe2[2], pn2[0], pn2[1], pn2[2], pc2[0], pc2[1], pc2[2], pc2[3], pt2[0], pt2[1]),
                d6 = geom.addVertex(pe3[0], pe3[1], pe3[2], pn3[0], pn3[1], pn3[2], pc3[0], pc3[1], pc3[2], pc3[3], pt3[0], pt3[1]);

        // create extruded triangle in geom
        geom.addTriangleChecked(d4,d5,d6);

        // count number of extruded vertices use
        if (createNewFaces && recognizeEdges)
        {
            // also remember them!
            extVerts.push_back(d4);
            extVerts.push_back(d5);
            extVerts.push_back(d6);

            Hash3   hash1 = MO__MAKEHASH(pe1[0], pe1[1], pe1[2]),
                    hash2 = MO__MAKEHASH(pe2[0], pe2[1], pe2[2]),
                    hash3 = MO__MAKEHASH(pe3[0], pe3[1], pe3[2]);

            auto i = verts.find(hash1);
            if (i == verts.end())
                verts.insert(hash1, 1);
            else
                i.value()++;

            i = verts.find(hash2);
            if (i == verts.end())
                verts.insert(hash2, 1);
            else
                i.value()++;

            i = verts.find(hash3);
            if (i == verts.end())
                verts.insert(hash3, 1);
            else
                i.value()++;
        }

        if (createNewFaces && !recognizeEdges)
        {
            const IndexType
                // original triangle for side-faces
                d1 = geom.addVertex(p1[0], p1[1], p1[2], pn1[0], pn1[1], pn1[2], pc1[0], pc1[1], pc1[2], pc1[3], pt1[0], pt1[1]),
                d2 = geom.addVertex(p2[0], p2[1], p2[2], pn2[0], pn2[1], pn2[2], pc2[0], pc2[1], pc2[2], pc2[3], pt2[0], pt2[1]),
                d3 = geom.addVertex(p3[0], p3[1], p3[2], pn3[0], pn3[1], pn3[2], pc3[0], pc3[1], pc3[2], pc3[3], pt3[0], pt3[1]);

            // create side faces
            geom.addTriangleChecked(d1,d2,d5);
            geom.addTriangleChecked(d1,d5,d4);
            geom.addTriangleChecked(d2,d3,d6);
            geom.addTriangleChecked(d2,d6,d5);
            geom.addTriangleChecked(d3,d1,d4);
            geom.addTriangleChecked(d3,d4,d6);

        }
    }

    // create side faces dependend of extruded vertex count
    if (createNewFaces && recognizeEdges)
    {
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

                    pn1 = getNormal(t1),
                    pn2 = getNormal(t2),
                    pn3 = getNormal(t3);

            const Float
                    dist12 = glm::distance(p1, p2),
                    dist13 = glm::distance(p1, p3),
                    dist23 = glm::distance(p2, p3);

            const Vec3
                    // extruded points
                    pe1 = p1 + pn1 * (constant + factor * (dist12 + dist13)),
                    pe2 = p2 + pn2 * (constant + factor * (dist12 + dist23)),
                    pe3 = p3 + pn3 * (constant + factor * (dist13 + dist23));

            const Vec4
                    pc1 = getColor(t1),
                    pc2 = getColor(t2),
                    pc3 = getColor(t3);

            const Vec2
                    pt1 = getTexCoord(t1),
                    pt2 = getTexCoord(t2),
                    pt3 = getTexCoord(t3);

            Hash3   hash1 = MO__MAKEHASH(pe1[0], pe1[1], pe1[2]),
                    hash2 = MO__MAKEHASH(pe2[0], pe2[1], pe2[2]),
                    hash3 = MO__MAKEHASH(pe3[0], pe3[1], pe3[2]);

            const uint
                    count1 = verts.value(hash1, 0),
                    count2 = verts.value(hash2, 0),
                    count3 = verts.value(hash3, 0);

            const IndexType
                // original triangle for side-faces
                d1 = geom.addVertex(p1[0], p1[1], p1[2], pn1[0], pn1[1], pn1[2], pc1[0], pc1[1], pc1[2], pc1[3], pt1[0], pt1[1]),
                d2 = geom.addVertex(p2[0], p2[1], p2[2], pn2[0], pn2[1], pn2[2], pc2[0], pc2[1], pc2[2], pc2[3], pt2[0], pt2[1]),
                d3 = geom.addVertex(p3[0], p3[1], p3[2], pn3[0], pn3[1], pn3[2], pc3[0], pc3[1], pc3[2], pc3[3], pt3[0], pt3[1]),
                d4 = extVerts[i*3],
                d5 = extVerts[i*3+1],
                d6 = extVerts[i*3+2];


            if (geom.checkTriangle(p1,p2,p3))
            {
                // create side faces
                if (count1 < 2 || count2 < 2)
                {
                    geom.addTriangleChecked(d1,d2,d5);
                    geom.addTriangleChecked(d1,d5,d4);
                }
                if (count2 < 2 || count3 < 2)
                {
                    geom.addTriangleChecked(d2,d3,d6);
                    geom.addTriangleChecked(d2,d6,d5);
                }
                if (count3 < 2 || count1 < 2)
                {
                    geom.addTriangleChecked(d3,d1,d4);
                    geom.addTriangleChecked(d3,d4,d6);
                }
            }
        }
    }

#undef MO__MAKEHASH
}

/** @todo color/texcoord handling in tesselateLines() */
void Geometry::tesselateLines(uint level)
{
    if (!numLines())
        return;

    Geometry tess;
    tess.sharedVertices_ = sharedVertices_;
    tess.p_shareThreshold_ = p_shareThreshold_;

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

        progress_ = (i * 100) / numLines();
    }

    *this = tess;
}

/** @todo take care of edge flags */
void Geometry::tesselateTriangles(uint level)
{
    if (!numTriangles())
        return;
    for (uint l = 0; l<level; ++l)
    {
        Geometry tess;
        tess.sharedVertices_ = sharedVertices_;
        tess.p_shareThreshold_ = p_shareThreshold_;

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
                    // original triangle corners
                    n1 = tess.addVertex(p1[0], p1[1], p1[2], pn1[0], pn1[1], pn1[2], pc1[0], pc1[1], pc1[2], pc1[3], pt1[0], pt1[1]),
                    n2 = tess.addVertex(p2[0], p2[1], p2[2], pn2[0], pn2[1], pn2[2], pc2[0], pc2[1], pc2[2], pc2[3], pt2[0], pt2[1]),
                    n3 = tess.addVertex(p3[0], p3[1], p3[2], pn3[0], pn3[1], pn3[2], pc3[0], pc3[1], pc3[2], pc3[3], pt3[0], pt3[1]),
                    // new tesselated corners
                    n12a = tess.addVertex(p12[0], p12[1], p12[2], pn12[0], pn12[1], pn12[2], pc12[0], pc12[1], pc12[2], pc12[3], pt12[0], pt12[1]),
                    n13a = tess.addVertex(p13[0], p13[1], p13[2], pn13[0], pn13[1], pn13[2], pc13[0], pc13[1], pc13[2], pc13[3], pt13[0], pt13[1]),
                    n23a = tess.addVertex(p23[0], p23[1], p23[2], pn23[0], pn23[1], pn23[2], pc23[0], pc23[1], pc23[2], pc23[3], pt23[0], pt23[1]),
                    // copies for unshared vertex mode
                    // (same for shared vertex mode)
                    n12b = tess.duplicateVertex(n12a),
                    n12c = tess.duplicateVertex(n12a),
                    n13b = tess.duplicateVertex(n13a),
                    n13c = tess.duplicateVertex(n13a),
                    n23b = tess.duplicateVertex(n23a),
                    n23c = tess.duplicateVertex(n23a);

            tess.addTriangle(n1, n12a, n13a);
            tess.addTriangle(n12b, n2, n23a);
            tess.addTriangle(n12c, n23c, n13c);
            tess.addTriangle(n13b, n23b, n3);
        }

        // XXX hmm...
        //progress_ = ???

        *this = tess;
    }
}

/** @todo take care of edge flags */
void Geometry::tesselateTriangles(VertexType minArea, VertexType minLength, uint level)
{
    if (!numTriangles())
        return;
    for (uint l = 0; l<level; ++l)
    {
        Geometry tess;
        tess.sharedVertices_ = sharedVertices_;
        tess.p_shareThreshold_ = p_shareThreshold_;

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
                    n3 = tess.addVertex(p3[0], p3[1], p3[2], pn3[0], pn3[1], pn3[2], pc3[0], pc3[1], pc3[2], pc3[3], pt3[0], pt3[1]);

            // stop at minimum area
            if (MATH::triangle_area(p1, p2, p3) < minArea)
            {
                tess.addTriangle(n1, n2, n3);
                continue;
            }

            // check lengths
            const bool
                    split1 = glm::length(p2 - p1) > minLength,
                    split2 = glm::length(p3 - p2) > minLength,
                    split3 = glm::length(p1 - p3) > minLength;

            const IndexType
                    n12a = !split1 ? 0 : tess.addVertex(p12[0], p12[1], p12[2], pn12[0], pn12[1], pn12[2], pc12[0], pc12[1], pc12[2], pc12[3], pt12[0], pt12[1]),
                    n13a = !split3 ? 0 : tess.addVertex(p13[0], p13[1], p13[2], pn13[0], pn13[1], pn13[2], pc13[0], pc13[1], pc13[2], pc13[3], pt13[0], pt13[1]),
                    n23a = !split2 ? 0 : tess.addVertex(p23[0], p23[1], p23[2], pn23[0], pn23[1], pn23[2], pc23[0], pc23[1], pc23[2], pc23[3], pt23[0], pt23[1]);

            if (split1)
            {
                if (split2)
                {
                    const IndexType
                            n12b = tess.duplicateVertex(n12a),
                            n23b = tess.duplicateVertex(n23a),
                            n23c = tess.duplicateVertex(n23a);
                    if (split3)
                    {
                        const IndexType
                                n12c = tess.duplicateVertex(n12a),
                                n13b = tess.duplicateVertex(n13a),
                                n13c = tess.duplicateVertex(n13a);
                        // full tesselation
                        tess.addTriangle(n1, n12a, n13a);
                        tess.addTriangle(n12b, n2, n23a);
                        tess.addTriangle(n12c, n23c, n13c);
                        tess.addTriangle(n13b, n23b, n3);
                    }
                    else
                    {
                        tess.addTriangle(n1, n12a, n23c);
                        tess.addTriangle(n12b, n2, n23a);
                        tess.addTriangle(n1, n23b, n3);
                    }
                }
                else // !split2
                {
                    const IndexType
                            n12b = tess.duplicateVertex(n12a);
                    if (split3)
                    {
                        const IndexType
                                n13b = tess.duplicateVertex(n13a),
                                n13c = tess.duplicateVertex(n13a),
                                n12b = tess.duplicateVertex(n12a);

                        tess.addTriangle(n1, n12a, n13a);
                        tess.addTriangle(n12b, n2, n13c);
                        tess.addTriangle(n2, n3, n13b);
                    }
                    else
                    {
                        // split1 only
                        tess.addTriangle(n1, n12a, n3);
                        tess.addTriangle(n12b, n2, n3);
                    }
                }
            }
            else // !split1
            {
                if (split2)
                {
                    const IndexType
                            n23b = tess.duplicateVertex(n23a);

                    if (split3)
                    {
                        const IndexType
                                n13b = tess.duplicateVertex(n13a),
                                n23c = tess.duplicateVertex(n23a);
                        tess.addTriangle(n1, n23c, n13a);
                        tess.addTriangle(n1, n2, n23a);
                        tess.addTriangle(n13b, n23b, n3);
                    }
                    else
                    {
                        // split2 only
                        tess.addTriangle(n1, n2, n23a);
                        tess.addTriangle(n1, n23b, n3);
                    }
                }
                else // !split2
                {
                    if (split3)
                    {
                        const IndexType
                                n13b = tess.duplicateVertex(n13a);
                        // split3 only
                        tess.addTriangle(n1, n2, n13a);
                        tess.addTriangle(n2, n3, n13b);
                    }
                    else
                    {
                        // no split
                        tess.addTriangle(n1, n2, n3);
                    }
                }
            }

        }

        // XXX hmm...
        //progress_ = ???

        *this = tess;
    }
}

void Geometry::removePrimitivesRandomly(float probability, int seed)
{
    setChanged();

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

    if (numLines())
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
    setChanged();

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


#if (0)
void Geometry::groupVertices(Geometry &dst, VertexType range) const
{
    range = std::max((VertexType)0.001, range);

//#define MO__MAKE_KEY(x__, y__, z__)
    ((Key)((x__)/range) | ((Key)((y__)/range) << 24) | ((Key)((z__)/range) << 48))



//#undef MO__MAKE_KEY
}
#endif




void Geometry::getVertexArrayObject(GL::VertexArrayObject * vao, GL::Shader * s) const
{
    if (!s->getAttribute(s->source()->attribNamePosition()))
        MO_GL_ERROR("No position attribute in shader");
    if (vertex_.empty())
        MO_GL_ERROR("Can't create a vertex array object from empty geometry");

    if (vao->isCreated())
        vao->release();

    vao->create();
    vao->bind();

    // --- position ---
    if (auto a = s->getAttribute(s->source()->attribNamePosition()))
    {
        vao->createAttribBuffer(GL::VertexArrayObject::A_POSITION,
                                a->location(),
                                vertexEnum, numVertexComponents(),
                                numVertexBytes(),
                                vertices());
    }

    // --- color ---
    if (auto a = s->getAttribute(s->source()->attribNameColor()))
    {
        MO_ASSERT(color_.size() >= numVertices() * numColorComponents(),
                  color_.size() << " >= " << numVertices() << " * " << numColorComponents());

        vao->createAttribBuffer(GL::VertexArrayObject::A_COLOR,
                                a->location(),
                                colorEnum, numColorComponents(),
                                numColorBytes(),
                                colors());
    }

    // --- normal ---
    if (auto a = s->getAttribute(s->source()->attribNameNormal()))
    {
        MO_ASSERT(normal_.size() >= numVertices() * numNormalComponents(),
                  normal_.size() << " >= " << numVertices() << " * " << numNormalComponents());

        vao->createAttribBuffer(GL::VertexArrayObject::A_NORMAL,
                                a->location(),
                                normalEnum, numNormalComponents(),
                                numNormalBytes(),
                                normals());
    }

    // --- texcoord ---
    if (auto a = s->getAttribute(s->source()->attribNameTexCoord()))
    {
        MO_ASSERT(texcoord_.size() >= numVertices() * numTextureCoordComponents(),
                  texcoord_.size() << " >= " << numVertices() << " * " << numTextureCoordComponents());

        vao->createAttribBuffer(GL::VertexArrayObject::A_TEX_COORD,
                                a->location(),
                                textureCoordEnum, numTextureCoordComponents(),
                                numTextureCoordBytes(),
                                textureCoords());
    }

    // --- user attributes ---
    int k=0;
    for (auto i : attributes_)
    {
        UserAttribute * ua = i.second;
        if (auto a = s->getAttribute(ua->attributeName))
        {
            vao->createAttribBuffer(GL::VertexArrayObject::A_USER + k++,
                                    a->location(),
                                    attributeEnum, ua->numComponents,
                                    ua->numBytes(),
                                    &ua->data[0]);
        }
    }

    // --- indices ---

    if (numTriangles())
    {
        MO_ASSERT(numVertices(), "");
        vao->createIndexBuffer(GL_TRIANGLES,
                               indexEnum,
                               numTriangles() * numTriangleIndexComponents(),
                               triangleIndices());
    }

    if (numLines())
    {
        MO_ASSERT(numVertices(), "");
        vao->createIndexBuffer(GL_LINES,
                               indexEnum,
                               numLines() * numLineIndexComponents(),
                               lineIndices());
    }

    if (numPoints())
    {
        MO_ASSERT(numVertices(), "");
        vao->createIndexBuffer(GL_POINTS,
                               indexEnum,
                               numPoints(),
                               pointIndices());
    }

#ifndef MO_DISABLE_EDGEFLAG
    // --- edgeflags ---
    if (numTriangles())
    {
        MO_ASSERT(edgeFlags_.size(), "");
        vao->createEdgeFlagBuffer(numEdgeFlagBytes(),
                                  edgeFlags());
    }
#endif

    vao->unbind();
}



QString Geometry::toJavaScriptArray(
        const QString &baseName2, bool, bool /*bool withNormals, bool withTexCoords*/) const
{
    QString script;
    QTextStream s(&script);

    QString baseName = baseName2;
    if (!baseName.isEmpty())
        baseName.append("_");

    // verts
    s << baseName << "vertices = [ ";

    for (size_t i = 0; i<vertex_.size(); ++i)
    {
        if (i != 0)
            s << ", ";
        s << vertex_[i];
    }

    s << " ];\n";

    // indices
    s << baseName << "indices = [ ";

    for (size_t i = 0; i<triIndex_.size(); ++i)
    {
        if (i != 0)
            s << ", ";
        s << triIndex_[i];
    }

    s << " ];\n";

    return script;
}



} // namespace GEOM
} // namespace MO
