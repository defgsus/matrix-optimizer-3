/** @file midisettingsdialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#ifndef MIDISETTINGSDIALOG_H
#define MIDISETTINGSDIALOG_H

#include <QDialog>

#include "audio/audio_fwd.h"

class QComboBox;
class QPlainTextEdit;
class QToolButton;
class QTimer;

namespace MO {
namespace GUI {


class MidiSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MidiSettingsDialog(QWidget *parent = 0);
    ~MidiSettingsDialog();

signals:

public slots:

private slots:

    void onTimer_();
    void onTest_(bool go);
    void onTestSynth_(bool go);
    void onApiChoosen_();
    void onDeviceChoosen_();
    void onOk_();

    void saveSettings_();
    void loadSettings_();

private:

    void createWidgets_();
    void checkDevices_();
    void updateDeviceBox_();
    void updateWidgets_();
    void startTest_(bool start, bool audio);
    void startAudio_(bool start);

    QString curApiName_, curDeviceName_;
    int curId_;

    AUDIO::MidiDevice * curDevice_;
    AUDIO::MidiDevices * devices_;

    QComboBox *comboApi_, *comboDevice_;
    QPlainTextEdit *textBuffer_;
    QToolButton * butTest_, *butTestSynth_;

    QTimer * timer_;

    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MIDISETTINGSDIALOG_H
