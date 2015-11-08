/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#ifndef MOSRC_GUI_CSGDIALOG_H
#define MOSRC_GUI_CSGDIALOG_H

#include <QDialog>


namespace MO {
namespace GUI {


class CsgDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CsgDialog(QWidget *parent = 0);
    ~CsgDialog();

signals:

public slots:

protected:

    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

private:
    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_CSGDIALOG_H
