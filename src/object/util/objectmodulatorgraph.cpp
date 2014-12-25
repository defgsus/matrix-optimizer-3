/** @file objectmodulatorgraph.cpp

    @brief Functions for managing modulations between objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.10.2014</p>
*/

#include "objectmodulatorgraph.h"

#include "object/object.h"
#include "object/param/parameter.h"
#include "object/param/parameters.h"
#include "object/param/modulator.h"

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
            graph.addEdge(mod->modulator(), object);
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
