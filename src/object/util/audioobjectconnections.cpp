/** @file audioobjectconnections.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include "audioobjectconnections.h"
#include "object/audioobject.h"
#include "io/datastream.h"
#include "io/error.h"

namespace MO {

AudioObjectConnection::AudioObjectConnection(
        AudioObject *from, AudioObject *to,
        uint outputChannel, uint inputChannel,
        uint numChannels)
    : p_from_           (from),
      p_to_             (to),
      p_outputChannel_  (outputChannel),
      p_inputChannel_   (inputChannel),
      p_numChannels_    (numChannels)
{

}

bool AudioObjectConnection::operator == (const AudioObjectConnection& o) const
{
    return from() == o.from()
            && to() == o.to()
            && outputChannel() == o.outputChannel()
            && inputChannel() == o.inputChannel()
            && numChannels() == o.numChannels();
}


AudioObjectConnections::AudioObjectConnections()
{
}

AudioObjectConnections::~AudioObjectConnections()
{
    clear();
}

void AudioObjectConnections::serialize(IO::DataStream & io) const
{
    io.writeHeader("aocons", 1);

    io << quint32(cons_.size());
    for (auto c : cons_)
        io << c->from()->idName() << c->to()->idName()
           << quint32(c->outputChannel())
           << quint32(c->inputChannel())
           << quint32(c->numChannels());
}

void AudioObjectConnections::deserialize(IO::DataStream & io, Object * rootObject)
{
    clear();

    io.readHeader("aocons", 1);

    quint32 num, outChan, inChan, numChan;
    QString fromId, toId;

    io >> num;

    if (num > 0)
    {
        QMap<QString, Object*> map;
        rootObject->getIdMap(map);

        for (uint i=0; i<num; ++i)
        {
            io >> fromId >> toId >> outChan >> inChan >> numChan;

            auto from = map.find(fromId),
                 to = map.find(toId);
            if (from == map.end() || to == map.end())
                MO_IO_WARNING(VERSION_MISMATCH, "Read of unknown connection " << fromId << "->" << toId)
            else
                connect(static_cast<AudioObject*>(from.value()),
                        static_cast<AudioObject*>(to.value()), outChan, inChan, numChan);
        }
    }


}


AudioObjectConnection * AudioObjectConnections::find(const AudioObjectConnection & c) const
{
    auto i = toMap_.find(c.to());
    if (i == toMap_.end())
        return 0;

    do
    {
        if (*(i->second) == c)
            return i->second;

        ++i;
    }
    while (i != toMap_.end() && i->second->to() == c.to());

    return 0;
}


QList<AudioObjectConnection*> AudioObjectConnections::getInputs(AudioObject * to) const
{
    QList<AudioObjectConnection*> list;
    auto i = toMap_.find(to);
    while (i != toMap_.end() && i->second->to() == to)
    {
        list << i->second;
        ++i;
    }
    return list;
}

QList<AudioObjectConnection*> AudioObjectConnections::getOutputs(AudioObject * from) const
{
    QList<AudioObjectConnection*> list;
    auto i = fromMap_.find(from);
    while (i != fromMap_.end() && i->second->from() == from)
    {
        list << i->second;
        ++i;
    }
    return list;
}


// ------------------------------ edit ----------------------------------

void AudioObjectConnections::clear()
{
    for (auto c : cons_)
        delete c;
    cons_.clear();
    toMap_.clear();
    fromMap_.clear();
}

AudioObjectConnection *AudioObjectConnections::connect(AudioObject *from,
                                                       AudioObject *to, uint outputChannel, uint inputChannel,
                                                       uint numChannels)
{
    return connect(AudioObjectConnection(from, to, outputChannel, inputChannel, numChannels));
}

AudioObjectConnection * AudioObjectConnections::connect(const AudioObjectConnection &connection)
{
    if (auto con = find(connection))
        return con;

    // create
    auto con = new AudioObjectConnection(connection);

    // install
    cons_.insert(con);
    toMap_.insert(std::make_pair(con->to(), con));
    fromMap_.insert(std::make_pair(con->from(), con));

    return con;
}

void AudioObjectConnections::disconnect(AudioObject *from, AudioObject *to,
                                        uint outputChannel, uint inputChannel,
                                        uint numChannels)
{
    disconnect(AudioObjectConnection(from, to, outputChannel, inputChannel, numChannels));
}

void AudioObjectConnections::disconnect(const AudioObjectConnection &con)
{
    if (auto c = find(con))
        disconnect(c);
}

void AudioObjectConnections::disconnect(AudioObjectConnection * con)
{
    for (auto i = toMap_.begin(); i != toMap_.end(); ++i)
    {
        if (i->second == con)
        {
            toMap_.erase(i);
            break;
        }
    }

    for (auto i = fromMap_.begin(); i != fromMap_.end(); ++i)
    {
        if (i->second == con)
        {
            fromMap_.erase(i);
            break;
        }
    }

    cons_.erase(con);

    delete con;
}

void AudioObjectConnections::remove(Object * obj)
{
    if (auto o = qobject_cast<AudioObject*>(obj))
    {
        toMap_.erase(o);
        fromMap_.erase(o);

        // remove from set

        std::set<AudioObjectConnection*> cons;

        for (auto i : cons_)
        {
            if (i->from() == o || i->to() == o)
                delete i;
            else
                cons.insert(i);
        }
        cons_.swap(cons);
    }

    // children
    for (auto c : obj->childObjects())
        remove(c);
}


} // namespace MO
