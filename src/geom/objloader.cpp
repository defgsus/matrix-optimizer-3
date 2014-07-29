/** @file objloader.cpp

    @brief Wavefront .obj loader

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/29/2014</p>
*/

#include <QFile>
#include <QTextStream>

#include "objloader.h"
#include "io/log.h"
#include "io/error.h"
#include "geometry.h"


namespace MO {
namespace GEOM {


ObjLoader::ObjLoader()
{
}

void ObjLoader::clear()
{
    triangle_.clear();
    vertex_.clear();
    normal_.clear();
    texCoord_.clear();
    //color_.clear();
}

void ObjLoader::loadFile(const QString &filename)
{
    MO_DEBUG_IO("ObjLoader::loadFile(" << filename << ")");

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "Could not load .obj file '" << filename <<"'\n"
                    << f.errorString());

    const QByteArray a = f.readAll();

    filename_ = filename;

    loadFromMemory(a);

    filename_ = "";
}


namespace {

    // XXX This should be moved to a generic parser!

    /** skips whitespace, returns TRUE when string is not ended */
    bool skipWS(const QString& s, int& x)
    {
        while (x < s.length() && s.at(x).isSpace()) ++x;
        return (x < s.length());
    }

    double expectDouble(const QString& s, int& x)
    {
        if (!skipWS(s, x))
            MO_IO_ERROR(PARSE, "expected float, found end of line");

        if (s.at(x).isNumber() || s.at(x) == '-' || s.at(x) == '.')
        {
            int x2 = x;
            ++x;
            while (x < s.length() &&
                   (s.at(x).isNumber() || s.at(x) == '.' || s.at(x) == 'e'))
                    ++x;
            return s.mid(x2, x - x2).toDouble();
        }
        MO_IO_ERROR(PARSE, "expected float, found '"
                    << s.right(s.length() - x) << "'");
    }

    /** Trys to read a double value, returns @p default_ when there was none */
    double readDouble(const QString& s, int& x, double default_)
    {
        if (!skipWS(s, x))
            return default_;

        if (s.at(x).isNumber() || s.at(x) == '-' || s.at(x) == '.')
        {
            int x2 = x;
            ++x;
            while (x < s.length() &&
                   (s.at(x).isNumber() || s.at(x) == '.' || s.at(x) == 'e'))
                    ++x;
            return s.mid(x2, x - x2).toDouble();
        }
        return default_;
    }

    uint expectInt(const QString& s, int& x)
    {
        if (!skipWS(s, x))
            MO_IO_ERROR(PARSE, "expected index, found end of line");

        if (s.at(x).isNumber() || s.at(x) == '-')
        {
            int x2 = x;
            ++x;
            while (x < s.length() && s.at(x).isNumber())
                    ++x;
            return s.mid(x2, x - x2).toInt();
        }
        MO_IO_ERROR(PARSE, "expected index, found '"
                    << s.right(s.length() - x) << "'");
    }

    void expectChar(const QString& s, int& x, QChar c)
    {
        while (x < s.length() && s.at(x).isSpace()) ++x;

        if (x == s.length())
            MO_IO_ERROR(PARSE, "expected '" << c << "', found end of line");

        if (s.at(x) != c)
            MO_IO_ERROR(PARSE, "expected '" << c << "', found '"
                        << s.right(s.length() - x) << "'");
        ++x;
    }

