/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/18/2016</p>
*/

#include <QImage>
#include <QTextStream>
#include <QJsonObject>

#include "KalisetEvolution.h"

namespace MO {

MO_REGISTER_EVOLUTION(KaliSetEvolution)

KaliSetEvolution::KaliSetEvolution()
    : EvolutionVectorBase(50, true, 20)
{

}

void KaliSetEvolution::serialize(QJsonObject& o) const
{
    EvolutionVectorBase::serialize(o);
    o.insert("version", 2);
}

void KaliSetEvolution::deserialize(const QJsonObject& o)
{
    EvolutionVectorBase::deserialize(o);
    int ver = o.value("version").toInt();

    // added params in v2
    if (ver < 2)
        vector().insert(vector().begin() + 10, { 0., 0., 0. });
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

void KaliSetEvolution::getRgb(double u, double v, double *r, double *g, double *b) const
{
    double
    // screen pos + random scale and offset
            poX = pPosX() + u * pScale(),
            poY = pPosY() + v * pScale(),
            poZ = pPosZ(),
            colX = 0., colY = 0., colZ = 0.,
            md = 1000.;

    for (int i=0; i<pNumIter(); ++i)
    {
        // kali set (first half)
        double dot = poX * poX + poY * poY + poZ * poZ;
        poX = std::abs(poX) / dot;
        poY = std::abs(poY) / dot;
        poZ = std::abs(poZ) / dot;

        // accumulate some values
        colX += pColAccX() * poX;
        colY += pColAccY() * poY;
        colZ += pColAccZ() * poZ;
        dot = pMinAccX() * poX + pMinAccY() * poY + pMinAccZ() * poZ;
        md = std::min(md, std::abs(dot));

        // kali set (second half)
        if (i != pNumIter() - 1)
        {
            // (a different magic param for each iteration step!)
            poX -= pMagicX(i);
            poY -= pMagicY(i);
            poZ -= pMagicZ(i);
        }
    }
    // average color
    colX = std::abs(colX) / double(pNumIter());
    colY = std::abs(colY) / double(pNumIter());
    colZ = std::abs(colZ) / double(pNumIter());

    // "min-distance stripes" or "orbit traps"
    md = std::pow(1. - md, pMinExp());
    colX += pMinAmtX() * md;
    colY += pMinAmtY() * md;
    colZ += pMinAmtZ() * md;

    // mix-in color from last iteration step
    colX += poX * pAmtX();
    colY += poY * pAmtY();
    colZ += poZ * pAmtZ();

    *r = colX;
    *g = colY;
    *b = colZ;
}

QString KaliSetEvolution::toString() const
{
    QString str;
    QTextStream s(&str);

    s <<"/* uv is in [-1, 1] */\n"
        "vec3 kaliSet(in vec2 uv)\n"
        "{\n"
        "\tconst vec3 colAcc = vec3("
            << pColAccX() << ", " << pColAccY() << ", " << pColAccZ() << ");\n"
        "\tconst vec3 minAcc = vec3("
            << pMinAccX() << ", " << pMinAccY() << ", " << pMinAccZ() << ");\n"
        "\tconst vec3 minCol = vec3("
            << pMinAmtX() << ", " << pMinAmtY() << ", " << pMinAmtZ() << ");\n"
        "\tconst float minExp = " << pMinExp() << ";\n"
        "\tconst float scale = " << pScale() << ";\n"
        "\n\t// start pos + random scale and offset\n"
        "\tvec3 po = vec3(uv, 0.) * scale\n"
        "\t          + vec3("
            << pPosX() << ", " << pPosY() << ", " << pPosZ() << ");\n"
        "\n\tvec3 col = vec3(0.);\n"
        "\tfloat md = 1000.;\n"
        "\tconst int numIter = " << pNumIter() << ";\n"
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
    for (int i=0; i<pNumIter()-1; ++i)
    {
        s << "\t\tif (i == " << i << ") po -= vec3("
          << pMagicX(i) << ", " << pMagicY(i) << ", " << pMagicZ(i) << ");\n";
    }
    s <<"\t}\n\t// average color\n"
        "\tcol = abs(col) / float(numIter);\n"
        "\n\t// 'min-distance stripes' or 'orbit traps'\n"
        "\tmd = pow(1. - md, minExp);\n"
        "\n"
        "\t// mix-in color from last iteration step\n"
        "\tcol += po * vec3("
             << pAmtX() << ", " << pAmtY() << ", " << pAmtZ() << ");\n"
        "\n"
        "\tcol += md * minCol;\n"
        "\n\treturn col;\n"
        "}\n";
    return str;
}


} // namespace MO
