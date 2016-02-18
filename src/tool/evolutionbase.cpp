/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#include <QImage>
#include <QPainter>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QTextStream>
#include <QFile>
#include <QImageWriter>

#include "evolutionbase.h"
#include "math/random.h"
#include "types/properties.h"
#include "io/error.h"
#include "io/version.h"

namespace MO {

void EvolutionBase::copyFrom(const EvolutionBase* o)
{
    *p_props_ = *o->p_props_;
}

EvolutionBase::EvolutionBase()
    : RefCounted    ()
    , p_props_      (new Properties())
{
}

EvolutionBase::~EvolutionBase()
{
    delete p_props_;
}

QString EvolutionBase::toJsonString() const
{
    QJsonDocument doc;
    toJson(doc);
    return QString::fromUtf8(doc.toJson());
}

EvolutionBase* EvolutionBase::fromJsonString(const QString& s)
{
    auto doc = QJsonDocument::fromJson(s.toUtf8());
    return fromJson(doc);
}

EvolutionBase* EvolutionBase::fromJson(const QJsonDocument& doc)
{
    if (!doc.isObject())
        MO_IO_ERROR(VERSION_MISMATCH, "Json string contains no evolution object");
    auto dobj = doc.object();
    auto keys = dobj.keys();
    for (auto& key : keys)
    {
        auto evo = createClass(key);
        if (!evo)
            continue;
        auto obj = dobj.value(key).toObject();
        if (obj.isEmpty())
        {
            evo->releaseRef();
            continue;
            //MO_IO_ERROR(VERSION_MISMATCH, "Value '" << key << "' "
            //            "in Json string contains no evolution object");
        }
        try
        {
            evo->deserialize(obj);
            return evo;
        }
        catch (...)
        {
            evo->releaseRef();
            throw;
        }
    }
    MO_IO_ERROR(VERSION_MISMATCH, "Json string contains no evolution object");
}

void EvolutionBase::toJson(QJsonDocument &doc) const
{
    QJsonObject obj;
    serialize(obj);

    QJsonObject dobj;
    dobj.insert(className(), obj);
    doc.setObject(dobj);

}

void EvolutionBase::saveJsonFile(const QString &fn)
{
    QJsonDocument doc;
    toJson(doc);

    QFile file(fn);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        MO_IO_ERROR(WRITE, "Could not open json file for saving\n'"
                    << fn << "'\n" << file.errorString());

    auto data = doc.toJson();
    if (file.write(data.constData(), data.size()) != data.size())
        MO_IO_ERROR(WRITE, "Could not write to json file\n'"
                    << fn << "'\n" << file.errorString());
}

EvolutionBase* EvolutionBase::loadJsonFile(const QString &fn)
{
    QFile file(fn);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        MO_IO_ERROR(READ, "Could not open json file\n'"
                    << fn << "'\n" << file.errorString());

    auto doc = QJsonDocument::fromJson(file.readAll());
    return fromJson(doc);
}

void EvolutionBase::saveImage(const QString &fn, const QImage &img)
{
    QImageWriter writer(fn);

    QJsonDocument doc;
    toJson(doc);
    writer.setText("Author", applicationName());
    writer.setDescription(QString::fromUtf8(doc.toJson()));

    if (!writer.write(img))
        MO_IO_ERROR(WRITE, "Error saving evolution image\n'"
                    << fn <<"'\n" << writer.errorString());
}

// ----- factory -----

std::map<QString, EvolutionBase*> EvolutionBase::p_reg_map_;

bool EvolutionBase::registerClass(EvolutionBase* evo)
{
    auto it = p_reg_map_.find(evo->className());
    if (it != p_reg_map_.end())
    {
        MO_WARNING("Duplicate evolution object registered '" << evo->className() << "'");
        return false;
    }
    p_reg_map_.insert(std::make_pair(evo->className(), evo));
    return true;
}

EvolutionBase* EvolutionBase::createClass(const QString& cname)
{
    auto it = p_reg_map_.find(cname);
    if (it == p_reg_map_.end())
    {
        MO_WARNING("Request for unknown evolution class '" << cname << "'");
        return nullptr;
    }
    return it->second->createClone();
}


MO_REGISTER_EVOLUTION(EvolutionVectorBase)

EvolutionVectorBase::EvolutionVectorBase(size_t size)
    : p_vec_    (size)
{
    properties().set("seed",
              QObject::tr("seed"),
              QObject::tr("Random initial seed"),
              42u);
    properties().set("mutation_amount",
              QObject::tr("mutation amount"),
              QObject::tr("Maximum change per mutation"),
              0.3, 0.025);
    properties().setMin("mutation_amount", 0.);
    properties().set("mutation_prob",
              QObject::tr("mutation probability"),
              QObject::tr("Probability of a random change"),
              0.1, 0., 1., 0.025);
    properties().set("grow_prob",
              QObject::tr("grow probability"),
              QObject::tr("Probability of a random insert"),
              0.01, 0., 1., 0.01);
    properties().set("shrink_prob",
              QObject::tr("shrink probability"),
              QObject::tr("Probability of a random deletion"),
              0.01, 0., 1., 0.01);

    properties().set("init_mean",
              QObject::tr("init mean"),
              QObject::tr("Mean value of random initialization"),
              0.0, 0.1);
    properties().set("init_var",
              QObject::tr("init variance"),
              QObject::tr("Range of random initialization"),
              1., 0.1);
    properties().setMin("init_var", 0.0);
    properties().set("init_dev",
              QObject::tr("init deviation"),
              QObject::tr("Distribution of random initialization, "
                          "close to mean (<1) or +/- variance (>1)"),
              1., 0.1);
    properties().setMin("init_dev", 0.0001);
}

void EvolutionVectorBase::serialize(QJsonObject& obj) const
{
    QJsonArray ar;
    for (auto v : vector())
        ar.append( v );
    obj.insert("vector", ar);
}

void EvolutionVectorBase::deserialize(const QJsonObject& obj)
{
    auto aro = obj.find("vector");
    if (aro == obj.end())
        MO_IO_ERROR(VERSION_MISMATCH, "Missing 'vector' in json evolution");
    if (!aro->isArray())
        MO_IO_ERROR(VERSION_MISMATCH, "'vector' of wrong type in json evolution");
    auto ar = aro->toArray();
    p_vec_.resize(ar.size());
    for (size_t i=0; i<p_vec_.size(); ++i)
        p_vec_[i] = ar[i].toDouble();
}


void EvolutionVectorBase::randomize()
{
    MATH::Twister rnd(properties().get("seed").toUInt());
    double  mean = properties().get("init_mean").toDouble(),
            var = properties().get("init_var").toDouble(),
            dev = 1./properties().get("init_dev").toDouble(),
            grow = properties().get("grow_prob").toDouble(),
            shrink = properties().get("shrink_prob").toDouble();

    for (auto& v : vector())
    {
        double r = rnd() - rnd();
        r = std::pow(std::abs(r), dev) * (r > 0. ? 1. : -1.);
        v = mean + var * r;
    }

    // grow
    for (size_t i=0; i<vector().size(); ++i)
    if (rnd() < grow)
    {
        double r = rnd() - rnd();
        r = std::pow(std::abs(r), dev) * (r > 0. ? 1. : -1.);
        r = mean + var * r;

        vector().insert(vector().begin() + i, r);
    }
    // shrink
    for (size_t i=0; i<vector().size(); ++i)
    if (rnd() < shrink)
        vector().erase(vector().begin() + i);
}

void EvolutionVectorBase::mutate()
{
    MATH::Twister rnd(properties().get("seed").toUInt());
    double  amt = properties().get("mutation_amount").toDouble(),
            prob = properties().get("mutation_prob").toDouble(),
            mean = properties().get("init_mean").toDouble(),
            var = properties().get("init_var").toDouble(),
            dev = 1./properties().get("init_dev").toDouble(),
            grow = properties().get("grow_prob").toDouble(),
            shrink = properties().get("shrink_prob").toDouble();

    for (auto& v : vector())
        if (rnd() < prob)
            v += amt * (rnd() - rnd());

    // grow
    for (size_t i=0; i<vector().size(); ++i)
    if (rnd() < grow)
    {
        double r = rnd() - rnd();
        r = std::pow(std::abs(r), dev) * (r > 0. ? 1. : -1.);
        r = mean + var * r;

        vector().insert(vector().begin() + i, r);
    }
    // shrink
    for (size_t i=0; i<vector().size(); ++i)
    if (rnd() < shrink)
        vector().erase(vector().begin() + i);
}

bool EvolutionVectorBase::canMate(const EvolutionBase* o) const
{
    return dynamic_cast<const EvolutionVectorBase*>(o);
}

void EvolutionVectorBase::mate(const EvolutionBase* otherBase)
{
    auto other = dynamic_cast<const EvolutionVectorBase*>(otherBase);
    if (!other)
        return;

    MATH::Twister rnd(properties().get("seed").toUInt());
    double range = rnd(1., 50.),
           phase = rnd(0., 6.28),
           amp = rnd(1.);
    size_t num = std::min(vector().size(), other->vector().size());

    for (size_t i=0; i<num; ++i)
    {
        double v1 = vector(i),
               v2 = other->vector(i),
               mx = amp * std::sin(double(i)/num * 3.14159265 * range + phase);

        p_vec_[i] = v1 + mx * (v2 - v1);
    }
}

void EvolutionVectorBase::getImage(QImage &img) const
{
    QPainter p;

    p.begin(&img);

        p.setRenderHint(QPainter::Antialiasing, true);

        p.fillRect(img.rect(), QColor(0,10,30));

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(200, 255, 200, 100));

