/** @file objloader.cpp

    @brief Wavefront .obj loader

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/29/2014</p>
*/

/* some nice format docs:
    .obj
    http://www.martinreddy.net/gfx/3d/OBJ.spec
    https://en.wikipedia.org/wiki/Wavefront_.obj_file
    http://www.fileformat.info/format/wavefrontobj/egff.htm

    .mtl
    http://paulbourke.net/dataformats/mtl/
*/


#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QMutexLocker>

#include "objloader.h"
#include "io/log.h"
#include "io/error.h"
#include "geometry.h"

/** Writes the textstream to the internal log.
    @note Only useable within the ObjLoader class. */
#define MO_OBJ_LOG_IMPL_(textstream__)  \
{                                       \
    std::stringstream s__;              \
    s__ << textstream__ << std::endl;   \
    log_ += QString::fromStdString(     \
            s__.str());                 \
    MO_DEBUG_IO(s__.str());             \
}

/** Writes info during parsing into the logger */
#define MO_OBJ_LOG(textstream__) MO_OBJ_LOG_IMPL_(textstream__)

/** Writes info (with text position) during parsing into the logger.
    @note 'line' and 'x' are supposed to be defined. */
#define MO_OBJ_LOG_LN(textstream__) \
    MO_OBJ_LOG_IMPL_("[" << line << ":" << x << "] " << textstream__)

namespace MO {
namespace GEOM {


std::map<QString, ObjLoader*> ObjLoader::instances_;
QMutex ObjLoader::instanceMutex_;

ObjLoader::ObjLoader()
    :   progress_   (0),
        isLoading_  (false)
{
}

void ObjLoader::clear()
{
    log_.clear();

    material_.clear();
    materialUse_.clear();

    triangle_.clear();
    vertex_.clear();
    normal_.clear();
    texCoord_.clear();
}

void ObjLoader::loadFile(const QString &filename)
{
    MO_DEBUG_GEOM("ObjLoader::loadFile(" << filename << ")");

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "Could not load .obj file '" << filename <<"'\n"
                    << f.errorString());

    const QByteArray a = f.readAll();

    filename_ = filename;

    loadFromMemory(a);

    filename_ = "";
}


void ObjLoader::getGeometry(const QString &filename, Geometry * g)
{
    QMutexLocker lock(&instanceMutex_);

    auto i = instances_.find(filename);

    // reuse
    if (i != instances_.end())
    {
        i->second->getGeometry(g);
        return;
    }

    // create and load
    ObjLoader * loader = new ObjLoader();
    loader->loadFile(filename);
    loader->getGeometry(g);

    // store for later
    instances_.insert(std::make_pair(filename, loader));
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
                   !s.at(x).isSpace() // XXX assume the number is correct
                   //(s.at(x).isNumber() || s.at(x) == '.' || s.at(x) == 'e' || s.at(x) == '-')
                   )
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

    int expectInt(const QString& s, int& x)
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

    QString expectName(const QString& s, int& x)
    {
        if (!skipWS(s, x))
            MO_IO_ERROR(PARSE, "expected identifier, found end of line");

        if (!s.at(x).isSpace())
        {
            int x2 = x;
            ++x;
            while (x < s.length() && !(s.at(x).isSpace()))
                    ++x;
            return s.mid(x2, x - x2);
        }
        MO_IO_ERROR(PARSE, "expected identifier, found '"
                    << s.right(s.length() - x) << "'");
    }

    /** Returns empty string if nothing found */
    QString readName(const QString& s, int& x)
    {
        if (!skipWS(s, x)) return QString();

        if (!s.at(x).isSpace())
        {
            int x2 = x;
            ++x;
            while (x < s.length() && !(s.at(x).isSpace()))
                    ++x;
            return s.mid(x2, x - x2);
        }
        return QString();
    }

