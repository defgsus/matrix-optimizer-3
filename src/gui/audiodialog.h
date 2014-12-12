/** @file audiosettings.h

    @brief Audio settings dialog

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#ifndef MOSRC_GUI_AUDIODIALOG_H
#define MOSRC_GUI_AUDIODIALOG_H

#include <vector>

#include <QDialog>

#include "audio/audio_fwd.h"

class QComboBox;
class QToolButton;
class QSpinBox;
class QTimer;

namespace MO {
namespace AUDIO { class AudioDevices; class AudioDevice; }
namespace GUI {

class EnvelopeWidget;

class AudioDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AudioDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~AudioDialog();

    /** Returns the selected device index, or -1 */
    int selectedInDeviceIndex() const;
    int selectedOutDeviceIndex() const;

signals:

public slots:

private slots:
    void toggleTesttone_();
    void apiSelected_();
    void deviceSelected_();
//    void inDeviceSelected_();
//    void outDeviceSelected_();
    void setDefaultSamplerate_();
    void setDefaultBuffersize_();
    void setDefaultChannelsIn_();
    void setDefaultChannelsOut_();

private:

    void fillInDeviceBox_();
    void fillOutDeviceBox_();
    void checkDevices_();
    void setWidgetChannelLimits_();
    void startTone_();
    void stopTone_();

    void storeConfig_();

    AUDIO::AudioDevices * devices_;
#ifdef __APPLE__
    AUDIO::AudioDevices * inDevices_;
#endif
    AUDIO::AudioDevice * device_;
#ifdef __APPLE__
    AUDIO::AudioDevice * inDevice_;
#endif


    std::vector<AUDIO::EnvelopeFollower*> envFollower_;

    QComboBox *apiBox_, *inDeviceBox_, *outDeviceBox_;
    QSpinBox *sampleRate_, *bufferSize_, *numInputs_, *numOutputs_;
    QToolButton * testButt_;
    EnvelopeWidget * envWidget_;
    QPushButton * okButt_;
    QTimer * timer_;

    int freq_, vol_;
    float realfreq_, phase_, env_;
    bool doEnv_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_AUDIODIALOG_H
