/** @file audioobjectconnections.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include <QStack>

#include "audioobjectconnections.h"
#include "object/audioobject.h"
#include "graph/directedgraph.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

#if 0
#   define MO__DEBUG(arg__) MO_DEBUG("AudioObjectConnection::" << arg__);
#else
#   define MO__DEBUG(arg__)
#endif

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

std::ostream& operator << (std::ostream& out, const AudioObjectConnection& c)
{
    out << "Con(" << c.from()->name() << ":" << c.outputChannel()
           << ", " << c.to()->name() << ":" << c.inputChannel()
           << ", " << c.numChannels() << ")";
    return out;
}

AudioObjectConnections::AudioObjectConnections()
{
}

AudioObjectConnections::~AudioObjectConnections()
{
    clear();
}

AudioObjectConnections::AudioObjectConnections(const AudioObjectConnections & other)
{
    copyFrom(other);
}

AudioObjectConnections& AudioObjectConnections::operator =(const AudioObjectConnections& other)
{
    copyFrom(other);
    return *this;
}

void AudioObjectConnections::copyFrom(const AudioObjectConnections& other)
{
    clear();
    for (auto c : other)
        connect(*c);
}

void AudioObjectConnections::addFrom(const AudioObjectConnections& other)
{
    for (auto c : other)
        connect(*c);
}

void AudioObjectConnections::swap(AudioObjectConnections &other)
{
    toMap_.swap(other.toMap_);
    fromMap_.swap(other.fromMap_);
    cons_.swap(other.cons_);
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

void AudioObjectConnections::dump(std::ostream & out) const
{
    out << "AudioObjectConnections:\n";
    for (AudioObjectConnection * c : cons_)
        out << " " << c->from()->name() << "->" << c->to()->name();
    out << "\n--fromMap--\n";
    for (auto i = fromMap_.begin(); i!=fromMap_.end(); ++i)
        out << i->first << ", " << i->second << "\n";
    out << "--toMap--\n";
    for (auto i = toMap_.begin(); i!=toMap_.end(); ++i)
        out << i->first << ", " << i->second << "\n";
    out << std::endl;
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
    auto range = toMap_.equal_range(to);
    auto i = range.first;
    while (i != toMap_.end() && i != range.second)
    {
        list << i->second;
        ++i;
    }
    return list;
}

QList<AudioObjectConnection*> AudioObjectConnections::getOutputs(AudioObject * from) const
{
    QList<AudioObjectConnection*> list;
    auto range = fromMap_.equal_range(from);
    auto i = range.first;
    while (i != fromMap_.end() && i != range.second)
    {
        list << i->second;
        ++i;
    }
    return list;
}


bool AudioObjectConnections::hasLoop() const
{
    if (cons_.empty())
        return false;

    // create a directed graph
    DirectedGraph<AudioObject*> graph;
    for (auto c : cons_)
        graph.addEdge(c->from(), c->to());

    if (graph.beginnings().isEmpty())
        return false;

    std::vector<AudioObject*> temp;
    // makeLinear returns false when loop encountered
    return !graph.makeLinear(temp);
}

bool AudioObjectConnections::isSaveToAdd(AudioObject * from, AudioObject * to)
{
    if (cons_.empty())
        return true;

    // create a directed graph
    DirectedGraph<AudioObject*> graph;
    for (auto c : cons_)
        graph.addEdge(c->from(), c->to());

    // add the desired connection
    graph.addEdge(from, to);

    // no beginnings after we added the edge?
    // means we created a loop already
    if (graph.beginnings().isEmpty())
        return false;

    std::vector<AudioObject*> temp;
    // makeLinear returns false when loop encountered
    return graph.makeLinear(temp);
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
    MO__DEBUG("connect(" << connection << ")");

    if (AudioObjectConnection * con = find(connection))
    {
        MO_WARNING("AudioObjectConnections::connect() duplicate connection requested " << connection);
        return con;
    }

    // create
    auto con = new AudioObjectConnection(connection);

    // install
    cons_.insert(con);
    toMap_.insert(std::make_pair(con->to(), con));
    fromMap_.insert(std::make_pair(con->from(), con));

    hasLoop();

    return con;
}

bool AudioObjectConnections::disconnect(AudioObject *from, AudioObject *to,
                                        uint outputChannel, uint inputChannel,
                                        uint numChannels)
{
    return disconnect(AudioObjectConnection(from, to, outputChannel, inputChannel, numChannels));
}

bool AudioObjectConnections::disconnect(const AudioObjectConnection &con)
{
    if (auto c = find(con))
    {
        disconnect(c);
        return true;
    }
    return false;
}

void AudioObjectConnections::disconnect(AudioObjectConnection * con)
{
    MO__DEBUG("disconnect(" << *con << ")");

    // remove here
    for (auto i = toMap_.begin(); i != toMap_.end(); ++i)
    {
        if (i->second == con)
        {
            toMap_.erase(i);
            break;
        }
    }

    // remove there
    for (auto i = fromMap_.begin(); i != fromMap_.end(); ++i)
    {
        if (i->second == con)
        {
            fromMap_.erase(i);
            break;
        }
    }

    // and also
    cons_.erase(con);

    delete con;
}

void AudioObjectConnections::remove(Object *obj)
{
    MO__DEBUG("remove(" << obj << ")");

    if (auto o = qobject_cast<AudioObject*>(obj))
    {
        // remove from maps
        std::multimap<AudioObject*, AudioObjectConnection*>
                toMap, fromMap;
        for (auto i : toMap_)
            if (i.second->from() != o && i.second->to() != o)
                toMap.insert(i);
        if (toMap.size() != toMap_.size())
            toMap_.swap(toMap);

        for (auto i : fromMap_)
            if (i.second->from() != o && i.second->to() != o)
                fromMap.insert(i);
        if (fromMap.size() != fromMap_.size())
            fromMap_.swap(fromMap);

        // remove from set
        std::set<AudioObjectConnection*> cons;

        for (auto i : cons_)
        {
            if (i->from() == o || i->to() == o)
                delete i;
            else
                cons.insert(i);
        }
        if (cons.size() != cons_.size())
            cons_.swap(cons);
    }

    // children
    for (auto c : obj->childObjects())
        remove(c);
}

AudioObjectConnections AudioObjectConnections::reducedTo(const Object* tree) const
{
    QList<AudioObject*> list = tree->findChildObjects<AudioObject>(QString(), true);

    AudioObjectConnections cons;

    // go through all connections
    for (AudioObjectConnection* c : *this)
    {
        if (list.contains(c->from()) && list.contains(c->to()))
            cons.connect(*c);
    }

    return cons;
}


} // namespace MO