        p.drawRect(QRect(0, img.height() / 2., img.width(), 1));

        p.setBrush(QColor(255, 255, 255, 50));
        double y1,y2, width = std::max(1., double(img.width()) / vector().size() - 1.);
        for (size_t i = 0; i < vector().size(); ++i)
        {
            double x = double(i) / vector().size() * img.width();
            double v = vector(i);
            if (v > 0.)
                y1 = (1.-v) * img.height() / 2., y2 = img.height() / 2;
            else
                y1 = img.height() / 2, y2 = (1.-v) * img.height() / 2.;
            p.drawRect(QRectF(x, y1, width, y2-y1));
        }

    p.end();
}



MO_REGISTER_EVOLUTION(KaliSetEvolution)

KaliSetEvolution::KaliSetEvolution()
    : EvolutionVectorBase(50)
{

}

void KaliSetEvolution::getImage(QImage &img) const
{
    for (int j=0; j<img.height(); ++j)
    {
        double v = 1. - double(j) / (img.height()-1) * 2.;
        for (int i=0; i<img.width(); ++i)
        {
            double u = double(i) / (img.width()-1) * 2. - 1.;
            double r,g,b;
            getRgb(u, v, &r, &g, &b);
            img.setPixel(i, j, qRgb(
                         255 * std::max(0.,std::min(1., r)),
                         255 * std::max(0.,std::min(1., g)),
                         255 * std::max(0.,std::min(1., b))
                         ));
        }
    }
}

