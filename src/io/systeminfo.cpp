/** @file systeminfo.cpp

    @brief System information receiver

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#include <QDesktopWidget>

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

    io << appPath_ << w_ << h_;
}

void SystemInfo::deserialize(IO::DataStream &io)
{
    io.readHeader("sysinfo", 1);

    io >> appPath_ >> w_ >> h_;
}

void SystemInfo::get()
{
    appPath_ = application->applicationFilePath();

    auto desk = application->desktop();

    w_ = desk->width();
    h_ = desk->height();
}



} // namespace MO
