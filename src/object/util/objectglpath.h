/** @file objectglpath.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.01.2015</p>
*/

#ifndef MOSRC_OBJECT_UTIL_OBJECTGLPATH_H
#define MOSRC_OBJECT_UTIL_OBJECTGLPATH_H

#include <QList>

#include "object/object_fwd.h"
#include "types/time.h"

namespace MO {
namespace GL { class Context; }

class ObjectGlPath
{
public:
    ObjectGlPath();
    ~ObjectGlPath();

    // ---------------- getter ----------------

    std::ostream& dump(std::ostream &) const;

    // --------------- creation ---------------

    /** Creates a fully new path.
        @note Any resources still bound will be discarded! */
    void createPath(Scene * scene, GL::Context*, uint thread);

    // ---------------- calc ------------------

    void calcTransformations(const RenderTime& time);

    // --------------- render -----------------

    bool isGlInitialized() const;

    void initGl();

    void render(Double time);

    void releaseGl();

    // ------------- audio io -----------------

    /** Audio input buffers, as requested by createPath() */
    const QList<AUDIO::AudioBuffer*> & audioInputs();

    /** Audio output buffers, as requested by createPath() */
    const QList<AUDIO::AudioBuffer*> & audioOutputs();

private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_OBJECTGLPATH_H
