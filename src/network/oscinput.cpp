/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#include <QMap>
#include <QDebug>

#include "oscinput.h"
#include "network/udpconnection.h"
#include "types/float.h"

namespace MO {

struct OscInput::Private
{
    Private(OscInput * p)
        : p     (p)
        , udp   (new UdpConnection)
    {
        connect(udp, &UdpConnection::dataReady, [=]() { readData(); } );
    }

    ~Private()
    {
        udp->releaseRef();
    }

    void readData();
    void readMessage(const QByteArray&);

    OscInput * p;
    UdpConnection * udp;
    QMap<QString, QVariant> valueMap;
};



OscInput::OscInput(QObject * parent)
    : QObject       (parent)
    , RefCounted    ()
    , p_            (new Private(this))
{

}

OscInput::~OscInput()
{
    delete p_;
}

bool OscInput::open(const QHostAddress &addr, uint16_t port)
{
    return p_->udp->open(addr, port);
}

bool OscInput::open(uint16_t port)
{
    return p_->udp->open(port);
}

void OscInput::close()
{
    p_->udp->close();
}

bool OscInput::hasValue(const QString &id) const
{
    return p_->valueMap.contains(id);
}

uint16_t OscInput::port() const
{
    return p_->udp->port();
}

const QVariant& OscInput::value(const QString &id) const
{
    static const QVariant invalid;

    auto i = p_->valueMap.find(id);
    return (i == p_->valueMap.end())
            ? invalid
            : i.value();
}

const QMap<QString, QVariant>& OscInput::values() const
{
    return p_->valueMap;
}

namespace {

    /** Read POD type data from raw osc stream */
    template <typename T>
    T get_value(const char * d)
    {
    #if Q_BYTE_ORDER == Q_BIG_ENDIAN
        return *(reinterpret_cast<T*>(d));
    #else
        const size_t s = sizeof(T);
        char v[s];
        for (size_t i=0; i<s; ++i)
            v[i] = d[s-1-i];
        const auto t = reinterpret_cast<const T*>(v);
        return *t;
    #endif
    }
}

void OscInput::Private::readData()
{
    while (udp->isData())
        readMessage(udp->readData());
}

void OscInput::Private::readMessage(const QByteArray & data)
{
    //qInfo() << data;

    const QString id = QString::fromLatin1(data);
    if (id.isEmpty())
        return;

    // go to end of zeros after id
    int i = id.size();
    while (i < data.size() && (char)data[i] == 0)
        ++i;
    if (i >= data.size() - 1)
        return;

    if (data[i] != ',')
        return;
    auto fmt = data[++i];

    // go to end of zeros after format(s)
    while (i < data.size() && (char)data[i] != 0)
        ++i;

    i = (i/4 + 1) * 4;

    //while (i < data.size() && data[i] == 0)
    //    ++i;

    //qInfo() << data << i << "/" << data.size() << fmt;

    switch (fmt)
    {
        default: return;

        case 'f':
            if (i > data.size() - (int)sizeof(F32))
                return;
            valueMap.insert(id, get_value<F32>(&data.data()[i]));
        break;

        case 'i':
            if (i > data.size() - (int)sizeof(int32_t))
                return;
            valueMap.insert(id, get_value<int32_t>(&data.data()[i]));
        break;
    }

    //qInfo() << p->value(id);

    emit p->valueChanged(id);
}



} // namespace MO
