/** @file netevent.h

    @brief Abstract network event

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/8/2014</p>
*/

#ifndef MOSRC_NETWORK_NETEVENT_H
#define MOSRC_NETWORK_NETEVENT_H

#include <QString>

class QIODevice;
class QAbstractSocket;

namespace MO {
namespace IO { class DataStream; }

#define MO_NETEVENT_CONSTRUCTOR(Class__)                                        \
    Class__();                                                                  \
    const QString& className() const Q_DECL_OVERRIDE                            \
                        { static QString s(#Class__); return s; }               \
    virtual void serialize(IO::DataStream & io) const Q_DECL_OVERRIDE;          \
    virtual void deserialize(IO::DataStream & io) Q_DECL_OVERRIDE;              \

#define MO_NETEVENT_CONSTRUCTOR_1(Class__, Par1__)                              \
    Class__(Par1__);                                                            \
    const QString& className() const Q_DECL_OVERRIDE                            \
                        { static QString s(#Class__); return s; }               \
    virtual void serialize(IO::DataStream & io) const Q_DECL_OVERRIDE;          \
    virtual void deserialize(IO::DataStream & io) Q_DECL_OVERRIDE;              \



class AbstractNetEvent
{
public:
    AbstractNetEvent();
    virtual ~AbstractNetEvent() { }

    // ------------ getter ----------------

    virtual const QString& className() const = 0;

    bool isValid() const { return isValid_; }
    bool isReceived() const { return isReceived_; }
    bool isSend() const { return isSend_; }

    // ------------ io --------------------

    virtual void serialize(IO::DataStream & io) const = 0;
    virtual void deserialize(IO::DataStream & io) = 0;

    // ---------- network -----------------

    bool send(QAbstractSocket*);

private:

    void serialize_(QIODevice & io) const;
    void deserialize_(QIODevice & io);

    bool isValid_, isReceived_, isSend_;
};


class NetInfoEvent : public AbstractNetEvent
{
public:
    MO_NETEVENT_CONSTRUCTOR_1(NetInfoEvent, const QString& id)

    // --------- getter -------------------

    const QString& id() const { return id_; }
    const QString& info() const { return info_; }


private:

    QString id_, info_;
};




} // namespace MO

#endif // MOSRC_NETWORK_NETEVENT_H
