/** @file systeminfo.cpp

    @brief System information receiver

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#include <QDesktopWidget>
#include <QTextStream>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QScreen>

#include "systeminfo.h"
#include "io/datastream.h"
#include "io/application.h"

namespace MO {


SystemInfo::SystemInfo()
{
}

void SystemInfo::serialize(IO::DataStream &io) const
{
    io.writeHeader("sysinfo", 1);

    io << appPath_ << screenSizes_;
}

void SystemInfo::deserialize(IO::DataStream &io)
{
    io.readHeader("sysinfo", 1);

    io >> appPath_ >> screenSizes_;
}

void SystemInfo::get()
{
    appPath_ = application->applicationFilePath();

    // local ip
    localIp_ = "unknown";
    const auto ips = QNetworkInterface::allAddresses();
    for (const QHostAddress ip : ips)
    {
        if (!ip.isLoopback() && ip.protocol() == QAbstractSocket::IPv4Protocol
            && ip != QHostAddress::LocalHost)
        {
            localIp_ = ip.toString();
            break;
        }
    }

    // screen geometries
#ifdef THATS_THE_QDESKTOPWIDGET_METHOD_
    auto desk = application->desktop();
    for (int i=0; i<desk->screenCount(); ++i)
    {
        screenSizes_.append(desk->screenGeometry(i).size());
    }
#else
    auto screens = qApp->screens();
    for (int i=0; i<screens.count(); ++i)
    {
        screenSizes_.append(screens[i]->geometry().size());
    }
#endif
}

QString SystemInfo::toString() const
{
    QString text;
    QTextStream s(&text);

    s << "path: " << appPath_
      << "\nip: " << localIp_
      << "\nscreens:";
    for (int i=0; i<screenSizes_.size(); ++i)
        s << " " << i << ": " << screenSizes_[i].width()
          << "x" << screenSizes_[i].height();


    return text;
}


} // namespace MO
