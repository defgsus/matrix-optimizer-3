/** @file objectdsppath.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_OBJECTDSPPATH_H
#define MOSRC_OBJECT_AUDIO_OBJECTDSPPATH_H

#include <iostream>

#include <QList>

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

    const AUDIO::Configuration & config() const;

    std::ostream& dump(std::ostream &) const;

    // --------------- creation ---------------

    void createPath(Scene * scene, const AUDIO::Configuration& conf);

    // ---------------- calc ------------------

    void calcTransformations(SamplePos pos, uint thread);

    void calcAudio(SamplePos pos, uint thread);

    // ------------- audio io -----------------

    /** The system audio output buffers. */
    const QList<AUDIO::AudioBuffer*> & audioOutputs();

private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_OBJECTDSPPATH_H
