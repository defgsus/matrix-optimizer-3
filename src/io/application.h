/** @file moapplication.h

    @brief QApplication wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_IO_MOAPPLICATION_H
#define MOSRC_IO_MOAPPLICATION_H

#include <QApplication>

namespace MO {

class Application : public QApplication
{
    Q_OBJECT
public:
    explicit Application(int& argc, char** args);

signals:

public slots:

    void updateStyle();

protected:

    virtual bool notify(QObject * o, QEvent * e);
};

extern Application * application;

} // namespace MO

#endif // MOSRC_IO_MOAPPLICATION_H
