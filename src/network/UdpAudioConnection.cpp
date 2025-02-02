/** @file udpaudioconnection.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.01.2015</p>
*/

#include <memory>
#include <vector>

#include <QDataStream>
#include <QMap>

#include "UdpAudioConnection.h"
#include "UdpConnection.h"
#include "audio/tool/AudioBuffer.h"
#include "object/AudioObject.h"
#include "network/netlog.h"
#include "io/Settings.h"
#include "io/log.h"
#include "io/error.h"

namespace MO {

struct UdpAudioConnection::Private
{
    // structure per AUDIO::AudioBuffer
    struct Buffer
    {
        AudioObject * ao;
        AUDIO::AudioBuffer * buf;
        uint channel;
        QString id;
        SamplePos curPos;
    };


    // ---------- ctor ----------

    Private(UdpAudioConnection * p)
        : p   (p),
          udp (new UdpConnection(p))
    {
    }

    ~Private()
    {
        udp->close();
    }

    static void constructPacket_(QByteArray&, Buffer* b);
    void deconstructPacket_(const QByteArray& data);

    void createBuffer(AUDIO::AudioBuffer * buf, AudioObject * obj, uint channel);
    Buffer * bufferFor(AUDIO::AudioBuffer * );
    Buffer * bufferFor(const QString& id);

    std::vector<std::shared_ptr<Buffer>> bufferStore;
    QMap<AUDIO::AudioBuffer*, Buffer*> buffers;
    QMap<QString, Buffer*> buffersId;

