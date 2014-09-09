/** @file systeminfo.h

    @brief System information receiver

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#ifndef MOSRC_IO_SYSTEMINFO_H
#define MOSRC_IO_SYSTEMINFO_H

#include <QString>

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

    /** Fills all datafields from current system */
    void get();

    const QString& applicationPath() const { return appPath_; }

    int width() const { return w_; }
    int height() const { return h_; }

private:

    QString appPath_;

    int w_, h_;
};

} // namespace MO

#endif // MOSRC_IO_SYSTEMINFO_H
