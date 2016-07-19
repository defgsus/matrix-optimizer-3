/** @file sequenceoverpaint.h

    @brief Painter for Sequence boundaries

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_PAINTER_SEQUENCEOVERPAINT_H
#define MOSRC_GUI_PAINTER_SEQUENCEOVERPAINT_H

#include <QObject>
#include <QPen>
#include <QBrush>

#include "types/float.h"
#include "gui/util/ViewSpace.h"

class QRect;
class QSize;
class QPainter;

namespace MO {
class Sequence;
namespace GUI {
namespace PAINTER {


class SequenceOverpaint : public QObject
{
    Q_OBJECT

public:

    SequenceOverpaint(QObject * parent = 0);

    // -------------- getter ----------------

    const Sequence * sequence() const { return sequence_; }

    QRect playBarRect(const QRect& widget) const;

    // -------------- setter ----------------

    void setSequence(const Sequence * s) { sequence_ = s; }

    /** Sets the viewspace for the whole painter area given to paint() */
    void setViewSpace(const UTIL::ViewSpace& viewspace) { viewspace_ = viewspace; }

    // ----------- draw action --------------

    /** Paints over the whole area of @p p */
    void paint(QPainter & p);

    /** Paints the area specified by @p rect using the whole area of p as viewspace */
    void paint(QPainter & p, const QRect& rect);

protected:

    const Sequence * sequence_;

    UTIL::ViewSpace viewspace_;

    QPen penLoop_;
    QBrush brushOutside_;

};

} // namespace PAINTER
} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_PAINTER_SEQUENCEOVERPAINT_H