    /** Also handles material names that include spaces.
        @note Name must end with .mtl */
    QString readMaterialName(const QString& s, int& x, bool expect)
    {
        if (!skipWS(s, x))
        {
            if (expect)
                MO_IO_ERROR(PARSE, "expected material lib name, found end of line");
            return QString();
        }

        if (!s.at(x).isSpace())
        {
            int x2 = s.indexOf(".mtl", x, Qt::CaseInsensitive);
            if (x2 < 0)
            {
                if (expect)
                    MO_IO_ERROR(PARSE, "expected material lib name, found '"
                            << s.right(s.length() - x) << "'");
                return QString();
            }

            x2 += 4;
            QString n = s.mid(x, x2 - x);
            x = x2;
            return n;
        }
        if (expect)
            MO_IO_ERROR(PARSE, "expected material lib name, found '"
                    << s.right(s.length() - x) << "'");
        return QString();
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
    isLoading_ = true;

    QTextStream stream(bytes);

    // current line and column
    int line = 0, x = 0;

    clear();

    Material * curMaterial = 0;

    qint64 position = 0;
    try
    {
        while (!stream.atEnd())
        {
            const QString s = stream.readLine();
            if (s.isNull())
                break;

            position += s.size();
            progress_ = (position * 100) / bytes.size();

            line++;
            x = 0;

            // skip beginning whitespace
            if (!skipWS(s, x))
                continue;

            // skip comments and empty lines
            if (s.isEmpty() || s.at(x) == '#')
                continue;

            // load material library
            if (s.startsWith("mtllib "))
            {
                if (filename_.isEmpty())
                {
                    MO_OBJ_LOG_LN("ignoring material lib statement when loading from memory.");
                    continue;
                }
                x += 6;
                QString name = readMaterialName(s, x, true);

/*  Note: The original spec (www.martinreddy.net/gfx/3d/OBJ.spec)
    allows for multiple names but that breaks .obj files
    of users that like spaces in filenames :(
    So we require all material libs to have the .mtl extension! */

                while (!name.isEmpty())
                {
                    // get full path
                    QString path = QFileInfo(filename_).absolutePath();
                    if (!path.endsWith(QDir::separator())
                    && !name.startsWith(QDir::separator()))
                        path += QDir::separator();

                    // load library
                    path += name;
                    if (loadMaterialLib_(path))
                        MO_OBJ_LOG_LN("loaded material lib '" << path << "'");

                    // check for additional file arguments
                    name = readMaterialName(s, x, false);
                }
            }

            // use material
            if (s.startsWith("usemtl "))
            {
                x += 6;
                QString name = expectName(s, x);
                if (!material_.contains(name))
                {
                    MO_OBJ_LOG_LN("Material '" << name << "' not defined");
                    continue;
                }
                curMaterial = &material_[name];
            }

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

                // XXX
                // parsing could be speed-up here,
                // noting that a statement like
                // f 1/1/1 2/2/2 3//3
                // is illegal
                // so once the check has been performed
                // for the first vertex, the others
                // could be read without checks

                readFaceVertex_(s, x, v1, true);
                readFaceVertex_(s, x, v2, true);
                v1.mat = v2.mat = curMaterial;

                // no 3rd vertex? (store line)
                if (!readFaceVertex_(s, x, v3, false))
                {
                    line_.push_back(v1);
                    line_.push_back(v2);
                    continue;
                }
                v3.mat = curMaterial;

                // no 4th vertex? (store triangle)
                if (!readFaceVertex_(s, x, v4, false))
                {
                    triangle_.push_back(v1);
                    triangle_.push_back(v2);
                    triangle_.push_back(v3);
                    continue;
                }
                v4.mat = curMaterial;

                // else store quad
                //MO_OBJ_LOG_LN("add quad " << v1.v << ", " << v2.v << ", " << v3.v << ", " << v4.v
                //              << " with material " << (v1.mat? v1.mat->name : "-"));
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
    catch (Exception & e)
    {
        // add information
        e << "\non parsing .obj ";
        if (!filename_.isEmpty())
            e << "file '" << filename_ << "'";
        else
            e << " in memory";

        e << "\nat " << line << ":" << (x + 1);

        MO_OBJ_LOG("ERROR: " << e.what());

        filename_ = "";
        isLoading_ = false;

        // and rethrow
        throw;
    }
    catch (std::exception & e)
    {
        MO_OBJ_LOG("ERROR: " << e.what());

        filename_ = "";
        isLoading_ = false;

        // and rethrow
        throw;
    }
    catch (...)
    {
        MO_OBJ_LOG("unknown exception");

        filename_ = "";
        isLoading_ = false;

        // and rethrow
        throw;
    }

    filename_ = "";
    isLoading_ = false;
}


void ObjLoader::initMaterial_(Material & m) const
{
    m.alpha = 1.0;
    m.a_r = m.a_g = m.a_b = 0.5;
    m.d_r = m.d_g = m.d_b = 0.5;
    m.s_r = m.s_g = m.s_b = 1.0;
}

bool ObjLoader::loadMaterialLib_(const QString &filename)
{
    MO_DEBUG_IO("ObjLoader::loadMaterialLib_(" << filename << ")");

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
    {
        MO_OBJ_LOG("Could not load material lib '" << filename << "'");
        return false;
    }

    const QByteArray a = f.readAll();
    QTextStream stream(a);

    int line = 0, x = 0;

    Material * material = 0;

    try
    {
        while (!stream.atEnd())
        {
            const QString s = stream.readLine();
            if (s.isNull())
                break;

            ++line;
            x = 0;

            // skip comments and empty lines
            if (s.isEmpty() || s.at(x) == '#')
                continue;

            // new material
            if (s.startsWith("newmtl "))
            {
                x += 6;
                // create material
                QString name = expectName(s, x);
                material = &material_[name];
                material->name = name;
                initMaterial_(*material);
            }
            else if (!material)
                MO_IO_ERROR(PARSE, "material statement without material");


            // ambient color
            if (s.startsWith("Ka "))
            {
                x += 3;
                // ignore spectral and xyz modes
                if (!(x < s.length() &&
                      (s.at(x).isNumber() || s.at(x) == '.' || s.at(x) == '-')))
                        continue;
                material->a_r = expectDouble(s, x);
                material->a_g = readDouble(s, x, material->a_r);
                material->a_b = readDouble(s, x, material->a_r);
                //MO_OBJ_LOG_LN("read material '" << material->name << "' ambient color "
                //           << material->a_r << ", " << material->a_g << ", " << material->a_b);
            }

            // diffuse color
            if (s.startsWith("Kd "))
            {
                x += 3;
                // ignore spectral and xyz modes
                if (!(x < s.length() &&
                      (s.at(x).isNumber() || s.at(x) == '.' || s.at(x) == '-')))
                        continue;
                material->d_r = expectDouble(s, x);
                material->d_g = readDouble(s, x, material->d_r);
                material->d_b = readDouble(s, x, material->d_r);
            }

            // specular color
            if (s.startsWith("Ks "))
            {
                x += 3;
                // ignore spectral and xyz modes
                if (!(x < s.length() &&
                      (s.at(x).isNumber() || s.at(x) == '.' || s.at(x) == '-')))
                        continue;
                material->s_r = expectDouble(s, x);
                material->s_g = readDouble(s, x, material->s_r);
                material->s_b = readDouble(s, x, material->s_r);
            }

            // transparency
            if (s.startsWith("d ") || s.startsWith("Kt "))
            {
                x += 2;
                // ignore -halo mode
                if (!(x < s.length() &&
                      (s.at(x).isNumber() || s.at(x) == '.' || s.at(x) == '-')))
                        continue;
                material->alpha = expectDouble(s, x);
            }

        }
    }
    catch (IoException & e)
    {
        // add information
        e << "\non parsing .mtl '" << filename << "'"
             "\nat " << line << ":" << (x + 1);

        // and rethrow
        throw;
    }

    return true;
}








bool ObjLoader::isEmpty() const
{
    return triangle_.empty() && line_.empty();
}

void ObjLoader::getGeometry(Geometry * g) const
{
    if (isEmpty())
        return;

    const Float defNormal[] = { 0, 1, 0 };
    const Float defTex[] = { 0, 0, 0 };
    const Float defColor[] = { g->currentRed(), g->currentGreen(), g->currentBlue(), g->currentAlpha() };

    Geometry::IndexType cur[3];

    if (triangle_.size())
    {
        const uint numTriangles = triangle_.size() / 3;
        for (uint i = 0; i<numTriangles; ++i)
        {

            for (uint j=0; j<3; ++j)
            {
                const Vertex& vert = triangle_[i*3+j];
                const Float
                        *v = &vertex_[(vert.v-1) * vertexComponents],
                        *n = vert.n ? &normal_[(vert.n-1) * normalComponents] : defNormal,
                        *t = vert.t ? &texCoord_[(vert.t-1) * texCoordComponents] : defTex;

                if (vert.mat == 0)
                    cur[j] = g->addVertex(v[0], v[1], v[2],
                                          n[0], n[1], n[2],
                                          defColor[0], defColor[1], defColor[2], defColor[3],
                                          t[0], t[1]);
                else
                    cur[j] = g->addVertex(v[0], v[1], v[2],
                                          n[0], n[1], n[2],
                                          vert.mat->a_r, vert.mat->a_g, vert.mat->a_b, vert.mat->alpha,
                                          t[0], t[1]);
            }

            g->addTriangle(cur[0], cur[1], cur[2]);
        }
    }

    // lines only
    else
    {
        const uint numLines = line_.size() / 2;
        for (uint i = 0; i<numLines; ++i)
        {

            for (uint j=0; j<2; ++j)
            {
                const Vertex& vert = line_[i*2+j];
                const Float
                        *v = &vertex_[(vert.v-1) * vertexComponents],
                        *n = vert.n ? &normal_[(vert.n-1) * normalComponents] : defNormal,
                        *t = vert.t ? &texCoord_[(vert.t-1) * texCoordComponents] : defTex;

                if (vert.mat == 0)
                    cur[j] = g->addVertex(v[0], v[1], v[2],
                                          n[0], n[1], n[2],
                                          defColor[0], defColor[1], defColor[2], defColor[3],
                                          t[0], t[1]);
                else
                    cur[j] = g->addVertex(v[0], v[1], v[2],
                                          n[0], n[1], n[2],
                                          vert.mat->a_r, vert.mat->a_g, vert.mat->a_b, vert.mat->alpha,
                                          t[0], t[1]);
            }

            g->addLine(cur[0], cur[1]);
        }
    }
}





} // namespace GEOM
} // namespace MO
