/** @file netlog.cpp

    @brief logging for network objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include "netlog.h"
#include "io/application.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

NetworkLogger * NetworkLogger::instance_ = 0;

NetworkLogger::NetworkLogger(QObject * p)
    : QObject   (p),
      stream_   (new QTextStream(&curText_))
{
    MO_DEBUG_IO("NetworkLogger::NetworkLogger(" << p << ")");
}

NetworkLogger::~NetworkLogger()
{
    delete stream_;
}

NetworkLogger& NetworkLogger::instance()
{
    if (!instance_)
    {
        MO_ASSERT(application, "NetworkLogger needs Application object");
        instance_ = new NetworkLogger(application);
    }
    return *instance_;
}

void NetworkLogger::beginWrite(Level l)
{
    NetworkLogger & n = instance();
    n.stream_->seek(0);
    n.curText_.clear();
    n.curLevel_ = l;
}

void NetworkLogger::endWrite()
{
    NetworkLogger & n(instance());

    LogLine line;
    line.level = n.curLevel_;
    line.string = n.curText_;

    n.text_.append(line);

    MO_DEBUG("NETLOG: " << line.string.left(line.string.count()-1));
}

} // namespace MO
