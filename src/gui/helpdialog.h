/** @file helpdialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#ifndef MOSRC_GUI_HELPDIALOG_H
#define MOSRC_GUI_HELPDIALOG_H

#include <QDialog>
#include <QList>

class QAction;

namespace MO {
namespace GUI {

class HelpTextBrowser;

class HelpDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HelpDialog(QWidget *parent = 0);
    explicit HelpDialog(const QString& url, QWidget *parent = 0);

signals:

public slots:

private slots:

private:

    void createWidgets_();

    HelpTextBrowser * browser_;

};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_HELPDIALOG_H
