/** @file grid.h

    @brief grid painter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/24/2014</p>
*/

#include <QDebug>
#include <QPainter>

#include "grid.h"
#include "math/functions.h"

namespace MO {
namespace GUI {
namespace PAINTER {

Grid::Grid(QObject *parent)
    :   QObject     (parent),
        pen_        (QColor(255,255,255,50)),
        doDrawX_    (true),
        doDrawY_    (true)
{

    spacingX_.insert(0.0010);
    spacingX_.insert(0.0025);
    spacingX_.insert(0.005);
    spacingX_.insert(0.01);
    spacingX_.insert(0.02);
    spacingX_.insert(0.025);
    spacingX_.insert(0.05);
    spacingX_.insert(0.1);
    spacingX_.insert(0.25);
    spacingX_.insert(0.5);
    spacingX_.insert(1.0);
    spacingX_.insert(2.0);
    spacingX_.insert(5.0);
    spacingX_.insert(10.0);
    spacingX_.insert(20.0);
    spacingX_.insert(30.0);
    spacingX_.insert(60.0);
    spacingX_.insert(120.0);
    spacingX_.insert(240.0);
    spacingX_.insert(360.0);
    spacingX_.insert(720.0);

    spacingY_.insert(0.000001);
    spacingY_.insert(0.000025);
    spacingY_.insert(0.00005);
    spacingY_.insert(0.0001);
    spacingY_.insert(0.00025);
    spacingY_.insert(0.0005);
    spacingY_.insert(0.001);
    spacingY_.insert(0.0025);
    spacingY_.insert(0.005);
    spacingY_.insert(0.01);
    spacingY_.insert(0.025);
    spacingY_.insert(0.05);
    spacingY_.insert(0.1);
    spacingY_.insert(0.25);
    spacingY_.insert(0.5);
    spacingY_.insert(1.0);
    spacingY_.insert(2.0);
    spacingY_.insert(5.0);
    spacingY_.insert(10.0);
    spacingY_.insert(20.0);
    spacingY_.insert(50.0);
    spacingY_.insert(100.0);
    spacingY_.insert(200.0);
    spacingY_.insert(500.0);
    spacingY_.insert(1000.0);
    spacingY_.insert(2000.0);
    spacingY_.insert(5000.0);
    spacingY_.insert(10000.0);
    spacingY_.insert(20000.0);
    spacingY_.insert(50000.0);
    spacingY_.insert(100000.0);
    spacingY_.insert(200000.0);
    spacingY_.insert(500000.0);
    spacingY_.insert(1000000.0);

}

Double Grid::quantizeX(Double x) const
{
    auto it = spacingX_.lower_bound(x);

    if (it != spacingX_.end())
        return *it;

    return x;
}

Double Grid::quantizeY(Double y) const
{
    auto it = spacingY_.lower_bound(y);

    if (it != spacingY_.end())
        return *it;

    return y;
}

void Grid::paint(QPainter & p)
{
    paint(p, p.window());
}

void Grid::paint(QPainter &p, const QRect &rect)
{
    if (!doDrawX_ && !doDrawY_)
        return;

    p.setPen(pen_);

    if (doDrawX_)
    {
        const int width = p.window().width();

        const Double
        // find meaningful spacing approximately at 20 pixels
            spacing = quantizeX( viewspace_.mapXDistanceTo((Double)20 / width) ),
        // start and end of screen in mapped space
            x0 = viewspace_.mapXTo((Double)rect.left() / width),
            x1 = viewspace_.mapXTo((Double)rect.right() / width);

        // draw nice lines
        if (spacing > 0)
        for (Double x = x0; x<x1+spacing; x += spacing)
        {
            const int sx = viewspace_.mapXFrom(MATH::quant(x,spacing)) * width;

            p.drawLine(sx, rect.top(), sx, rect.bottom());
        }
    }

    if (doDrawY_)
    {
        const int height = p.window().height();

        const Double
            spacing = quantizeY( viewspace_.mapYDistanceTo((Double)20 / height) ),
            y0 = viewspace_.mapYTo((Double)rect.top() / height),
            y1 = viewspace_.mapYTo((Double)rect.bottom() / height);

        if (spacing > 0)
        for (Double y = y0; y<y1+spacing; y += spacing)
        {
            const int sy = viewspace_.mapYFrom(MATH::quant(y,spacing)) * height;

            p.drawLine(rect.left(), sy, rect.right(), sy);
        }
    }
}


} // namespace PAINTER
} // namespace GUI
} // namespace MO

