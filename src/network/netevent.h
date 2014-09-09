/** @file netevent.h

    @brief Abstract network event

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/8/2014</p>
*/

#ifndef MOSRC_NETWORK_NETEVENT_H
#define MOSRC_NETWORK_NETEVENT_H

#include <QString>
#include <QMap>


class QIODevice;
class QAbstractSocket;

namespace MO {
namespace IO { class DataStream; }



#define MO_NETEVENT_CONSTRUCTOR(Class__)                                        \
    Class__();                                                                  \
    const QString& className() const Q_DECL_OVERRIDE                            \
                        { static QString s(#Class__); return s; }               \
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

    // ------------ io --------------------

    virtual void serialize(IO::DataStream & io) const = 0;
    virtual void deserialize(IO::DataStream & io) = 0;

    // ---------- network -----------------

    /** Serializes the event onto the socket stream */
    bool send(QAbstractSocket*);

    /** Creates the appropriate event class from the socket stream.
        Returns the event if succesful, or NULL otherwise */
    static AbstractNetEvent * receive(QAbstractSocket *);

private:

    void serialize_(QIODevice & io) const;

    bool isValid_, isReceived_, isSend_;

    static QMap<QString, AbstractNetEvent*> registeredEvents_;
};


class NetInfoEvent : public AbstractNetEvent
{
public:
    MO_NETEVENT_CONSTRUCTOR(NetInfoEvent)

    // --------- getter -------------------

    const QString& id() const { return id_; }
    const QString& info() const { return info_; }

    // --------- setter -------------------

    void setId(const QString& id) { id_ = id; }
    void setInfo(const QString& info) { info_ = info; }

private:

    QString id_, info_;
};




} // namespace MO

#endif // MOSRC_NETWORK_NETEVENT_H
