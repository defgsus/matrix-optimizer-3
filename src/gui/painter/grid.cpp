/** @file grid.h

    @brief grid painter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/24/2014</p>
*/

#include <QDebug>
#include <QPainter>

#include "grid.h"

namespace MO {
namespace GUI {
namespace PAINTER {

Grid::Grid(QObject *parent)
    :   QObject(parent)
{
}

void Grid::paint(QPainter & p)
{
    paint(p, p.window());
}

void Grid::paint(QPainter &p, const QRect &rect)
{
    p.setPen(QPen(Qt::white));

    Double
        x0 = viewspace_.mapXTo((Double)rect.left() / rect.width()),
        x1 = viewspace_.mapXTo((Double)rect.right() / rect.width()),
        spacing = 0.5;

    for (Double x = x0; x<=x1; x += spacing)
    {
        int sx = viewspace_.mapXFrom(x) * rect.width();

        p.drawLine(sx, 0, sx, rect.height());
    }
}


} // namespace PAINTER
} // namespace GUI
} // namespace MO

