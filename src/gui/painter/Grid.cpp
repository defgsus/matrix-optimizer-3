/** @file grid.h

    @brief grid painter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/24/2014</p>
*/

#include <QDebug>
#include <QPainter>

#include "Grid.h"
#include "math/functions.h"

namespace MO {
namespace GUI {
namespace PAINTER {

Grid::Grid(QObject *parent)
    :   QObject     (parent),
        options_    (O_DrawAll),
        pen_        (QColor(255,255,255,50)),
        penCenter_  (QColor(255,255,255,200)),
        penText_    (QColor(255,255,255,150))
{
    setSpacingX(GS_SECONDS);
    setSpacingY(GS_SMALL_AND_LARGE);
}

void Grid::setSpacingX(GridSpacing s)
{
    setSpacing_(spacingX_, s);
}

void Grid::setSpacingY(GridSpacing s)
{
    setSpacing_(spacingY_, s);
}

void Grid::setSpacing_(std::set<Double> & set, GridSpacing s)
{
    set.clear();

    if (s == GS_SECONDS)
    {
        set.insert(0.0010);
        //set.insert(0.0025);
        set.insert(0.005);
        set.insert(0.01);
        //set.insert(0.02);
        //set.insert(0.025);
        set.insert(0.05);
        set.insert(0.1);
        //set.insert(0.25);
        set.insert(0.5);
        set.insert(1.0);
        set.insert(2.0);
        set.insert(5.0);
        set.insert(10.0);
        set.insert(20.0);
        set.insert(30.0);
        set.insert(60.0);
        set.insert(120.0);
        set.insert(240.0);
        set.insert(360.0);
        set.insert(720.0);
    }

    if (s == GS_SMALL_AND_LARGE)
    {
        set.insert(0.000001);
        set.insert(0.000025);
        set.insert(0.00005);
        set.insert(0.0001);
        set.insert(0.00025);
        set.insert(0.0005);
        set.insert(0.001);
        set.insert(0.0025);
        set.insert(0.005);
        set.insert(0.01);
        set.insert(0.025);
        set.insert(0.05);
        set.insert(0.1);
        set.insert(0.25);
        set.insert(0.5);
    }

    if (s == GS_SMALL_AND_LARGE || s == GS_INTEGER)
    {
        set.insert(1.0);
        set.insert(2.0);
        set.insert(5.0);
        set.insert(10.0);
        set.insert(20.0);
        set.insert(50.0);
        set.insert(100.0);
        set.insert(200.0);
        set.insert(500.0);
        set.insert(1000.0);
        set.insert(2000.0);
        set.insert(5000.0);
        set.insert(10000.0);
        set.insert(20000.0);
        set.insert(50000.0);
        set.insert(100000.0);
        set.insert(200000.0);
        set.insert(500000.0);
        set.insert(1000000.0);
    }
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
    p.setBrush(Qt::NoBrush);

    if ((options_ & O_DrawX) || (options_ & O_DrawTextX))
    {
        const int width = p.window().width();

        const Double
        // find meaningful spacing approximately at 20 pixels
            spacing = quantizeX( viewspace_.mapXDistanceTo((Double)20 / width) ),
        // start and end of screen in mapped space
            x0 = viewspace_.mapXTo((Double)rect.left() / width),
            x1 = viewspace_.mapXTo((Double)rect.right() / width);

        if (spacing > 0)
        {
            // draw nice lines
            if (options_ & O_DrawX)
            {
                p.setPen(pen_);
                for (Double x = x0; x<x1+spacing; x += spacing)
                {
                    const Double qx = MATH::quant(x,spacing);
                    const int sx = viewspace_.mapXFrom(qx) * width;

                    if (qx == 0)
                        p.setPen(penCenter_);
                    p.drawLine(sx, rect.top(), sx, rect.bottom());
                    if (qx == 0)
                        p.setPen(pen_);
                }
            }

            // draw text
            if ((options_ & O_DrawTextX) && rect.top() < 40)
            {
                p.setPen(penText_);
                const int fh = p.fontMetrics().height() * 0.8;
                // determine start offset of text
                int hs = (int)(x0/spacing) % 2;
                if (x0>=0)
                    hs = (hs + 1) % 2;
                for (Double x = x0-spacing; x<x1+spacing; x += spacing)
                {
                    const Double qx = MATH::quant(x,spacing);
                    const int sx = viewspace_.mapXFrom(qx) * width;

                    p.drawText(sx+2, (hs+1)*fh, QString::number(qx));

                    hs = (hs + 1) % 2;
                }
            }
        }
    }

    if ((options_ & O_DrawY) || (options_ & O_DrawTextY))
    {
        const int height = p.window().height();

        const Double
            spacing = quantizeY( viewspace_.mapYDistanceTo((Double)20 / height) ),
            y1 = viewspace_.mapYTo((Double)(height-1-rect.top()) / height),
            y0 = viewspace_.mapYTo((Double)(height-1-rect.bottom()) / height);

        if (spacing > 0)
        {
            // grid lines
            if (options_ & O_DrawY)
            {
                p.setPen(pen_);
                for (Double y = y0; y<y1+spacing; y += spacing)
                {
                    const Double qy = MATH::quant(y,spacing);
                    const int sy = height - 1 - viewspace_.mapYFrom(qy) * height;

                    if (qy == 0)
                        p.setPen(penCenter_);
                    p.drawLine(rect.left(), sy, rect.right(), sy);
                    if (qy == 0)
                        p.setPen(pen_);
                }
            }

            // text
            if (options_ & O_DrawTextY)
            {
                p.setPen(penText_);
                for (Double y = y0; y<y1+spacing; y += spacing)
                {
                    const Double qy = MATH::quant(y,spacing);
                    const int sy = height - 1 - viewspace_.mapYFrom(qy) * height;

                    p.drawText(0, sy-1, QString::number(qy));
                }
            }
        }
    }
}


} // namespace PAINTER
} // namespace GUI
} // namespace MO

