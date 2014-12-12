/** @file netlog.h

    @brief logging for network objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <sstream>

#include <QObject>
#include <QList>

#include "io/streamoperators_qt.h"

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
        DEBUG_V2    = 1<<3,
        EVENT       = 1<<4,
        EVENT_V2    = 1<<5,
        CTOR        = 1<<6,
        // for normal MO_ERROR and MO_WARNING messages
        APP_ERROR   = 1<<7,
        APP_WARNING = 1<<8
    };

    struct LogLine
    {
        Level level;
        QString string;
    };

    static NetworkLogger & instance();

    // -------- logging ----------

    /** Sets the bit-mask of messages that should be printed */
    static void setLevels(int levels);

    /** Connects all the signals of the socket to logging functions.
        Convenience function. */
    static void connectForLogging(QTcpSocket*);

    /** Starts writing a message (locked).
        @note Use MO_NETLOG() macro instead. */
    static void beginWrite(Level);
    /** Ends writing a message (unlock).
        @note Use MO_NETLOG() macro instead. */
    static void endWrite();

    /** Stream operator into an internal QTextStream */
    template <typename T>
    NetworkLogger& operator << (const T& stuff)
    {
        if (curLevel_ & acceptedLevels_)
            stream_ << stuff;
        return *this;
    }

signals:

    /** Emitted on endWrite() */
    void textAdded(int level, const QString& text);

private:

    static NetworkLogger * instance_;
    QList<LogLine> text_;
    Level curLevel_;
    std::stringstream stream_;
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
