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

#include <QString>
#include <QList>
#include <QMap>

#include "types/int.h"

namespace MO {
namespace IO { class DataStream; }

class AudioObject;
class Object;

class AudioObjectConnection
{
public:
    AudioObjectConnection();
    AudioObjectConnection(AudioObject * from, AudioObject * to,
               uint outputChannel = 0, uint inputChannel = 0,
               uint numChannels = 1);

    bool operator == (const AudioObjectConnection& other) const;
    bool operator != (const AudioObjectConnection& other) const
        { return !(*this == other); }

    const QString& fromId() const { return p_fromId_; }
    const QString& toId() const { return p_toId_; }
    AudioObject * from() const { return p_from_; }
    AudioObject * to() const { return p_to_; }
    uint outputChannel() const { return p_outputChannel_; }
    uint inputChannel() const { return p_inputChannel_; }
    uint numChannels() const { return p_numChannels_; }

private:
    friend class AudioObjectConnections;
    QString p_fromId_, p_toId_;
    AudioObject * p_from_, * p_to_;
    uint p_outputChannel_, p_inputChannel_,
         p_numChannels_;
};

std::ostream& operator << (std::ostream& out, const AudioObjectConnection&);



/** Container for holding the edges between AudioObjects.
    <p>This is the actual data structure used by Scene to store the
    connections between audio objects. The information is [de]serialized in
    Scene::[de]serializeAfterChilds().
    */
class AudioObjectConnections
{
public:


    // --------------- ctor --------------------------

    AudioObjectConnections();
    ~AudioObjectConnections();

    AudioObjectConnections(const AudioObjectConnections&);
    AudioObjectConnections& operator = (const AudioObjectConnections&);

    void swap(AudioObjectConnections& other);
    void copyFrom(const AudioObjectConnections& other);
    void addFrom(const AudioObjectConnections& other);

    // -------------------- io -----------------------

    void serialize(IO::DataStream&) const;
    /** Reads connections from io stream.
        @p rootObject is used to querry the pointers for each connected object. */
    void deserialize(IO::DataStream&, Object * rootObject);
    /** Reads connections from io stream.
        Will only read the object IDs which creates an invalid class.
        Use assignPointers() to make the class useable. */
    void deserialize(IO::DataStream&);

    /** Assigns the object pointers for each connection according to the ids. */
    void assignPointers(Object * rootObject);
    /** Returns true if any of the connections has unassigned pointers */
    bool isUnassigned() const;

    /** Renames the ids (must be done before assignPointers()) */
    void idNamesChanged(const QMap<QString, QString> & map);

    void dump(std::ostream&) const;

    // ------------- stl container -------------------

    std::set<AudioObjectConnection*>::iterator begin() { return cons_.begin(); }
    std::set<AudioObjectConnection*>::iterator end() { return cons_.end(); }
    std::set<AudioObjectConnection*>::const_iterator begin() const { return cons_.cbegin(); }
    std::set<AudioObjectConnection*>::const_iterator end() const { return cons_.cend(); }

    // --------------- getter ------------------------

    bool isEmpty() const { return cons_.empty(); }

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

    /** Removes all objects and their edges recursively */
    void remove(Object * tree);

    /** Returns a new connection container containing only the connections
        that are in the given tree (from and to). */
    AudioObjectConnections reducedTo(const Object * tree) const;

    /** Returns a new connection container containing only the connections
        that are in the given trees (from and to). */
    AudioObjectConnections reducedTo(const QList<const Object*>& tree) const;

private:

    // disable copy
    //AudioObjectConnections(const AudioObjectConnections&);
    //AudioObjectConnections& operator = (const AudioObjectConnections&);

    std::multimap<AudioObject*, AudioObjectConnection*>
        toMap_, fromMap_;
    std::set<AudioObjectConnection*> cons_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_AUDIOOBJECTCONNECTIONS_H