    UdpAudioConnection * p;
    UdpConnection * udp;

};

UdpAudioConnection::UdpAudioConnection(QObject *parent)
    :   QObject (parent),
        p_      (new Private(this))
{
    MO_DEBUG_UDP("UdpAudioConnection::UdpAudioConnection(" << parent << ")");
    MO_NETLOG(CTOR, "UdpAudioConnection::UdpAudioConnection()");

    connect(p_->udp, SIGNAL(dataReady()), this, SLOT(receive_()));
}

UdpAudioConnection::~UdpAudioConnection()
{
    MO_DEBUG_UDP("UdpAudioConnection::~UdpAudioConnection()");
    MO_NETLOG(CTOR, "UdpAudioConnection::~UdpAudioConnection()");

    delete p_;
}


void UdpAudioConnection::clear()
{
    p_->buffers.clear();
    p_->buffersId.clear();
    p_->bufferStore.clear();
}

bool UdpAudioConnection::addBuffer(AUDIO::AudioBuffer *buffer, AudioObject *obj, uint channel)
{
    MO_DEBUG_UDP("UdpAudioConnection::addBuffer("
             << buffer << ", " << obj->idName() << ", " << channel << ")");

    if (p_->bufferFor(buffer))
    {
        MO_WARNING("UdpAudioConnection: add of duplicate buffer "
                   << buffer << ", forgot clear() ? fix : check");
        return false;
    }

    p_->createBuffer(buffer, obj, channel);

    return true;
}

bool UdpAudioConnection::isOpen() const
{
    return p_->udp->isOpen();
}


bool UdpAudioConnection::openForRead()
{
    MO_ASSERT(isClient(), "wrong request");

    return p_->udp->openMulticastRead(settings()->udpAudioMulticastAddress(),
                                      settings()->udpAudioMulticastPort());
}

void UdpAudioConnection::close()
{
    p_->udp->close();
}

#ifndef MO_DISABLE_SERVER
bool UdpAudioConnection::sendAudioBuffer(AUDIO::AudioBuffer * buf, SamplePos pos)
{
    MO_ASSERT( isServer(), "UdpAudioConnection: send not sensible for clients");

    MO_DEBUG_UDP("UdpAudioConnection::sendBuffer(" << buf << ", " << pos << ")");
    MO_ASSERT( p_->bufferFor(buf), "audio buffer " << buf << " unknown to UdpAudioConnection");

    // get buffer
    Private::Buffer * b = p_->bufferFor(buf);
    if (!b)
    {
        MO_NETLOG(WARNING, "UdpAudioConnection::sendAudioBuffer("
                  << buf << ", " << pos << ") for unknown buffer");
        return false;
    }
    // update buffer info
    b->curPos = pos;

    // repackage
    QByteArray data;
    Private::constructPacket_(data, b);

    // send off
    return p_->udp->sendDatagram(data, QHostAddress(settings()->udpAudioMulticastAddress()),
                                       settings()->udpAudioMulticastPort());
}
#endif

void UdpAudioConnection::receive_()
{
    MO_DEBUG_UDP("UdpAudioConnection::receive()");

    QByteArray data = p_->udp->readData();
    p_->deconstructPacket_(data);
}







void UdpAudioConnection::Private::constructPacket_(QByteArray & data, Buffer * b)
{
    QDataStream s(&data, QIODevice::WriteOnly);

    s << "_audio_" << b->id << quint32(b->channel) << quint64(b->curPos) << quint32(b->buf->blockSize());
    s.writeRawData((const char*)b->buf->readPointer(), b->buf->blockSizeBytes());
}

void UdpAudioConnection::Private::deconstructPacket_(const QByteArray& data)
{
    if (data.startsWith("_audio_"))
    {
        MO_NETLOG(WARNING, "UdpAudioConnection: received unknown data (" << data.size() << " bytes)");
        return;
    }

    QDataStream s(data);

    // reconstruct
    QString idName;
    quint32 channel;
    quint64 pos;
    quint32 blockSize;
    // skip header
    s >> idName;
    // read stuff
    s >> idName >> channel >> pos >> blockSize;

    Buffer * b = bufferFor(idName);
    if (!b)
    {
        MO_NETLOG(WARNING, "UdpAudioConnection: received unknown buffer '" << idName << "'");
#ifndef NDEBUG
        for (auto b : buffers)
            MO_PRINT(b->id);
#endif
        return;
    }

    if (blockSize != b->buf->blockSize())
    {
        MO_NETLOG(WARNING, "UdpAudioConnection: blocksize mismatch, received " << blockSize << ", should be " << b->buf->blockSize());
        return;
    }

    if (channel != b->channel)
    {
        MO_NETLOG(WARNING, "UdpAudioConnection: channel mismatch, received " << channel << ", should be " << b->channel);
        /*return; not really an error, is it?*/
    }

    // read and forward pointer
    s.readRawData((char *)b->buf->writePointer(), b->buf->blockSizeBytes());
    b->buf->nextBlock();
    // ZZZ
    b->ao->clientFakeAudio(RenderTime(pos, b->ao->sampleRate(), b->buf->blockSize(), MO_AUDIO_THREAD));
    b->curPos = pos;

    MO_DEBUG_UDP("UdpAudioConnection: received buffer "
             << idName << ", " << blockSize << ", " << channel << ", " << pos
             //<< "\n" << b->buf->toAscii()
             );
}


void UdpAudioConnection::Private::createBuffer(AUDIO::AudioBuffer *buf, AudioObject *obj, uint channel)
{
    MO_ASSERT(!bufferFor(buf), "");

    auto b = new Buffer();
    auto shp = std::shared_ptr<Buffer>(b);

    b->ao = obj;
    b->buf = buf;
    b->channel = channel;
    b->curPos = 0;
    b->id = QString("%1_%2").arg(obj->idName()).arg(channel);

    buffers.insert(b->buf, b);
    buffersId.insert(b->id, b);
    bufferStore.push_back(shp);
}

UdpAudioConnection::Private::Buffer *
UdpAudioConnection::Private::bufferFor(AUDIO::AudioBuffer * ab)
{
    auto i = buffers.find(ab);
    return (i == buffers.end()) ? 0 : i.value();
}

UdpAudioConnection::Private::Buffer *
UdpAudioConnection::Private::bufferFor(const QString& id)
{
    auto i = buffersId.find(id);
    return (i == buffersId.end()) ? 0 : i.value();
}

} // namespace MO
