/** @file grid.h

    @brief grid painter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/24/2014</p>
*/

#ifndef MOSRC_GUI_PAINTER_GRID_H
#define MOSRC_GUI_PAINTER_GRID_H

#include <set>

#include <QObject>
#include <QPen>

#include "gui/util/viewspace.h"

class QRect;
class QSize;
class QPainter;

namespace MO {
namespace GUI {
namespace PAINTER {

class Grid : public QObject
{
    Q_OBJECT

public:

    enum Option
    {
        O_DrawX = 1,
        O_DrawY = 2,
        O_DrawTextX = 4,
        O_DrawTextY = 8,

        O_DrawAll = 0xffff
    };

    enum GridSpacing
    {
        GS_SECONDS,
        GS_INTEGER,
        GS_SMALL_AND_LARGE
    };

    // -------------- types -----------------

    Grid(QObject * parent = 0);

    // -------------- getter ----------------

    Double quantizeX(Double x) const;
    Double quantizeY(Double y) const;

    int options() const { return options_; }

    const UTIL::ViewSpace& viewSpace() const { return viewspace_; }

    // -------------- setter ----------------

    void setOptions(int options) { options_ = options; }

    void setViewSpace(const UTIL::ViewSpace& viewspace) { viewspace_ = viewspace; }

    void setSpacingX(GridSpacing);
    void setSpacingY(GridSpacing);

    // ----------- draw action --------------

    void paint(QPainter & p);

    void paint(QPainter & p, const QRect& rect);

protected:

    void setSpacing_(std::set<Double>&, GridSpacing);

    UTIL::ViewSpace viewspace_;

    std::set<Double> spacingX_, spacingY_;

    int options_;
    GridSpacing spacingXMode_, spacingYMode_;

    QPen pen_, penCenter_, penText_;
};

} // namespace PAINTER
} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_PAINTER_GRID_H
