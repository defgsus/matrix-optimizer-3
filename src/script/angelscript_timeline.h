/** @file angelscript_timeline.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12/29/2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_SCRIPT_ANGELSCRIPT_TIMELINE_H
#define MOSRC_SCRIPT_ANGELSCRIPT_TIMELINE_H

class asIScriptEngine;

namespace MO {
namespace MATH { class Timeline1d; }

// Ref-counted wrapper of MATH::Timeline1D for AngelScript
class Timeline1AS;

// Ref-counted wrapper of multiple MATH::Timeline1Ds for AngelScript (Timeline2 - Timeline4) */
template <class Vec, unsigned int NUM>
class TimelineXAS;

/** Registers the timeline namespace.
    Dependency: string, vector */
void registerAngelScript_timeline(asIScriptEngine *engine);

/** Wraps the timeline into the AngelScript representation.
    Ownership is on caller.
    Timeline1D must stay valid during the lifetime of Timeline1AS, of course.
*/
Timeline1AS * timeline_to_angelscript(MATH::Timeline1d * );

/** Wraps the timeline into the AngelScript representation.
    Ownership is on caller.
    The timeline data will be copied to the internal angelscript timeline. */
Timeline1AS * timeline_to_angelscript(const MATH::Timeline1d& );

/** Returns the associated timeline for the angelscript object */
const MATH::Timeline1d& get_timeline(const Timeline1AS*);

} // namespace MO


#endif // MOSRC_SCRIPT_ANGELSCRIPT_TIMELINE_H

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
