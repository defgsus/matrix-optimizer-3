/** @file client.h

    @brief Client runtime process

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#ifndef MOSRC_ENGINE_CLIENT_H
#define MOSRC_ENGINE_CLIENT_H

#include <QObject>

#include "gl/opengl_fwd.h"

namespace MO {

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);

    int run();

signals:

public slots:

private:

    void createObjects_();

    GL::Manager * glManager_;
    GL::Window * glWindow_;
};

} // namespace MO

#endif // MOSRC_ENGINE_CLIENT_H
