/** @file application.h

    @brief QApplication wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_IO_APPLICATION_H
#define MOSRC_IO_APPLICATION_H

#include <QApplication>
#include <QMainWindow>

namespace MO {

class Application;

/** Access to singleton instance */
Application * application();

void createApplication(int& argc, char ** argv);

class Application : public QApplication
{
    Q_OBJECT
public:
    explicit Application(int& argc, char** args);

    QMainWindow * mainWindow() const { return mainWindow_; }
    void setMainWindow(QMainWindow * win) { mainWindow_ = win; }

    /** Returns the geometry of the given screen */
    QRect screenGeometry(uint screenIndex) const;

    /** Returns the users name for this session */
    const QString& userName() const { return userName_; }

    /** Returns the session id, made from the user name,
        valid for filenames, or empty string if no user name. */
    const QString& sessionId() const { return sessionId_; }
signals:

public slots:

    void setPaletteFor(QWidget*);

    /** Creates a dockwidget for the widget and installs it in the mainwindow */
    QDockWidget * createDockWidget(const QString& name, QWidget * w);

    /** Sets the user's name for this session.
        Must be called as early as possible */
    void setUserName(const QString& n);

    /** About dialog */
    void aboutMO();

protected:

    virtual bool notify(QObject * o, QEvent * e);

    QMainWindow * mainWindow_;

    QString userName_, sessionId_;
};


} // namespace MO

#endif // MOSRC_IO_APPLICATION_H
