/** @file objectmodulatorgraph.cpp

    @brief Functions for managing modulations between objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.10.2014</p>
*/

#include "ObjectModulatorGraph.h"

#include "object/Object.h"
#include "object/param/Parameter.h"
#include "object/param/Parameters.h"
#include "object/param/Modulator.h"

namespace MO {




void get_object_modulator_graph(ObjectGraph &graph, Object * object)
{
    // for all parameters
    for (Parameter * param : object->params()->parameters())
    {
        // modulators of each parameter
        for (Modulator * mod : param->modulators())
        {
            // add incoming edge from modulator object to this object
            if (mod->modulator())
                graph.addEdge(mod->modulator(), object);
            else
                MO_WARNING("unassigned modulator " << mod->nameAutomatic() <<
                           " detected in get_object_modulator_graph()");
        }
    }

    // -- special cases --

    // clip as part of clipcontainer
    /*if (object->isClip())
    {
        if (Object * con = object->findParentObject(Object::T_CLIP_CONTAINER))
            graph.addEdge(object, con);
    }
    else*/

    // sequences
    if (object->isSequence())
    {
        // sequence as part of clip
        /*if (Object * clip = object->findParentObject(Object::T_CLIP))
            graph.addEdge(object, clip);
        else*/

        // sequence as part of track
        if (Object * track = object->findParentObject(Object::TG_TRACK))
            graph.addEdge(object, track);
    }

    // for all children
    for (auto c : object->childObjects())
        get_object_modulator_graph(graph, c);
}


} // namespace MO
