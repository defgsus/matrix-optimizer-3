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

    /** Completely creates the whole dsp path, with all buffers */
    void createPath(Scene * scene, const AUDIO::Configuration& conf, uint thread);

    /** Final creation of dsp stuff
        - to be called from the thread that will call calcAudio() */
    void preparePath();

    // ---------------- calc ------------------

    void calcTransformations(SamplePos pos);

    void calcAudio(SamplePos pos);

    /** Notify all objects that the current audio thread is going away */
    void sendCloseThread();

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

#endif // MOSRC_OBJECT_AUDIO_OBJECTDSPPATH_H
