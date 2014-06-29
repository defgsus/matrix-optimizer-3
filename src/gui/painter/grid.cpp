/** @file grid.h

    @brief grid painter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

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
        textPen_    (QColor(255,255,255,150)),
        options_    (O_DrawAll)
{

    spacingX_.insert(0.0010);
    //spacingX_.insert(0.0025);
    spacingX_.insert(0.005);
    spacingX_.insert(0.01);
    //spacingX_.insert(0.02);
    //spacingX_.insert(0.025);
    spacingX_.insert(0.05);
    spacingX_.insert(0.1);
    //spacingX_.insert(0.25);
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

                    p.drawLine(sx, rect.top(), sx, rect.bottom());
                }
            }

            // draw text
            if ((options_ & O_DrawTextX) && rect.top() < 40)
            {
                p.setPen(textPen_);
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
                    const int sy = height - 1 - viewspace_.mapYFrom(MATH::quant(y,spacing)) * height;

                    p.drawLine(rect.left(), sy, rect.right(), sy);
                }
            }

            // text
            if (options_ & O_DrawTextY)
            {
                p.setPen(textPen_);
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

