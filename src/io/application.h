/** @file moapplication.h

    @brief QApplication wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_IO_MOAPPLICATION_H
#define MOSRC_IO_MOAPPLICATION_H

#include <QApplication>
#include <QMainWindow>

namespace MO {
class TcpServer;

class Application : public QApplication
{
    Q_OBJECT
public:
    explicit Application(int& argc, char** args);

    QMainWindow * mainWindow() const { return mainWindow_; }
    void setMainWindow(QMainWindow * win) { mainWindow_ = win; }

#ifndef MO_CLIENT
    /** Returns the one server object */
    TcpServer * server();
#endif

signals:

public slots:

    void updateStyle();
    void setPaletteFor(QWidget*);
protected:

    virtual bool notify(QObject * o, QEvent * e);

    QMainWindow * mainWindow_;

#ifndef MO_CLIENT
    TcpServer * server_;
#endif
};

extern Application * application;

} // namespace MO

#endif // MOSRC_IO_MOAPPLICATION_H
