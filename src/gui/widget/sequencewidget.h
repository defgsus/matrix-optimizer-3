/** @file sequencewidget.h

    @brief Widget for display MO::Sequence in MO::GUI::TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_SEQUENCEWIDGET_H
#define MOSRC_GUI_WIDGET_SEQUENCEWIDGET_H

#include <QWidget>
#include <QPen>
#include <QBrush>
#include <QPoint>
#include <QStaticText>

#include "gui/util/viewspace.h"

namespace MO {
class Track;
class Sequence;
namespace GUI {
namespace PAINTER { class ValueCurve; class ValueCurveData; }

class SequenceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SequenceWidget(Track *, Sequence *, QWidget *parent = 0);
    ~SequenceWidget();

    Sequence * sequence() const { return sequence_; }
    Track * track() const { return track_; }

    bool selected() const { return selected_; }
    bool onLeftEdge() const { return onLeft_; }
    bool onRightEdge() const { return onRight_; }

    QList<SequenceWidget*>& influencedWidgets() { return influencedWidgets_; }

    void updateViewSpace();

signals:

    void hovered(SequenceWidget*, bool);

public slots:

    void setSelected(bool enable);

    void updateValueRange();
    void updateName();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void mouseMoveEvent(QMouseEvent *);

private:

    Track * track_;
    Sequence * sequence_;
    QList<SequenceWidget*> influencedWidgets_;

    PAINTER::ValueCurve * curvePainter_;
    PAINTER::ValueCurveData * curveData_;
    UTIL::ViewSpace space_;
    Double minValue_, maxValue_;

    QStaticText nameText_;

    // ---- interaction ----

    bool hovered_, selected_, onLeft_, onRight_;
    QPoint mouseClickPos_;

    // ---- config ----

    int edgeWidth_;

    QColor
        colorOutline_,
        colorOutlineSel_,
        colorBody_,
        colorBodySel_;

    QPen penText_,
         penStart_,
         penLoop_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_SEQUENCEWIDGET_H
