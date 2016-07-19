/** @file timelineeditdialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.10.2014</p>
*/

#ifndef MOSRC_GUI_TIMELINEEDITDIALOG_H
#define MOSRC_GUI_TIMELINEEDITDIALOG_H

#include <QDialog>

class QTimer;

namespace MO {
namespace MATH { class Timeline1d; }
namespace GUI {
namespace UTIL { class ViewSpace; }

class Timeline1DView;
class Timeline1DRulerView;

class TimelineEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TimelineEditDialog(QWidget *parent = 0);
    ~TimelineEditDialog();

    const MATH::Timeline1d & timeline() const { return *tl_; }

    int options() const { return options_; }

    const Timeline1DView & editor() const;
    Timeline1DView & editor();

signals:

    /** Emitted when a changed has been made to the timeline */
    void timelineChanged();

public slots:

    /** Sets the timeline data to edit */
    void setTimeline(const MATH::Timeline1d& tl);

    /** Sets the Timeline1DView::Option options */
    void setOptions(int options);

    void setViewSpace(const UTIL::ViewSpace&);

private:

    void createWidgets_();

    MATH::Timeline1d * tl_;
    Timeline1DRulerView * editor_;
    QTimer * timer_;
    bool autoUpdate_;
    int options_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_TIMELINEEDITDIALOG_H
