/** @file audiosettings.h

    @brief Audio settings dialog

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#ifndef MOSRC_GUI_AUDIODIALOG_H
#define MOSRC_GUI_AUDIODIALOG_H

#include <QDialog>

class QComboBox;
class QToolButton;

namespace MO {
namespace AUDIO { class AudioDevices; class AudioDevice; }
namespace GUI {


class AudioDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AudioDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~AudioDialog();

    /** Returns the selected device index, or -1 */
    int selectedDeviceIndex() const;

signals:

public slots:

private slots:
    void toggleTesttone_();
    void fillDeviceBox_();
    void deviceSelected_();

private:

    void checkDevices_();
    void startTone();
    void stopTone();

    AUDIO::AudioDevices * devices_;
    AUDIO::AudioDevice * device_;

    QComboBox *apiBox_, *deviceBox_;
    QToolButton * testButt_;

    int freq_, vol_;
    float phase_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_AUDIODIALOG_H
