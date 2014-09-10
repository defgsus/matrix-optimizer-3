/** @file netlog.h

    @brief logging for network objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QObject>
#include <QList>
#include <QTextStream>

#ifndef MOSRC_NETWORK_NETLOG_H
#define MOSRC_NETWORK_NETLOG_H

class QTcpSocket;

namespace MO {

class NetworkLogger : public QObject
{
    Q_OBJECT

    NetworkLogger(QObject*);
    ~NetworkLogger();
public:

    enum Level
    {
        ERROR,
        WARNING,
        DEBUG,
        EVENT,
        CTOR
    };

    struct LogLine
    {
        Level level;
        QString string;
    };

    static NetworkLogger & instance();

    static void connectForLogging(QTcpSocket*);

    // -------- logging ----------

    static void beginWrite(Level);
    static void endWrite();

    template <typename T>
    NetworkLogger& operator << (const T& stuff)
    {
        *stream_ << stuff;
        return *this;
    }

private:

    static NetworkLogger * instance_;
    QList<LogLine> text_;
    QString curText_;
    Level curLevel_;
    QTextStream * stream_;
};


#define MO_NETLOG(level__, stream_arg__)                \
{   ::MO::NetworkLogger::beginWrite(                    \
        ::MO::NetworkLogger::level__);                  \
    ::MO::NetworkLogger::instance()                     \
        << stream_arg__;                                \
    ::MO::NetworkLogger::endWrite();                    \
}



} // namespace MO

#endif // MOSRC_NETWORK_NETLOG_H
