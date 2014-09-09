/** @file client.cpp

    @brief Client runtime process

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QTcpSocket>

#include "clientengine.h"
#include "io/log.h"
#include "io/application.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "object/scene.h"
#include "network/tcpserver.h"
#include "network/netlog.h"
#include "network/networkmanager.h"
#include "network/netevent.h"
#include "network/client.h"

namespace MO {

ClientEngine::ClientEngine(QObject *parent) :
    QObject     (parent),
    glManager_  (0),
    client_     (0)
{
}

int ClientEngine::run(int , char ** )
{
    MO_PRINT(tr("Matrix Optimizer Client"));

//    createObjects_();

    //send_ = (argc>1 && QString("send") == argv[1]);

    startNetwork_();

    int ret = application->exec();

    delete glManager_;

    return ret;
}


void ClientEngine::createGlObjects_()
{
    glManager_ = new GL::Manager(this);
    glWindow_ = glManager_->createGlWindow(MO_GFX_THREAD);

    glWindow_->show();//FullScreen();
}

void ClientEngine::startNetwork_()
{
    client_ = new Client(this);

    connect(client_, &Client::eventReceived, [=](AbstractNetEvent*e)
    {
        MO_PRINT("got event '" << e->className() << "'");
    });

    client_->connectTo("192.168.1.33");

    /*
    while (!client_->connectToMaster())
    {
        MO_PRINT("retrying");
    }*/

/*
    NetworkLogger::connectForLogging(socket_);

    socket_->connectToHost(
                QHostAddress("0.0.0.0"),
                NetworkManager::defaultTcpPort());

    connect(socket_, &QTcpSocket::connected, [=]()
    {
        NetInfoEvent info;
        info.setId("hello");
        info.send(socket_);
    });
    */
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
