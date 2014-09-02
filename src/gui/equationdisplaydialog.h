/** @file equationdisplaydialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#ifndef EQUATIONDISPLAYDIALOG_H
#define EQUATIONDISPLAYDIALOG_H

#include <QDialog>

class QComboBox;

namespace MO {
namespace GUI {

class EquationDisplayWidget;
class EquationEditor;
class DoubleSpinBox;

class EquationDisplayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EquationDisplayDialog(QWidget *parent = 0);

signals:

public slots:

protected:

    void closeEvent(QCloseEvent *);

private slots:

    void loadSettings_();
    void saveSettings_();

    void updateViewspace_();
    void updateFromViewspace_();

private:

    void createWidgets_();
    void updateModeBox_();

    EquationDisplayWidget * display_;
    EquationEditor * editor_;

    DoubleSpinBox
        *spinX0_, *spinY0_, *spinX1_, *spinY1_;

    QComboBox * comboMode_;
};



} // namespace GUI
} // namespace MO


#endif // EQUATIONDISPLAYDIALOG_H
