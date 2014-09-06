/** @file client.cpp

    @brief Client runtime process

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include "client.h"
#include "io/log.h"
#include "io/application.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "object/scene.h"
#include "network/tcpserver.h"
#include "network/netlog.h"

namespace MO {

Client::Client(QObject *parent) :
    QObject     (parent),
    glManager_  (0)
{
}

int Client::run()
{
    MO_PRINT(tr("Matrix Optimizer Client"));

//    createObjects_();

    startNetwork_();

    int ret = application->exec();

    delete glManager_;

    return ret;
}


void Client::createGlObjects_()
{
    glManager_ = new GL::Manager(this);
    glWindow_ = glManager_->createGlWindow(MO_GFX_THREAD);

    glWindow_->show();//FullScreen();
}

void Client::startNetwork_()
{
    tcp_ = new TcpServer(this);

    tcp_->open();
}

} // namespace MO
