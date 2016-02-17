/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#include <QImage>
#include <QPainter>

#include "evolutionbase.h"
#include "math/random.h"
#include "types/properties.h"

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
    properties().set("mutation_prob",
              QObject::tr("mutation probability"),
              QObject::tr("Probability of a random change"),
              0.1, 0., 1., 0.025);

    properties().set("init_mean",
              QObject::tr("init mean"),
              QObject::tr("Mean value of random initialization"),
              0.0, 0.1);
    properties().set("init_var",
              QObject::tr("init variance"),
              QObject::tr("Range of random initialization"),
              1., 0.1);
    properties().set("init_dev",
              QObject::tr("init deviation"),
              QObject::tr("Distribution of random initialization, "
                          "close to mean (<1) or +/- variance (>1)"),
              1., 0.1);
}

void EvolutionVectorBase::randomize()
{
    MATH::Twister rnd(properties().get("seed").toUInt());
    double  mean = properties().get("init_mean").toDouble(),
            var = properties().get("init_var").toDouble(),
            dev = 1./properties().get("init_dev").toDouble();
    for (auto& v : vector())
    {
        double r = rnd() - rnd();
        r = std::pow(std::abs(r), dev) * (r > 0. ? 1. : -1.);
        v = mean + var * r;
    }
}

void EvolutionVectorBase::mutate()
{
    MATH::Twister rnd(properties().get("seed").toUInt());
    double  amt = properties().get("mutation_amount").toDouble(),
            prob = properties().get("mutation_prob").toDouble();
    for (auto& v : vector())
        if (rnd() < prob)
            v += amt * (rnd() - rnd());
}

bool EvolutionVectorBase::mate(const EvolutionBase* otherBase)
{
    auto other = dynamic_cast<const EvolutionVectorBase*>(otherBase);
    if (!other)
        return false;

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
    return true;
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



EvolutionKaliCpu::EvolutionKaliCpu()
    : EvolutionVectorBase(50)
{

}

void EvolutionKaliCpu::getImage(QImage &img) const
{
    for (int j=0; j<img.height(); ++j)
    {
        double v = double(j) / (img.height()-1) * 2. - 1.;
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

void EvolutionKaliCpu::getRgb(double u, double v, double *r, double *g, double *b) const
{
    size_t P = 0;
    double  colAccX = vector(P++),
            colAccY = vector(P++),
            colAccZ = vector(P++),
            minAccX = vector(P++),
            minAccY = vector(P++),
            minAccZ = vector(P++),
            colX = 0., colY = 0., colZ = 0.,
            scale = std::pow(vector(P++), 2.),
    // start pos + random scale and offset
            poX = vector(P++) + u * scale,
            poY = vector(P++) + v * scale,
            poZ = vector(P++),
            md = 1000.;

    const int numIter = 13;
    for (int i=0; i<numIter; ++i)
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
        if (i != numIter - 1)
        {
            // (a different magic param for each iteration step!)
            poX -= std::abs(vector(P++));
            poY -= std::abs(vector(P++));
            poZ -= std::abs(vector(P++));
        }
    }
    // average color
    colX = std::abs(colX) / double(numIter);
    colY = std::abs(colY) / double(numIter);
    colZ = std::abs(colZ) / double(numIter);

    // "min-distance stripes" or "orbit traps"
    md = std::pow(1. - md, 20. * std::abs(vector(P++)));
    colX += vector(P++) * md;
    colY += vector(P++) * md;
    colZ += vector(P++) * md;

    // mix-in color from last iteration step
    //vec3 col2 = po * abs(dot(po, parameter().xyz));
    //col += (col2 - col) * 0.2 * abs(parameter().x);

    *r = colX;
    *g = colY;
    *b = colZ;
}

} // namespace MO

