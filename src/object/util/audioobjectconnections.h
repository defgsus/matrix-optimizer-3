/** @file audioobjectconnections.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_AUDIOOBJECTCONNECTIONS_H
#define MOSRC_OBJECT_UTIL_AUDIOOBJECTCONNECTIONS_H

#include <map>
#include <set>

#include <QList>

#include "types/int.h"

namespace MO {
namespace IO { class DataStream; }

class AudioObject;
class Object;

class AudioObjectConnection
{
public:
    AudioObjectConnection(AudioObject * from, AudioObject * to,
               uint outputChannel = 0, uint inputChannel = 0,
               uint numChannels = 1);

    bool operator == (const AudioObjectConnection& other) const;
    bool operator != (const AudioObjectConnection& other) const
        { return !(*this == other); }

    AudioObject * from() const { return p_from_; }
    AudioObject * to() const { return p_to_; }
    uint outputChannel() const { return p_outputChannel_; }
    uint inputChannel() const { return p_inputChannel_; }
    uint numChannels() const { return p_numChannels_; }

private:
    AudioObject * p_from_, * p_to_;
    uint p_outputChannel_, p_inputChannel_,
         p_numChannels_;
};

/** Container for holding the edges between AudioObjects */
class AudioObjectConnections
{
public:


    // --------------- ctor --------------------------

    AudioObjectConnections();
    ~AudioObjectConnections();

    // -------------------- io -----------------------

    void serialize(IO::DataStream&) const;
    void deserialize(IO::DataStream&, Object * rootObject);

    void dump(std::ostream&) const;

    // ------------- stl container -------------------

    std::set<AudioObjectConnection*>::iterator begin() { return cons_.begin(); }
    std::set<AudioObjectConnection*>::iterator end() { return cons_.end(); }
    std::set<AudioObjectConnection*>::const_iterator begin() const { return cons_.cbegin(); }
    std::set<AudioObjectConnection*>::const_iterator end() const { return cons_.cend(); }

    // --------------- getter ------------------------

    bool contains(AudioObject * o) const { return toMap_.find(o) != toMap_.end()
                                               || fromMap_.find(o) != fromMap_.end(); }

    bool contains(const AudioObjectConnection& c) const { return find(c); }

    AudioObjectConnection * find(const AudioObjectConnection&) const;

    QList<AudioObjectConnection*> getInputs(AudioObject*) const;
    QList<AudioObjectConnection*> getOutputs(AudioObject*) const;

    /** Returns true when the graph contains a loop */
    bool hasLoop() const;

    /** Returns false if the connection would create a loop.
        @note This also returns false when there is a loop
        in the graph already! */
    bool isSaveToAdd(AudioObject *from, AudioObject *to);

    // ----------------- edit ------------------------

    void clear();

    AudioObjectConnection * connect(const AudioObjectConnection&);

    AudioObjectConnection * connect(
                         AudioObject * from, AudioObject * to,
                         uint outputChannel = 0, uint inputChannel = 0,
                         uint numChannels = 1);

    bool disconnect(AudioObject * from, AudioObject * to,
                    uint outputChannel = 0, uint inputChannel = 0,
                    uint numChannels = 1);

    bool disconnect(const AudioObjectConnection& con);
    void disconnect(AudioObjectConnection *);

    /** Removes all objects recursively */
    void remove(Object *);

private:

    // disable copy
    AudioObjectConnections(const AudioObjectConnections&);
    AudioObjectConnections& operator = (const AudioObjectConnections&);

    std::multimap<AudioObject*, AudioObjectConnection*>
        toMap_, fromMap_;
    std::set<AudioObjectConnection*> cons_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_AUDIOOBJECTCONNECTIONS_H
