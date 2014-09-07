/** @file saveequationdialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/7/2014</p>
*/

#ifndef SAVEEQUATIONDIALOG_H
#define SAVEEQUATIONDIALOG_H

#include <QDialog>

class QComboBox;
class QLineEdit;

namespace MO {
namespace IO { class EquationPresets; }
namespace GUI {


class SaveEquationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SaveEquationDialog(QWidget *parent = 0);
    explicit SaveEquationDialog(const QString& presetGroupName_, QWidget *parent = 0);
    ~SaveEquationDialog();

signals:

public slots:

private slots:

    void onGroupSelect_();

private:

    void createWidgets_();
    void updateCompleter_();

    IO::EquationPresets * presets_;

    QComboBox * comboGroup_;
    QLineEdit * edit_;

    QString curGroup_;
};

} // namespace GUI
} // namespace MO

#endif // SAVEEQUATIONDIALOG_H
