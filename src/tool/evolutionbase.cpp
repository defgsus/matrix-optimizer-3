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

namespace MO {



void EvolutionVectorBase::randomize(const MutationSettings* ms)
{
    MATH::Twister rnd(ms->seed);
    for (auto& v : vector())
        v = ms->mean + ms->deviation * (rnd() - rnd());
}

void EvolutionVectorBase::mutate(const MutationSettings* ms)
{
    MATH::Twister rnd(ms->seed);
    for (auto& v : vector())
        if (rnd() < ms->probability)
            v += ms->amount * (rnd() - rnd());
}

bool EvolutionVectorBase::mate(const MutationSettings* ms, const EvolutionBase* otherBase)
{
    auto other = dynamic_cast<const EvolutionVectorBase*>(otherBase);
    if (!other)
        return false;

    MATH::Twister rnd(ms->seed);
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


} // namespace MO

