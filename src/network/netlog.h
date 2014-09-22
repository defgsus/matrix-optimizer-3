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
        ERROR       = 1,
        WARNING     = 1<<1,
        DEBUG       = 1<<2,
        EVENT       = 1<<3,
        EVENT_V2    = 1<<4,
        CTOR        = 1<<5
    };

    struct LogLine
    {
        Level level;
        QString string;
    };

    static NetworkLogger & instance();

    // -------- logging ----------

    static void setLevels(int levels);

    static void connectForLogging(QTcpSocket*);

    static void beginWrite(Level);
    static void endWrite();

    template <typename T>
    NetworkLogger& operator << (const T& stuff)
    {
        *stream_ << stuff;
        return *this;
    }

signals:

    /** Emitted on endWrite() */
    void textAdded(int level, const QString& text);

private:

    static NetworkLogger * instance_;
    QList<LogLine> text_;
    QString curText_;
    Level curLevel_;
    QTextStream * stream_;
    int acceptedLevels_;
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
