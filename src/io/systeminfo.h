/** @file systeminfo.h

    @brief System information receiver

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#ifndef MOSRC_IO_SYSTEMINFO_H
#define MOSRC_IO_SYSTEMINFO_H

#include <QString>
#include <QSize>
#include <QList>

namespace MO {
namespace IO { class DataStream; }

class SystemInfo
{
public:
    SystemInfo();

    // ------------- io ---------------

    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    // ----------- getter -------------

    /** Returns true when system information has been gathered */
    bool isValid() const { return !appPath_.isEmpty(); }

    /** Fills all datafields from current system */
    void get();

    /** Returns all info as multiline string */
    QString toString() const;

    const QString& applicationPath() const { return appPath_; }

    int numScreens() const { return screenSizes_.size(); }

    const QSize& resolution(int screen) const { return screenSizes_.at(screen); }

private:

    QString appPath_, localIp_;

    QList<QSize> screenSizes_;
};

} // namespace MO

#endif // MOSRC_IO_SYSTEMINFO_H
