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
#include <QScreen>

#include "io/application.h"
#include "io/error.h"
#include "io/isclient.h"

// usefull to catch the backtrace of exceptions in debugger
#define MO_APP_EXCEPTIONS_ABORT //abort();

#define MO_APP_PRINT(text__)            \
{                                       \
    std::cout << text__ << std::endl;   \
    if (isClient)                       \
        MO_NETLOG(ERROR, text__);       \
}

namespace MO {

Application * application;

Application::Application(int& argc, char** args)
    : QApplication  (argc, args)
{
    //updateStyle();
}



bool Application::notify(QObject * o, QEvent * e)
{
    try
    {
        return QApplication::notify(o, e);
    }
    catch (const MO::AudioException& e)
    {
        MO_APP_PRINT("AudioException in notify [" << e.what() << "]");
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const MO::LogicException& e)
    {
        MO_APP_PRINT("LogicException in notify [" << e.what() << "]");
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const MO::IoException& e)
    {
        MO_APP_PRINT("IoException in notify [" << e.what() << "]");
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const MO::GlException& e)
    {
        MO_APP_PRINT("GlException in notify [" << e.what() << "]");
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const MO::Exception& e)
    {
        MO_APP_PRINT("Exception in notify [" << e.what() << "]");
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (const std::exception& e)
    {
        MO_APP_PRINT("std::exception in notify [" << e.what() << "]");
        MO_APP_EXCEPTIONS_ABORT
    }
    catch (...)
    {
        MO_APP_PRINT("unknown exception in notify");
        MO_APP_EXCEPTIONS_ABORT
    }
    return false;
}

QRect Application::screenGeometry(uint screenIndex) const
{
    // check desktop index
    auto scr = screens();
    if (screenIndex >= (uint)scr.size())
    {
        MO_WARNING("screen index " << screenIndex
                   << " is out of range ("
                   << scr.size() << ")");
        // XXX Does not check for ZERO screens..
        screenIndex = scr.size() - 1;
    }

    // XXX workaround because setScreen() is not very reliable right now
    // ( https://bugreports.qt-project.org/browse/QTBUG-33138 )
    return scr[screenIndex]->geometry();
}




void Application::updateStyle()
{
    // XXX some test for styles

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
