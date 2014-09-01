/** @file equationdisplaydialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#ifndef EQUATIONDISPLAYDIALOG_H
#define EQUATIONDISPLAYDIALOG_H

#include <QDialog>

namespace MO {
namespace GUI {

class EquationDisplayWidget;

class EquationDisplayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EquationDisplayDialog(QWidget *parent = 0);

signals:

public slots:

private:

    void createWidgets_();

    EquationDisplayWidget * display_;

};



} // namespace GUI
} // namespace MO


#endif // EQUATIONDISPLAYDIALOG_H
