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
class Sequence;
namespace GUI {
namespace UTIL { class ViewSpace; }

class SequenceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SequenceWidget(Sequence * = 0, QWidget *parent = 0);

    Sequence * sequence() const { return sequence_; }

signals:

public slots:

    void setSequence(Sequence *);
    //void adjustGeometry(const ViewSpace&);

private:

    Sequence * sequence_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_SEQUENCEWIDGET_H
