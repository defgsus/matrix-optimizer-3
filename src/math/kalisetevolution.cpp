/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/18/2016</p>
*/

#include <QImage>
#include <QTextStream>

#include "kalisetevolution.h"

namespace MO {

MO_REGISTER_EVOLUTION(KaliSetEvolution)

KaliSetEvolution::KaliSetEvolution()
    : EvolutionVectorBase(50, true)
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
