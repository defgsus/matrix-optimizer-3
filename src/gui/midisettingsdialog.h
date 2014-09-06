/** @file midisettingsdialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#ifndef MIDISETTINGSDIALOG_H
#define MIDISETTINGSDIALOG_H

#include <QDialog>

class QComboBox;

namespace MO {
namespace GUI {


class MidiSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MidiSettingsDialog(QWidget *parent = 0);

signals:

public slots:

private:

    void createWidgets_();
    void updateDeviceBox_();

    QComboBox *comboApi_, *comboDevice_;
};

} // namespace GUI
} // namespace MO

#endif // MIDISETTINGSDIALOG_H
