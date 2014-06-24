/** @file grid.h

    @brief grid painter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/24/2014</p>
*/

#ifndef MO_GUI_PAINTER_GRID_H
#define MO_GUI_PAINTER_GRID_H

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
    Grid(QObject * parent = 0);

    // -------------- getter ----------------

    Double quantizeX(Double x) const;
    Double quantizeY(Double y) const;

    bool isDrawX() const { return doDrawX_; }
    bool isDrawY() const { return doDrawY_; }

    // -------------- setter ----------------

    void setDrawX(bool doit) { doDrawX_ = doit; }
    void setDrawY(bool doit) { doDrawY_ = doit; }

    template <typename F>
    void setViewSpace(const UTIL::ViewSpace<F>& viewspace) { viewspace_ = viewspace; }

    // ----------- draw action --------------

    void paint(QPainter & p);

    void paint(QPainter & p, const QRect& rect);

protected:

    UTIL::ViewSpace<Double> viewspace_;

    std::set<Double> spacingX_, spacingY_;

    QPen pen_;

    bool doDrawX_, doDrawY_;
};

} // namespace PAINTER
} // namespace GUI
} // namespace MO


#endif // MO_GUI_PAINTER_GRID_H
