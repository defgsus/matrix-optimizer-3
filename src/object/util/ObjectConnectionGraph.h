/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/9/2016</p>
*/

#ifndef MOSRC_OBJECT_UTIL_OBJECTCONNECTIONGRAPH_H
#define MOSRC_OBJECT_UTIL_OBJECTCONNECTIONGRAPH_H

#include <QList>

#include "object/Object_fwd.h"

namespace MO {

/** Graph to collect and analyze object->parameter->object connections */
class ObjectConnectionGraph
{
public:
    ObjectConnectionGraph();
    ~ObjectConnectionGraph();

    // ---------- getter ----------

    /** Returns true if the object is known to the graph */
    bool hasObject(Object* obj);
    /** Returns true if there is a connection from @p modulator to @p goal */
    bool hasConnection(Object* modulator, Object* goal);

    /** Return a list of all objects in input dependency order. */
    QList<Object*> makeLinear() const;

    /* Return a list of all objects that feed into @p goal
    QList<Object*> getModulatingObjects(Object * goal, bool recursive) const; */

    // ---------- setter ----------

    void clear();

    //void addConnection(Parameter* modulator, Object* goal);
    void addConnection(Object* modulator, Object* goal);

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_OBJECTCONNECTIONGRAPH_H
