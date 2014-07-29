/** @file geometry_io_obj.cpp

    @brief Wavefront OBJ io implementation for Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/
#if (0)
#include <QFile>
#include <QTextStream>

#include "geometry.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GEOM {

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

    if (s.at(x).isNumber() || s.at(x) == '-' || s.at(x) == '.'
            || s.at(x) == 'e')
    {
        int x2 = x;
        ++x;
        while (x < s.length() &&
               (s.at(x).isNumber() || s.at(x) == '.'))
                ++x;
        return s.mid(x2, x - x2).toDouble();
    }
    MO_IO_ERROR(PARSE, "expected float, found '"
                << s.right(s.length() - x) << "'");
}

uint expectUInt(const QString& s, int& x)
{
    if (!skipWS(s, x))
        MO_IO_ERROR(PARSE, "expected index, found end of line");

    if (s.at(x).isNumber())
    {
        int x2 = x;
        ++x;
        while (x < s.length() && s.at(x).isNumber())
                ++x;
        return s.mid(x2, x - x2).toUInt();
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
    i1 = expectUInt(s, x);

    // statement ended?
    if (x == s.length() || s.at(x) != '/')
    {
        i2 = i3 = 0;
        return;
    }
    else ++x;

    if (s.at(x).isNumber())
        i2 = expectUInt(s, x);
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
        i3 = expectUInt(s, x);
    else
        i3 = 0;
}


/* https://en.wikipedia.org/wiki/Wavefront_.obj_file
 * http://www.fileformat.info/format/wavefrontobj/egff.htm */
void Geometry::loadOBJ(const QString &filename)
{
    MO_DEBUG_IO("Geometry::loadOBJ(" << filename << ")");

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "Could not load .obj file '" << filename <<"'\n"
                    << f.errorString());

    const QByteArray a = f.readAll();
    QTextStream stream(a);

    int line = 0, x = 0;

    std::vector<VertexType> vertices;
    std::vector<ColorType> colors;
    std::vector<TextureCoordType> texCoords;
    std::vector<NormalType> normals;

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
                vertices.push_back( expectDouble(s, x) );
                vertices.push_back( expectDouble(s, x) );
                vertices.push_back( expectDouble(s, x) );

                continue;
            }

            // normal data
            if (s.startsWith("vn "))
            {
                x += 3;
                normals.push_back( expectDouble(s, x) );
                normals.push_back( expectDouble(s, x) );
                normals.push_back( expectDouble(s, x) );

                continue;
            }

            // textcoord data
            if (s.startsWith("vt "))
            {
                x += 3;
                texCoords.push_back( expectDouble(s, x) );
                texCoords.push_back( expectDouble(s, x) );

                continue;
            }

            // face data
            if (s.startsWith("f "))
            {
                x += 2;
                int v1, v2, v3,
                    t1, t2, t3,
                    n1, n2, n3;

                expectFaceVertex(s, x, v1, t1, n1);
                expectFaceVertex(s, x, v2, t2, n2);

                // check for relative indices
                if (v1 < 0) v1 = vertices.size() / 3 - v1;
                if (v2 < 0) v2 = vertices.size() / 3 - v2;
                if (t1 < 0) t1 = texCoords.size() / 2 - t1;
                if (t2 < 0) t2 = texCoords.size() / 2 - t2;
                if (n1 < 0) n1 = normals.size() / 3 - n1;
                if (n2 < 0) n2 = normals.size() / 3 - n2;

                v1 = (v1-1) * 3;
                v2 = (v2-1) * 3;

                if (n1) { n1 = (n1-1) * 3; setNormal(normals[n1], normals[n1+1], normals[n1+2]); }
                if (t1) { t1 = (t1-1) * 3; setTexCoord(texCoords[t1], texCoords[t1+1]); }
                const IndexType i1 = addVertex(vertices[v1], vertices[v1+1], vertices[v1+2]);

                if (n2) { n2 = (n2-1) * 3; setNormal(normals[n2], normals[n2+1], normals[n2+2]); }
                if (t2) { t2 = (t2-1) * 3; setTexCoord(texCoords[t2], texCoords[t2+1]); }
                const IndexType i2 = addVertex(vertices[v2], vertices[v2+1], vertices[v2+2]);

                // skip ws and see if 3rd face vertex is present
                while (x < s.length() && s.at(x).isSpace()) ++x;
                if (x == s.length() || !s.at(x).isNumber())
                {
                    addLine(i1, i2);
                    continue;
                }

                // read third face vertex

                expectFaceVertex(s, x, v3, t3, n3);

                if (v3 < 0) v3 = vertices.size() / 3 - v3;
                if (t3 < 0) t3 = texCoords.size() / 2 - t3;
                if (n3 < 0) n3 = normals.size() / 3 - n3;

                v3 = (v3-1) * 3;

                if (n3) { n3 = (n3-1) * 3; setNormal(normals[n3], normals[n3+1], normals[n3+2]); }
                if (t3) { t3 = (t3-1) * 3; setTexCoord(texCoords[t3], texCoords[t3+1]); }
                const IndexType i3 = addVertex(vertices[v3], vertices[v3+1], vertices[v3+2]);

                addTriangle(i1, i2, i3);
            }
        }
    }
    catch (IoException & e)
    {
        e << "\non parsing .obj file '" << filename << "'"
          << "\nat " << line << ":" << (x + 1);
        throw e;
    }
}


} // namespace GEOM
} // namespace MO

#endif