    /** Returns vertex, texcoord and normal indices in i1, i2 and i3.
        If texcoord or normal indices are not present, 0 is returned for each.
        Indices might also be negative, meaning relative! */
    void expectFaceVertex(const QString& s, int& x, int& i1, int& i2, int &i3)
    {
        i1 = expectInt(s, x);

        // statement ended?
        if (x == s.length() || s.at(x) != '/')
        {
            i2 = i3 = 0;
            return;
        }
        else ++x;

        if (s.at(x).isNumber())
            i2 = expectInt(s, x);
        else
            i2 = 0;

        // statement ended?
        if (x == s.length() || s.at(x) != '/')
        {
            i3 = 0;
            return;
        }
        else ++x;

        if (s.at(x).isNumber())
            i3 = expectInt(s, x);
        else
            i3 = 0;
    }

} // namespace anonymous


bool ObjLoader::readFaceVertex_(const QString & s, int &x, Vertex & vertex, bool expect) const
{
    int v, t, n;
    if (expect)
        expectFaceVertex(s, x, v, t, n);
    else
    {
        // skip ws and see if number follows
        if (!(skipWS(s, x) &&
              (s.at(x).isNumber() || s.at(x) == '-')))
            return false;

        expectFaceVertex(s, x, v, t, n);
    }

    // adjust indices
    vertex.v = (v < 0)? (vertex_.size() / vertexComponents - v) : v;
    vertex.t = (t < 0)? (normal_.size() / normalComponents - t) : t;
    vertex.n = (n < 0)? (texCoord_.size() / texCoordComponents - n) : n;

    return true;
}

void ObjLoader::loadFromMemory(const QByteArray &bytes)
{
    QTextStream stream(bytes);

    // current line and column
    int line = 0, x = 0;

    clear();

    try
    {
        while (!stream.atEnd())
        {
            const QString s = stream.readLine();
            if (s.isNull())
                break;

            line++;
            x = 0;

            // skip beginning whitespace
            if (!skipWS(s, x))
                continue;

            // skip comment
            if (s.at(x) == '#')
                continue;

            // vertex data
            if (s.startsWith("v "))
            {
                x += 2;
                vertex_.push_back( expectDouble(s, x) );
                vertex_.push_back( expectDouble(s, x) );
                vertex_.push_back( expectDouble(s, x) );
                vertex_.push_back( readDouble(s, x, 1.0) );

                continue;
            }

            // normal data
            if (s.startsWith("vn "))
            {
                x += 3;
                normal_.push_back( expectDouble(s, x) );
                normal_.push_back( expectDouble(s, x) );
                normal_.push_back( expectDouble(s, x) );

                continue;
            }

            // textcoord data
            if (s.startsWith("vt "))
            {
                x += 3;
                texCoord_.push_back( expectDouble(s, x) );
                texCoord_.push_back( expectDouble(s, x) );
                texCoord_.push_back( readDouble(s, x, 0.0) );

                continue;
            }

            // face data
            if (s.startsWith("f "))
            {
                x += 2;
                Vertex v1, v2, v3, v4;

                readFaceVertex_(s, x, v1, true);
                // no 2nd vertex? (skip point primitives)
                if (!readFaceVertex_(s, x, v2, false))
                    continue;
                // XXX no 3rd vertex? (skip line primitives for now)
                if (!readFaceVertex_(s, x, v3, false))
                    continue;
                // no 4th vertex? (store triangle)
                if (!readFaceVertex_(s, x, v4, false))
                {
                    triangle_.push_back(v1);
                    triangle_.push_back(v2);
                    triangle_.push_back(v3);
                    continue;
                }

                // else store quad
                triangle_.push_back(v1);
                triangle_.push_back(v2);
                triangle_.push_back(v3);
                triangle_.push_back(v1);
                triangle_.push_back(v3);
                triangle_.push_back(v4);
            }
        }
    }
    // on parsing error
    catch (IoException & e)
    {
        // add information
        e << "\non parsing .obj ";
        if (!filename_.isEmpty())
            e << "file '" << filename_ << "'";
        else
            e << " in memory";

        e << "\nat " << line << ":" << (x + 1);

        // and rethrow
        throw e;
    }
}


bool ObjLoader::isEmpty() const
{
    return triangle_.empty();
}

void ObjLoader::getGeometry(Geometry * g) const
{
    if (isEmpty())
        return;

    // XXX implement shared vertices, pehw...

    const float defNormal[] = { 0, 1, 0 };
    const float defTex[] = { 0, 0, 0 };
    const float defColor[] = { 0.5, 0.5, 0.5, 1.0 };

    const uint numTriangles = triangle_.size() / 3;
    for (uint i = 0; i<numTriangles; ++i)
    {
        const Geometry::IndexType cur = g->numVertices();

        for (uint j=0; j<3; ++j)
        {
            const Vertex& vert = triangle_[i*3+j];
            const float
                    *v = &vertex_[(vert.v-1) * vertexComponents],
                    *n = vert.n ? &normal_[(vert.n-1) * normalComponents] : defNormal,
                    *t = vert.t ? &texCoord_[(vert.t-1) * texCoordComponents] : defTex,
                    *c = defColor;

            g->addVertex(v[0], v[1], v[2],
                         n[0], n[1], n[2],
                         c[0], c[1], c[2], c[3],
                         t[0], t[1]);
        }

        g->addTriangle(cur, cur + 1, cur + 2);
    }
}






} // namespace GEOM
} // namespace MO
