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
#include "io/error.h"
#include "gui/painter/valuecurve.h"
#include "object/sequencefloat.h"


namespace MO {
namespace GUI {

namespace {
    class SequenceFloatCurveData : public PAINTER::ValueCurveData
    {
    public:
        const SequenceFloat * sequence;
        Double value(Double time) const
            { return sequence->value(sequence->start() + time); }
    };
}

SequenceWidget::SequenceWidget(Track * track, Sequence * seq, QWidget *parent) :
    QWidget     (parent),
    track_      (track),
    sequence_   (seq),
    curvePainter_(0),
    curveData_   (0),
    hovered_    (false),
    selected_   (false),
    onLeft_     (false),
    onRight_    (false),
    edgeWidth_  (4)
{
    MO_DEBUG_GUI("SequenceWidget::SequenceWidget(" << track << ", " << seq << ", " << parent << ")");

    MO_ASSERT(track && seq, "No Sequence or Track given for SequenceWidget");

    setMouseTracking(true);

    colorBody_ = QColor(80, 120, 80);
    colorBodySel_ = colorBody_.lighter(150);

    colorOutline_ = QColor(0,0,0);
    colorOutlineSel_ = colorBody_.lighter(200);

    penText_ = QPen(QColor(255,255,255));
    penStart_ = QPen(QColor(255,255,255,50));
    penLoop_ = QPen(penStart_);

    // prepare a float curve painter
    if (SequenceFloat * seqf = qobject_cast<SequenceFloat*>(seq))
    {
        curvePainter_ = new PAINTER::ValueCurve(this);
        auto data = new SequenceFloatCurveData;
        data->sequence = seqf;

        curveData_ = data;
        curvePainter_->setCurveData(curveData_);
    }

    updateName();
}

SequenceWidget::~SequenceWidget()
{
    if (curveData_)
        delete curveData_;
}

void SequenceWidget::setSelected(bool enable)
{
    if (selected_ != enable)
        update();

    selected_ = enable;
}

void SequenceWidget::updateName()
{
    nameText_.setText(sequence_->name());
}

void SequenceWidget::resizeEvent(QResizeEvent *)
{
    space_ = UTIL::ViewSpace(
                0.0, -1.0, sequence_->length(), 2.0
                );
}

void SequenceWidget::enterEvent(QEvent *)
{
    hovered_ = true;
    emit hovered(this, true);
    update();
}

void SequenceWidget::leaveEvent(QEvent *)
{
    hovered_ = onLeft_ = onRight_ = false;
    emit hovered(this, false);
    update();
}

void SequenceWidget::paintEvent(QPaintEvent * e)
{
    QPainter p(this);

    // --- body ---

    QColor
        outline = selected_? colorOutlineSel_ : colorOutline_,
            body = selected_? colorBodySel_ : colorBody_;

    if (hovered_)
    {
        outline = outline.lighter(120);
        body = body.lighter(120);
    }

    QPen outpen = QPen(outline);
    outpen.setWidth(selected_? 3 : 1);

    p.setPen(outpen);
    p.setBrush(QBrush(body));

    p.drawRect(rect());


    // --- value curve ---

    if (curvePainter_)
    {
        curvePainter_->setViewSpace( space_ );
        curvePainter_->paint(p, e->rect());
    }

    // --- time offset ---

    int x = space_.mapXFrom(-sequence_->timeOffset()) * width();
    if (x>e->rect().left() && x<=e->rect().right())
    {
        p.setPen(penStart_);
        p.drawLine(x, e->rect().top(), x, e->rect().bottom());
    }

    // --- name ---

    p.setPen(penText_);
    p.drawStaticText(2, 2, nameText_);
}



void SequenceWidget::mouseMoveEvent(QMouseEvent * e)
{
    onLeft_ = (e->x() <= edgeWidth_);
    onRight_ = (e->x() >= width() - edgeWidth_ - 1);

    setCursor((onLeft_ || onRight_)
              ? Qt::SplitHCursor
              : Qt::ArrowCursor
                );
    e->ignore();
}

} // namespace GUI
} // namespace MO
