/** @file valuecurve.h

    @brief generic painter for f(x)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_PAINTER_VALUECURVE_H
#define MOSRC_GUI_PAINTER_VALUECURVE_H

#include <QObject>
#include <QPen>

#include "types/float.h"
#include "gui/util/viewspace.h"

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

    // -------------- setter ----------------

    /** Set the curve data. Ownership stays with the caller. */
    void setCurveData(const ValueCurveData * data) { data_ = data; }

    /** Sets the viewspace for the whole painter area given to paint() */
    void setViewSpace(const UTIL::ViewSpace& viewspace) { viewspace_ = viewspace; }

    // ----------- draw action --------------

    /** Paints over the whole area of @p p */
    void paint(QPainter & p);

    /** Paints the area specified by @p rect using the whole area of p as viewspace */
    void paint(QPainter & p, const QRect& rect);

protected:

    const ValueCurveData * data_;

    UTIL::ViewSpace viewspace_;

    QPen pen_;

    int overPaint_;
};

} // namespace PAINTER
} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_PAINTER_VALUECURVE_H
