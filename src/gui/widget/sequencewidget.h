/** @file sequencewidget.h

    @brief Widget for display MO::Sequence in MO::GUI::TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_SEQUENCEWIDGET_H
#define MOSRC_GUI_WIDGET_SEQUENCEWIDGET_H

#include <QWidget>

namespace MO {
class Track;
class Sequence;
namespace GUI {
namespace UTIL { class ViewSpace; }

class SequenceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SequenceWidget(Track *, Sequence *, QWidget *parent = 0);

    Sequence * sequence() const { return sequence_; }
    Track * track() const { return track_; }

signals:

public slots:

protected:

    void paintEvent(QPaintEvent *);

private:

    Track * track_;
    Sequence * sequence_;

};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_SEQUENCEWIDGET_H
