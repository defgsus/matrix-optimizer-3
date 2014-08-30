/** @file helpdialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#ifndef MOSRC_GUI_HELPDIALOG_H
#define MOSRC_GUI_HELPDIALOG_H

#include <QDialog>

class QTextBrowser;
class QAction;

namespace MO {
namespace GUI {

class HelpDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HelpDialog(QWidget *parent = 0);

signals:

public slots:

    void backward();
    void forward();

private slots:

    void onAnchor_(const QUrl&);

private:

    QString getHelpDocument(const QString & name);

    void createWidgets_();

    QTextBrowser * browser_;

    QAction
        * aBack_,
        * aForward_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_HELPDIALOG_H
