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


class Settings : public QSettings
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);

    QVariant getValue(const QString& key);

#if (0)
    /** Stores the geometry provided the QWidget::objectName() is set */
    void saveGeometry(QMainWindow *);
    /** Restores the geometry provided the QWidget::objectName() is set */
    void restoreGeometry(QMainWindow *);
#endif
    /** Stores the geometry provided the QWidget::objectName() is set */
    void saveGeometry(QWindow *);
    /** Restores the geometry provided the QWidget::objectName() is set */
    bool restoreGeometry(QWindow *);

    /** Stores the geometry provided the QWidget::objectName() is set */
    void saveGeometry(QWidget *);
    /** Restores the geometry provided the QWidget::objectName() is set */
    bool restoreGeometry(QWidget *);


    /** Returns the index of the client */
    int clientIndex();
    void setClientIndex(int);

    /** Sets the ProjectionSystemSettings as default for the application */
    void setDefaultProjectionSettings(const ProjectionSystemSettings&);

    /** Returns a previously saved setting. If there is no saved setting,
        a default setting with 1 projector will be returned. */
    ProjectionSystemSettings getDefaultProjectionSettings();

    DomeSettings domeSettings();
    ProjectorSettings projectorSettings();
    CameraSettings cameraSettings();

signals:

public slots:

private:

    void createDefaultValues_();

    QMap<QString, QVariant> defaultValues_;
};

/** singleton instance */
extern Settings * settings;


} // namespace MO

#endif // MOSRC_IO_SETTINGS_H
