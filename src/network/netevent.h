/** @file netevent.h

    @brief Abstract network event

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/8/2014</p>
*/

#ifndef MOSRC_NETWORK_NETEVENT_H
#define MOSRC_NETWORK_NETEVENT_H

#include <atomic>

#include <QString>
#include <QMap>
#include <QVariant>
#include <QDateTime>

#include "io/systeminfo.h"

class QIODevice;
class QAbstractSocket;

namespace MO {

class Scene;

namespace IO { class DataStream; }


#define MO_NETEVENT_CONSTRUCTOR(Class__)                                        \
    Class__();                                                                  \
    static const QString& staticClassName()                                     \
                        { static QString s(#Class__); return s; }               \
    const QString& className() const Q_DECL_OVERRIDE                            \
                                    { return staticClassName(); }               \
    virtual Class__ * cloneClass() const Q_DECL_OVERRIDE                        \
                        { return new Class__(); }                               \
    virtual void serialize(IO::DataStream & io) const Q_DECL_OVERRIDE;          \
    virtual void deserialize(IO::DataStream & io) Q_DECL_OVERRIDE;              \

#define MO_REGISTER_NETEVENT(Class__)                                           \
    namespace { static const bool registered##Class__ =                         \
        AbstractNetEvent::registerEventClass(new Class__); }

class AbstractNetEvent
{
public:
    AbstractNetEvent();
    virtual ~AbstractNetEvent() { }

    // ------------ factory ---------------

    static bool registerEventClass(AbstractNetEvent*);

    static AbstractNetEvent * createClass(const QString& className);

    virtual AbstractNetEvent * cloneClass() const = 0;

    // ------------ getter ----------------

    virtual const QString& className() const = 0;

    bool isValid() const { return isValid_; }
    bool isReceived() const { return isReceived_; }
    bool isSend() const { return isSend_; }

    QAbstractSocket * sender() const { return socket_; }

    /** Returns the internal counter to identify the event */
    int counter() const { return counter_; }

    // ------------ io --------------------

    virtual void serialize(IO::DataStream & io) const = 0;
    virtual void deserialize(IO::DataStream & io) = 0;

    // ---------- network -----------------

    /** Serializes the event onto the socket stream.
        If @p socket is NULL, the previously used socket will be used. */
    bool send(QAbstractSocket * socket = 0) noexcept;

    /** Creates the appropriate event class from the socket stream.
        Returns the event if succesful, or NULL otherwise */
    static AbstractNetEvent * receive(QAbstractSocket *) noexcept;

    /** Creates a new event as response to this incoming event.
        counter() and socket() will have the same value.
        If className is unknown, NULL is returned. */
    AbstractNetEvent * createResponse(const QString& className) const;

    /** Creates a new event as response to this incoming event.
        counter() and socket() will have the same value.
        If className is unknown, an IoException will be thrown. */
    AbstractNetEvent * createResponseThrow(const QString& className) const;

    /** Creates a new event as response to this incoming event.
        counter() and socket() will have the same value.
        @throws IoException will be thrown if something goes wrong. */
    template <class E>
    E * createResponse() const { return static_cast<E*>(createResponseThrow(E::staticClassName())); }

private:

    void serialize_(QIODevice & io) const;

    bool isValid_, isReceived_, isSend_;

    static QMap<QString, AbstractNetEvent*> registeredEvents_;

    QAbstractSocket * socket_;

    int counter_;
    static std::atomic_int global_counter_;
};

template <class E>
E * netevent_cast(AbstractNetEvent * e)
{
    return e->className() == E::staticClassName() ? static_cast<E*>(e) : 0;
}






class NetEventRequest : public AbstractNetEvent
{
public:
    /** Obviously, the order of these must be the same
        for server and clients! */
    enum Request
    {
        NONE,
        /** Requests a NetEventSysInfo event */
        GET_SYSTEM_INFO,
        /** Requests a NetEventInfo with the current index (int) */
        GET_CLIENT_INDEX,
        /** Sets the client index (int) */
        SET_CLIENT_INDEX,
        /** Shows the fullscreen info window */
        SHOW_INFO_WINDOW,
        /** Hides the fullscreen info window */
        HIDE_INFO_WINDOW,
        /** Sets the default ProjectionSystemSettings (QByteArray) */
        SET_PROJECTION_SETTINGS,
        /** Tells server to send a NetEventFileInfo for filename (QString) */
        GET_SERVER_FILE_TIME,
        /** Tells server to send a NetEventFile for filename (QString) */
        GET_SERVER_FILE
    };

    MO_NETEVENT_CONSTRUCTOR(NetEventRequest)

    // --------- getter -------------------

    Request request() const { return request_; }

    /** The data that is associated with the request */
    const QVariant& data() const { return data_; }

    // --------- setter -------------------

    void setRequest(Request id) { request_ = id; }
    void setData(const QVariant& d) { data_ = d; }

private:

    Request request_;
    QVariant data_;
};


/** General info event - typically an answer to NetEventRequest */
class NetEventInfo : public AbstractNetEvent
{
public:
    MO_NETEVENT_CONSTRUCTOR(NetEventInfo)

    // --------- getter -------------------

    /** Returns the type of the initial request */
    NetEventRequest::Request request() const { return request_; }

    /** Returns the answer to the initial request */
    const QVariant& data() const { return data_; }

    // --------- setter -------------------

    void setRequest(NetEventRequest::Request r) { request_ = r; }
    void setData(const QVariant& v) { data_ = v; }

private:

    NetEventRequest::Request request_;
    QVariant data_;
};


/** SystemInfo event */
class NetEventSysInfo : public AbstractNetEvent
{
public:
    MO_NETEVENT_CONSTRUCTOR(NetEventSysInfo)

    // --------- getter -------------------

    const SystemInfo& info() const { return info_; }

    // --------- setter -------------------

    void setInfo(const SystemInfo& info) { info_ = info; }

    /** Fills the info() with the current system info */
    void getInfo();

private:

    SystemInfo info_;
};


/** File info event - answer to NetEventRequest::GET_SERVER_FILE_TIME */
class NetEventFileInfo : public AbstractNetEvent
{
public:
    MO_NETEVENT_CONSTRUCTOR(NetEventFileInfo)

    // --------- getter -------------------

    /** The filename of the querried file */
    const QString& filename() const { return filename_; }

    /** The timestamp of the original file */
    const QDateTime& time() const { return time_; }

    /** File was found? */
    bool isPresent() const { return present_; }

    // --------- setter -------------------

    void setFilename(const QString& fn) { filename_ = fn; }
    void setTime(const QDateTime& t) { time_ = t; }
    void setPresent(bool p) { present_ = p; }

    /** Convenience function - sets all necessary data */
    void getFileTime();

private:

    QString filename_;
    QDateTime time_;
    bool present_;
};




/** File transfer vehicle - generally an answer to NetEventRequest::GET_SERVER_FILE */
class NetEventFile : public AbstractNetEvent
{
public:
    MO_NETEVENT_CONSTRUCTOR(NetEventFile)

    // --------- getter -------------------

    /** The filename of the transferred file */
    const QString& filename() const { return filename_; }

    /** The timestamp of the original file */
    const QDateTime& time() const { return time_; }

    /** File was found? */
    bool isPresent() const { return present_; }

    /** The whole file data */
    const QByteArray& data() { return data_; }

    // --------- setter -------------------

    void setFilename(const QString& fn) { filename_ = fn; }
    void setTime(const QDateTime& t) { time_ = t; }
    void setPresent(bool p) { present_ = p; }
    void setData(const QByteArray& a) { data_ = a; }

    /** Convenience function - sets all necessary data */
    void loadFile(const QString& fn);

    /** Saves the data to a file and returns the file's timestamp */
    bool saveFile(const QString& fn) const;

private:

    QString filename_;
    QDateTime time_;
    bool present_;
    QByteArray data_;
};




/** Sends a whole Scene */
class NetEventScene : public AbstractNetEvent
{
public:
    MO_NETEVENT_CONSTRUCTOR(NetEventScene)

    // --------- getter -------------------

    /** Constructs a scene object.
        On errors, NULL is returned. */
    Scene * getScene() const;

    // --------- setter -------------------

    /** Serializes the scene object.
        Returns success. Throws nothing. */
    bool setScene(const Scene * scene);

private:

    QByteArray data_;
};


} // namespace MO

#endif // MOSRC_NETWORK_NETEVENT_H
