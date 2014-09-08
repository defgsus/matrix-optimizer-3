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
class QPushButton;
class QLabel;

namespace MO {
namespace IO { class EquationPresets; class EquationPreset; }
namespace GUI {


class SaveEquationDialog : public QDialog
{
    Q_OBJECT
public:
    /** Constructs a save dialog for the given equation */
    explicit SaveEquationDialog(const QString& equation, QWidget *parent = 0);
    /** Constructs a save dialog for the given equation,
        preset group preselected to @p presetGroupName */
    explicit SaveEquationDialog(const QString& equation,
                                const QString& presetGroupName, QWidget *parent = 0);
    ~SaveEquationDialog();

    /** After accepted execution, this will contain the name of the saved preset */
    const QString& presetName() const { return curGroup_; }
    /** After accepted execution, this will contain the name of the saved equation */
    const QString& equationName() const { return equationName_; }

signals:

public slots:

private slots:

    void onGroupSelect_();
    void onOk_();
    void onEditChanged_();

private:

    void createWidgets_();
    void updateCompleter_();
    void updateGroupCompleter_();
    /** Returns preset or NULL, depending on curGroup_ */
    IO::EquationPreset * currentPreset_() const;

    IO::EquationPresets * presets_;

    QComboBox * comboGroup_;
    QLineEdit * edit_, *editGroup_;
    QPushButton * butOk_;
    QLabel * labelGroup_;

    QString curGroup_, equation_, equationName_;
};

} // namespace GUI
} // namespace MO

#endif // SAVEEQUATIONDIALOG_H
