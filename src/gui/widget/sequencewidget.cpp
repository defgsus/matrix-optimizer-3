/** @file sequencewidget.cpp

    @brief Widget for display MO::Sequence in MO::GUI::TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

//#include <QDebug>

#include <QPainter>
#include <QPaintEvent>

#include "sequencewidget.h"
#include "io/log.h"
#include "gui/painter/valuecurve.h"
#include "object/sequencefloat.h"


namespace MO {
namespace GUI {

namespace {
    class SequenceFloatCurveData : public PAINTER::ValueCurveData
    {
    public:
        const SequenceFloat * sequence;
        Double value(Double time) const { return sequence->value(time); }
    };
}

SequenceWidget::SequenceWidget(Track * track, Sequence * seq, QWidget *parent) :
    QWidget     (parent),
    track_      (track),
    sequence_   (seq),
    curvePainter_(0),
    curveData_   (0),
    action_     (A_NOTHING),
    hovered_    (false)
{
    MO_DEBUG_GUI("SequenceWidget::SequenceWidget(" << track << ", " << seq << ", " << parent << ")");

    setFocusPolicy(Qt::ClickFocus);
    //setMouseTracking(true);

    colorBody_ = QColor(80, 120, 80);
    colorBodySel_ = QColor(120, 140, 100);

    colorOutline_ = QColor(0,0,0);
    colorOutlineSel_ = QColor(160,160,160);

    // prepare a float curve painter
    if (SequenceFloat * seqf = qobject_cast<SequenceFloat*>(seq))
    {
        curvePainter_ = new PAINTER::ValueCurve(this);
        auto data = new SequenceFloatCurveData;
        data->sequence = seqf;

        curveData_ = data;
        curvePainter_->setCurveData(curveData_);
    }
}

SequenceWidget::~SequenceWidget()
{
    if (curveData_)
        delete curveData_;
}

void SequenceWidget::enterEvent(QEvent *)
{
    hovered_ = true;
    update();
}

void SequenceWidget::leaveEvent(QEvent *)
{
    hovered_ = false;
    update();
}

void SequenceWidget::paintEvent(QPaintEvent * e)
{
    QPainter p(this);

    // --- body ---

    QColor
        outline = hasFocus()? colorOutlineSel_ : colorOutline_,
            body = hasFocus()? colorBodySel_ : colorBody_;

    if (hovered_)
    {
        outline = outline.lighter(120);
        body = body.lighter(120);
    }

    QPen outpen = QPen(outline);
    outpen.setWidth(hasFocus()? 3 : 1);

    p.setPen(outpen);
    p.setBrush(QBrush(body));

    p.drawRect(rect());


    // --- value curve ---

    if (curvePainter_)
    {
        curvePainter_->setViewSpace( UTIL::ViewSpace(
            0.0, -1.0, sequence_->length(), 2.0
            ));
        curvePainter_->paint(p, e->rect());
    }
}

#if (0)
void SequenceWidget::mousePressEvent(QMouseEvent * e)
{
//    emit clicked(this, e->button());
//    e->accept();

    if (e->button() == Qt::LeftButton)
    {
        action_ = A_DRAG_POS;
        mouseClickPos_ = e->pos();

        e->accept();
        return;
    }
*/
}

void SequenceWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (action_ == A_DRAG_POS)
    {
        QPoint delta = mouseClickPos_ - e->pos();

        e->accept();
        return;
    }
}

void SequenceWidget::mouseReleaseEvent(QMouseEvent * )
{
    action_ = A_NOTHING;
}
#endif

} // namespace GUI
} // namespace MO
