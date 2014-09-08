/** @file client.cpp

    @brief Client runtime process

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QTcpSocket>

#include "client.h"
#include "io/log.h"
#include "io/application.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "object/scene.h"
#include "network/tcpserver.h"
#include "network/netlog.h"
#include "network/networkmanager.h"
#include "network/netevent.h"

namespace MO {

Client::Client(QObject *parent) :
    QObject     (parent),
    glManager_  (0)
{
}

int Client::run(int , char ** )
{
    MO_PRINT(tr("Matrix Optimizer Client"));

//    createObjects_();

    //send_ = (argc>1 && QString("send") == argv[1]);

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
    socket_ = new QTcpSocket(this);

    NetworkLogger::connectForLogging(socket_);

    socket_->connectToHost(
                QHostAddress("0.0.0.0"),
                NetworkManager::defaultTcpPort());

    connect(socket_, &QTcpSocket::connected, [=]()
    {
        NetInfoEvent info("hello");
        info.send(socket_);
    });

    /*
    if (send_)
    {
        MO_PRINT("SEND-MODE");

        socket_ = new QTcpSocket(this);

        NetworkLogger::connectForLogging(socket_);

        socket_->connectToHost(
                    QHostAddress("0.0.0.0"),
                    NetworkManager::defaultTcpPort());

        connect(socket_, &QTcpSocket::connected, [=]()
        {
            QTextStream stream(socket_);
            stream << "hello world";
        });
    }
    else
    {
        MO_PRINT("SERVER-MODE");

        tcp_ = new TcpServer(this);
        tcp_->open();

        connect(tcp_, &TcpServer::socketData, [=](QTcpSocket * s)
        {
            QTextStream stream(s);
            MO_PRINT("received: [" << stream.readAll() << "]");
        });
    }
    */
}

} // namespace MO
