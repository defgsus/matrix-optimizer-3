/** @file valuecurve.h

    @brief generic painter for f(x)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_PAINTER_VALUECURVE_H
#define MOSRC_GUI_PAINTER_VALUECURVE_H

#include <functional>

#include <QObject>
#include <QPen>

#include "types/float.h"
#include "gui/util/ViewSpace.h"

class QRect;
class QSize;
class QPainter;

namespace MO {
namespace GUI {
namespace PAINTER {

/** Derive from this class to input f(x) data to ValueCurve painter */
class ValueCurveData
{
public:
    virtual ~ValueCurveData() { }

    virtual Double value(Double time) const = 0;
};


class ValueCurve : public QObject
{
    Q_OBJECT

public:

    ValueCurve(QObject * parent = 0);

    // -------------- getter ----------------

    const ValueCurveData * curveData() const { return data_; }

    /** Returns the currently set pen for the curve */
    QPen pen() const { return pen_; }

    /** Returns the number of samples per pixel */
    int overpaint() const { return overPaint_; }

    // -------------- setter ----------------

    /** Set the curve data. Ownership stays with the caller. */
    void setCurveData(const ValueCurveData * data) { func_ = 0; data_ = data; }

    /** Sets a callback for retrieving the value, e.g. f(x) */
    void setCurveFunction(std::function<Double(Double)> func) { data_ = 0; func_ = func; }

    /** Sets the viewspace for the whole painter area given to paint() */
    void setViewSpace(const UTIL::ViewSpace& viewspace) { viewspace_ = viewspace; }

    void setPen(const QPen& pen) { pen_ = pen; }

    /** Sets the alpha channel of the Pen. */
    void setAlpha(int alpha);

    /** Sets the number of samples per pixel */
    void setOverpaint(int num) { overPaint_ = num; }

    // ----------- draw action --------------

    /** Paints over the whole area of @p p */
    void paint(QPainter & p);

    /** Paints the area specified by @p updateArea using the whole area of p as viewspace */
    void paint(QPainter & p, const QRect& updateArea);

    /** Paints the curve into rectangle @p rect within the range of @p updateArea */
    void paint(QPainter & p, const QRect& rect, const QRect& updateArea);

protected:

    const ValueCurveData * data_;
    std::function<Double(Double)> func_;

    UTIL::ViewSpace viewspace_;

    QPen pen_;

    int overPaint_;
};

} // namespace PAINTER
} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_PAINTER_VALUECURVE_H
