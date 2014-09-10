/** @file netlogwidget.h

    @brief Log display for NetworkLogger

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_NETLOGWIDGET_H
#define MOSRC_GUI_WIDGET_NETLOGWIDGET_H

#include <QPlainTextEdit>

namespace MO {
namespace GUI {


class NetLogWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit NetLogWidget(QWidget *parent = 0);

signals:

public slots:

private slots:

    void addLine_(int level, const QString&);

};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_NETLOGWIDGET_H
