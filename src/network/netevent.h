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
#include "clientstate.h"
#include "types/float.h"

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
    QString infoName() const Q_DECL_OVERRIDE;                                   \
    virtual Class__ * cloneClass() const Q_DECL_OVERRIDE                        \
                        { return new Class__(); }                               \
    virtual void serialize(IO::DataStream & io) const Q_DECL_OVERRIDE;          \
    virtual void deserialize(IO::DataStream & io) Q_DECL_OVERRIDE;              \

#define MO_REGISTER_NETEVENT(Class__)                                           \
    namespace { static const bool registered##Class__ =                         \
        AbstractNetEvent::registerEventClass(new Class__); }

class AbstractNetEvent
{
    friend class EventCom;
public:
    AbstractNetEvent();
    virtual ~AbstractNetEvent() { }

    // ------------ factory ---------------

    static bool registerEventClass(AbstractNetEvent*);

    static AbstractNetEvent * createClass(const QString& className);

    virtual AbstractNetEvent * cloneClass() const = 0;

    // ------------ getter ----------------

    virtual const QString& className() const = 0;

    /** Should return something to clearly identify the event (for debugging).
        Base implementation returns className() + counter() */
    virtual QString infoName() const;

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
        /** Requests a NetEventClientState event from the client */
        GET_CLIENT_STATE,
        /** Sets the client index (int) */
        SET_CLIENT_INDEX,
        /** Sets the index of the screen to be used for windows */
        SET_DESKTOP_INDEX,
        /** Shows the fullscreen info window */
        SHOW_INFO_WINDOW,
        /** Hides the fullscreen info window */
        HIDE_INFO_WINDOW,
        SHOW_RENDER_WINDOW,
        HIDE_RENDER_WINDOW,
        START_RENDER,
        STOP_RENDER,
        /** Sets the default ProjectionSystemSettings (QByteArray) */
        SET_PROJECTION_SETTINGS,
        /** Tells server to send a NetEventFileInfo for filename (QString) */
        GET_SERVER_FILE_TIME,
        /** Tells server to send a NetEventFile for filename (QString) */
        GET_SERVER_FILE,
    };

    static QString requestName(Request);

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


/** Clients current state info event - typically an answer to NetEventRequest */
class NetEventClientState : public AbstractNetEvent
{
public:
    MO_NETEVENT_CONSTRUCTOR(NetEventClientState)

    // --------- getter -------------------

    ClientState & state() { return state_; }

private:

    friend class ClientEngine;

    ClientState state_;
};


/** Clients current state info event - typically an answer to NetEventRequest */
class NetEventTime : public AbstractNetEvent
{
public:
    MO_NETEVENT_CONSTRUCTOR(NetEventTime)

    // --------- getter -------------------

    Double time() { return time_; }

    // --------- setter -------------------

    void setTime(Double t) { time_ = t; }

private:

    Double time_;
};

} // namespace MO

#endif // MOSRC_NETWORK_NETEVENT_H
