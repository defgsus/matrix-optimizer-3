/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/27/2016</p>
*/

#ifndef MOSRC_OBJECT_UTIL_SCENESIGNALS_H
#define MOSRC_OBJECT_UTIL_SCENESIGNALS_H

#include <QObject>

#include "types/float.h"
#include "types/int.h"

namespace MO {

class Parameter;
class Camera;

/** Class for sending signals from MO::Scene */
class SceneSignals : public QObject
{
    Q_OBJECT
public:
    explicit SceneSignals(QObject *parent = 0)
        : QObject(parent)
    { }

signals:

    /** Scene should be re-rendered */
    void renderRequest();

    void playbackStarted();
    void playbackStopped();

    /** Emitted when the number of channels is set/changed */
    void numberChannelsChanged(uint numIn, uint numOut);

    /** Emitted when the number of (currently) microphones changed */
    void numberOutputEnvelopesChanged(uint num);

    /** This is send regularily during playback, representing the microphone levels */
    void outputEnvelopeChanged(const F32 * levels);

    /** Emitted whenever the scene time changed */
    void sceneTimeChanged(Double);

    /** openGL resources have been released for the given thread. */
    void glReleased(uint thread);

    /** Send when a parameter changed it's visibility */
    void parameterVisibilityChanged(MO::Parameter*);

    void CameraFboChanged(Camera *);
    void sceneFboChanged();

};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_SCENESIGNALS_H
