/** @file moapplication.cpp

    @brief QApplication wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/


#include <iostream>

#include <QPalette>
#include <QWidget>
#include <QHostInfo>
#include <QHostAddress>

#include "io/application.h"
#include "io/error.h"
#include "network/tcpserver.h"

#define MO_APP_EXCEPTIONS_ABORT //abort();


namespace MO {

Application * application;

Application::Application(int& argc, char** args)
    : QApplication  (argc, args)
    #ifndef MO_CLIENT
      ,server_      (0)
    #endif
{
    //updateStyle();
}

#ifndef MO_CLIENT
TcpServer * Application::server()
{
    if (!server_)
        server_ = new TcpServer(this);

    return server_;
}

#endif



bool Application::notify(QObject * o, QEvent * e)
{
    try
    {
        return QApplication::notify(o, e);
    }
    catch (const MO::AudioException& e)
    {
        std::cout << "AudioException in notify [" << e.what() << "]" << std::endl;
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const MO::LogicException& e)
    {
        std::cout << "LogicException in notify [" << e.what() << "]" << std::endl;
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const MO::IoException& e)
    {
        std::cout << "IoException in notify [" << e.what() << "]" << std::endl;
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const MO::GlException& e)
    {
        std::cout << "GlException in notify [" << e.what() << "]" << std::endl;
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const MO::Exception& e)
    {
        std::cout << "Exception in notify [" << e.what() << "]" << std::endl;
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const std::exception& e)
    {
        std::cout << "std::exception in notify [" << e.what() << "]" << std::endl;
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (...)
    {
        std::cout << "unrecognized exception in notify" << std::endl;
        MO_APP_EXCEPTIONS_ABORT
    }
    return false;
}


void Application::updateStyle()
{
    setStyleSheet(
                "* { background-color: #202020; color: #a0a0a0; "
                "    border: 1px solid #404040; "
                "    selection-background-color: #505060; "
                "    selection-color: #ffffff } "
                "*:hover { background-color: #242424 } "
                "*:pressed { background-color: #121212 } "
                "QLabel { border: 0px } "
                "* { show-decoration-selected: 0 } "
                "QAbstractItemView { background-color: #808080 } "
                "QAbstractItemView:hover { background-color: #868686 } "
                );
}

void Application::setPaletteFor(QWidget * w)
{
    QPalette p(w->palette());

   // p.setColor(QPalette::Background, QColor(30,30,30));

    w->setPalette(p);
}


} // namespace MO
