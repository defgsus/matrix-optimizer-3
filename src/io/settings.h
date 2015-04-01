/** @file settings.h

    @brief Application settings

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#ifndef MOSRC_IO_SETTINGS_H
#define MOSRC_IO_SETTINGS_H

#include <QSettings>

class QWindow;
class QMainWindow;

namespace MO {

class DomeSettings;
class ProjectorSettings;
class CameraSettings;
class ProjectionSystemSettings;
class DiskRenderSettings;
class Settings;

/** singleton instance */
Settings * settings();

class Settings : public QSettings
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);

    /** Returns some settings as printable string */
    QString infoString() const;

    /** Returns the value for key.
        If key is unknown, warning is printed
        and empty QVariant() returned. */
    QVariant getValue(const QString& key) const;

#if (0)
    /** Stores the geometry provided the QWidget::objectName() is set */
    void saveGeometry(QMainWindow *);
    /** Restores the geometry provided the QWidget::objectName() is set */
    void restoreGeometry(QMainWindow *);
#endif

    /** Stores the geometry provided the QWidget::objectName() is set */
    void storeGeometry(QWindow *);
    /** Restores the geometry provided the QWidget::objectName() is set */
    bool restoreGeometry(QWindow *);

    /** Stores the geometry provided the QWidget::objectName() is set */
    void storeGeometry(QWidget *);
    /** Restores the geometry provided the QWidget::objectName() is set */
    bool restoreGeometry(QWidget *);

    /** Returns address of the server */
    QString serverAddress() const;
    void setServerAddress(const QString&);

    QString udpAudioMulticastAddress() const;
    void setUdpAudioMulticastAddress(const QString&);
    uint16_t udpAudioMulticastPort() const;
    void setUdpAudioMulticastPort(uint16_t);

    /** Returns the index of the client */
    int clientIndex() const;
    void setClientIndex(int);

    /** Selected desktop index */
    uint desktop() const;
    void setDesktop(uint index);

    /** Application style sheet */
    QString styleSheet() const;
    void setStyleSheet(const QString&);

    /** Sets the ProjectionSystemSettings as default for the application */
    void setDefaultProjectionSettings(const ProjectionSystemSettings&);

    /** Returns a previously saved setting. If there is no saved setting,
        a default setting with 1 projector will be returned. */
    ProjectionSystemSettings getDefaultProjectionSettings();

    DomeSettings domeSettings();
    ProjectorSettings projectorSettings();
    CameraSettings cameraSettings();

    DiskRenderSettings getDiskRenderSettings();
    void setDiskRenderSettings(const DiskRenderSettings&);

signals:

public slots:

private:

    void createDefaultValues_();

    QMap<QString, QVariant> defaultValues_;
};


} // namespace MO

#endif // MOSRC_IO_SETTINGS_H