int KaliSetEvolution::numIter() const { return (vector().size()-14) / 3; }

void KaliSetEvolution::getRgb(double u, double v, double *r, double *g, double *b) const
{
    size_t P = 0;
    double  colAccX = vector(P++),
            colAccY = vector(P++),
            colAccZ = vector(P++),
            minAccX = vector(P++),
            minAccY = vector(P++),
            minAccZ = vector(P++),
            minX = vector(P++),
            minY = vector(P++),
            minZ = vector(P++),
            minExp = 20. * std::abs(vector(P++)),
            scale = std::pow(vector(P++), 2.),
    // start pos + random scale and offset
            poX = vector(P++) + u * scale,
            poY = vector(P++) + v * scale,
            poZ = vector(P++),
            colX = 0., colY = 0., colZ = 0.,
            md = 1000.;

    for (int i=0; i<numIter(); ++i)
    {
        // kali set (first half)
        double dot = poX * poX + poY * poY + poZ * poZ;
        poX = std::abs(poX) / dot;
        poY = std::abs(poY) / dot;
        poZ = std::abs(poZ) / dot;

        // accumulate some values
        colX += colAccX * poX;
        colY += colAccY * poY;
        colZ += colAccZ * poZ;
        dot = minAccX * poX + minAccY * poY + minAccZ * poZ;
        md = std::min(md, std::abs(dot));

        // kali set (second half)
        if (i != numIter() - 1)
        {
            // (a different magic param for each iteration step!)
            poX -= std::abs(vector(P++));
            poY -= std::abs(vector(P++));
            poZ -= std::abs(vector(P++));
        }
    }
    // average color
    colX = std::abs(colX) / double(numIter());
    colY = std::abs(colY) / double(numIter());
    colZ = std::abs(colZ) / double(numIter());

    // "min-distance stripes" or "orbit traps"
    md = std::pow(1. - md, minExp);
    colX += minX * md;
    colY += minY * md;
    colZ += minZ * md;

    // mix-in color from last iteration step
    //vec3 col2 = po * abs(dot(po, parameter().xyz));
    //col += (col2 - col) * 0.2 * abs(parameter().x);

    *r = colX;
    *g = colY;
    *b = colZ;
}

