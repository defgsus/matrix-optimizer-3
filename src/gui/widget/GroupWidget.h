/** @file groupwidget.h

    @brief A Widget to group other widgets with drop-down function

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_GROUPWIDGET_H
#define MOSRC_GUI_WIDGET_GROUPWIDGET_H

#include <QWidget>
#include <QMap>

class QToolButton;
class QLabel;

namespace MO {
namespace GUI {


class GroupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GroupWidget(QWidget *parent = 0);
    explicit GroupWidget(const QString& title, QWidget *parent = 0);
    explicit GroupWidget(const QString& title, bool expanded, QWidget *parent = 0);

    void setTitle(const QString&);
    const QString& title() const { return title_; }

    /** Adds a widget to the drop-down group */
    void addWidget(QWidget *);
    /** Adds a layout to the group's layout */
    void addLayout(QLayout *);

    /** Returns if the group is expanded */
    bool isExpanded() const { return expanded_; }

    /** Returns whether the given widget is set visible in the group.
        This is independend of the expanded-state.
        Returns true also if widget is unknown */
    bool isVisible(QWidget *) const;

    /** Adds a widget to the header */
    void addHeaderWidget(QWidget * widget);

    void addHeaderSpacing(int s);

    void setHeaderStatusTip(const QString& tip);
signals:

    void expanded();
    void collapsed();

public slots:

    void setExpanded(bool expanded, bool send_signal = false);
    void expand(bool send_signal = false);
    void collapse(bool send_signal = false);

    /** Sets a widget inside the group box visible or hidden.
        Call this instead of w->setVisible(), because the group
        will control the visibility of it's widgets. */
    void setVisible(QWidget * w, bool visible);

    /** Pull from QWidget namespace */
    void setVisible(bool v) { QWidget::setVisible(v); }
private:

    void createLayout_();
    void updateArrow_();

    bool expanded_;
    QString title_;
    QList<QWidget*> containedWidgets_;
    QMap<QWidget*, bool> visibility_;

    QToolButton * button_;
    QLabel * label_;
    QWidget * header_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_GROUPWIDGET_H
