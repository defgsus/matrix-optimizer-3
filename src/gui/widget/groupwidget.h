/** @file groupwidget.h

    @brief A Widget to group other widgets with drop-down function

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_GROUPWIDGET_H
#define MOSRC_GUI_WIDGET_GROUPWIDGET_H

#include <QWidget>

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

    void setTitle(const QString&);
    const QString& title() const { return title_; }

    /** Adds a widget to the drop-down group */
    void addWidget(QWidget *);

signals:

    void expanded();
    void collapsed();

public slots:

    void expand(bool send_signal = false);
    void collapse(bool send_signal = false);

private:

    void createLayout_();

    bool expanded_;
    QString title_;
    QList<QWidget*> containedWidgets_;

    QToolButton * button_;
    QLabel * label_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_GROUPWIDGET_H