QString KaliSetEvolution::toString() const
{
    QString str;
    QTextStream s(&str);

    size_t P = 0;
    s <<"/* uv is in [-1, 1] */\n"
        "vec3 kaliSet(in vec2 uv)\n"
        "{\n"
        "\tconst vec3 colAcc = vec3("
            << vector(P) << ", " << vector(P+1) << ", " << vector(P+2) << ");\n";
    P += 3;
    s <<"\tconst vec3 minAcc = vec3("
            << vector(P) << ", " << vector(P+1) << ", " << vector(P+2) << ");\n";
    P += 3;
    s <<"\tconst vec3 minCol = vec3("
            << vector(P) << ", " << vector(P+1) << ", " << vector(P+2) << ");\n"
        "\tconst float minExp = " << (20. * std::abs(vector(P+3))) << ";\n"
        "\tconst float scale = " << std::pow(vector(P+4), 2.) << ";\n";
    P += 5;
    s <<"\n\t// start pos + random scale and offset\n"
        "\tvec3 po = vec3(uv, 0.) * scale\n"
        "\t          + vec3("
            << vector(P) << ", " << vector(P+1) << ", " << vector(P+2) << ");\n";
    P += 3;
    s <<"\n\tvec3 col = vec3(0.);\n"
        "\tfloat md = 1000.;\n"
        "\tconst int numIter = " << numIter() << ";\n"
        "\n\tfor (int i=0; i<numIter; ++i)\n"
        "\t{\n"
        "\t\t// kali set (first half)\n"
        "\t\tpo = abs(po.xyz) / dot(po, po);\n"
        "\n"
        "\t\t// accumulate some values\n"
        "\t\tcol += colAcc * po;\n"
        "\t\tmd = min(md, abs(dot(minAcc, po)));\n"
        "\n"
        "\t\t// kali set (second half)\n";
    for (int i=0; i<numIter()-1; ++i)
    {
        s << "\t\tif (i == " << i << ") po -= vec3("
             << std::abs(vector(P)) << ", "
             << std::abs(vector(P+1)) << ", "
             << std::abs(vector(P+2)) << ");\n";
        P += 3;
    }
    s <<"\t}\n\t// average color\n"
        "\tcol = abs(col) / float(numIter);\n"
        "\n\t// 'min-distance stripes' or 'orbit traps'\n"
        "\tmd = pow(1. - md, minExp);\n";
    s <<"\tcol += md * minCol;\n"
        "\n\treturn col;\n"
        "}\n";
    return str;
}

} // namespace MO
