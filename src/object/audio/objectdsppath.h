/** @file objectdsppath.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_OBJECTDSPPATH_H
#define MOSRC_OBJECT_AUDIO_OBJECTDSPPATH_H

#include <iostream>

#include "object/object_fwd.h"
#include "types/int.h"
#include "types/float.h"

namespace MO {


class ObjectDspPath
{
public:
    ObjectDspPath();
    ~ObjectDspPath();

    // ---------------- getter ----------------

    uint sampleRate() const;
    uint bufferSize() const;

    std::ostream& dump(std::ostream &) const;

    // --------------- creation ---------------

    void createPath(Scene * scene, uint sampleRate, uint bufferSize);

    // ---------------- calc ------------------

    void calcTransformations(SamplePos pos, uint thread);

private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_OBJECTDSPPATH_H
