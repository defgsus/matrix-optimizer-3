/** @file irmap.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 29.04.2015</p>
*/

#include <QObject> // for tr()
#include <QPainter>

#include "irmap.h"


namespace MO {
namespace AUDIO {


IrMap::IrMap()
{
    clear();
}

QString IrMap::getInfo() const
{
    return QObject::tr("samples: %1\namp: %2 - %3\nlength: %4 - %5")
                   .arg(p_map_.size())
                   .arg(p_min_amp_)
                   .arg(p_max_amp_)
                   .arg(p_min_dist_)
                   .arg(p_max_dist_);
}

QImage IrMap::getImage(const QSize &res)
{
    QImage img(res, QImage::Format_ARGB32);
    img.fill(Qt::black);

    if (p_map_.empty())
        return img;

    QPainter p(&img);
    //p.begin(&img);

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(QColor(255,255,255,50)));

    const Float
            invDist = Float(img.width()) / std::max(0.000001f, p_max_dist_),
            invAmp = -.5f * img.height() / std::max(0.000001f, p_max_amp_);

    // draw
    for (auto it = p_map_.begin(); it != p_map_.end(); ++it)
    {
        QPointF p0(it->first * invDist, .5 * img.height()),
                p1(p0.x(), p0.y() + it->second * invAmp);
        p.drawLine(p0, p1);
    }

    p.end();
    return img;
}


void IrMap::clear()
{
    p_map_.clear();
    p_max_amp_ = p_min_amp_ = p_max_dist_ = p_min_dist_ = 0.f;
}

void IrMap::addSample(Float distance, Float amplitude)
{
    if (p_map_.empty())
    {
        p_max_amp_ = p_min_amp_ = amplitude;
        p_max_dist_ = p_min_dist_ = distance;
        p_map_.insert(std::make_pair(distance, amplitude));
        return;
    }

    if (p_map_.find(distance) == p_map_.end())
    {
        p_min_amp_ = std::min(p_min_amp_, std::abs(amplitude));
        p_max_amp_ = std::max(p_max_amp_, std::abs(amplitude));
        p_min_dist_ = std::min(p_min_dist_, distance);
        p_max_dist_ = std::max(p_max_dist_, distance);

        p_map_.insert(std::make_pair(distance, amplitude));
    }
}

} // namespace AUDIO
} // namespace MO
