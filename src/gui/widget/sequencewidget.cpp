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
#include "object/control/sequencefloat.h"


namespace MO {
namespace GUI {

namespace {
    class SequenceFloatCurveData : public PAINTER::ValueCurveData
    {
    public:
        const SequenceFloat * sequence;
        Double value(Double time) const
            { return sequence->valueFloat(0, RenderTime(/*sequence->start() +*/ time, MO_GUI_THREAD)); }
    };
}

SequenceWidget::SequenceWidget(Track * track, Sequence * seq, QWidget *parent) :
    QWidget     (parent),
    track_      (track),
    sequence_   (seq),
    curvePainter_(0),
    curveData_   (0),
    minValue_   (-1.0),
    maxValue_   (1.0),
    hovered_    (false),
    selected_   (false),
    onLeft_     (false),
    onRight_    (false),
    edgeWidth_  (4)
{
    MO_DEBUG_GUI("SequenceWidget::SequenceWidget(" << track << ", " << seq << ", " << parent << ")");

    MO_ASSERT(track && seq, "No Sequence or Track given for SequenceWidget");

    setMouseTracking(true);

    // prepare a float curve painter
    if (SequenceFloat * seqf = qobject_cast<SequenceFloat*>(seq))
    {
        curvePainter_ = new PAINTER::ValueCurve(this);
        auto data = new SequenceFloatCurveData;
        data->sequence = seqf;

        curveData_ = data;
        curvePainter_->setCurveData(curveData_);
    }

    updateColors();
    updateValueRange();
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

void SequenceWidget::updateValueRange()
{
    if (SequenceFloat * seqf = qobject_cast<SequenceFloat*>(sequence_))
    {
        seqf->getValueFloatRange(
                    0, RenderTime(seqf->start(), MO_GUI_THREAD),
                    seqf->end() - seqf->start(), &minValue_, &maxValue_);
        maxValue_ += 0.1; minValue_ -= 0.1;
        auto delta = (maxValue_ - minValue_) / 50;
        maxValue_ += delta;
        minValue_ -= delta;
        updateViewSpace();
    }
}

void SequenceWidget::updateColors()
{
    colorBody_ = sequence_->color();
    colorBodySel_ = colorBody_.lighter(150);

    colorOutline_ = QColor(0,0,0);
    colorOutlineSel_ = colorBody_.lighter(200);

    // make text and curve darker or brighter than body?
    const bool dark = colorBody_.lightness() > 100;

    penText_ = dark ? QPen(Qt::black) : QPen(Qt::white);
    penStart_ = dark ? QPen(QColor(0,0,0,50)) : QPen(QColor(255,255,255,50));
    penLoop_ = QPen(penStart_);

    if (curvePainter_)
    {
        QPen pen = curvePainter_->pen();
        pen.setColor(dark ? colorBody_.darker(150) : colorBody_.lighter(250));
        curvePainter_->setPen(pen);
    }

    update();
}

void SequenceWidget::resizeEvent(QResizeEvent *)
{
    updateViewSpace();
}

void SequenceWidget::updateViewSpace()
{
    space_ = UTIL::ViewSpace(
                sequence_->start(), minValue_,
                sequence_->end() - sequence_->start(), maxValue_ - minValue_
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

    int x = space_.mapXFrom(sequence_->start() - sequence_->timeOffset() / sequence_->speed()) * width();
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
