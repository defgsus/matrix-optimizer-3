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

namespace MO {
class Track;
class Sequence;
namespace GUI {
namespace UTIL { class ViewSpace; }
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

    QList<SequenceWidget*>& influencedWidgets() { return influencedWidgets_; }

signals:

    void hovered(SequenceWidget*, bool);

public slots:

    void setSelected(bool enable);

protected:

    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    /*
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    */
private:

    enum Action_
    {
        A_NOTHING,
        A_DRAG_POS
    };

    Track * track_;
    Sequence * sequence_;
    QList<SequenceWidget*> influencedWidgets_;

    PAINTER::ValueCurve * curvePainter_;
    PAINTER::ValueCurveData * curveData_;

    Action_ action_;
    bool hovered_, selected_;
    QPoint mouseClickPos_;

    // ---- config ----

    QColor
        colorOutline_,
        colorOutlineSel_,
        colorBody_,
        colorBodySel_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_SEQUENCEWIDGET_H
