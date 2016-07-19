/** @file liveaudioengine.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#ifndef MOSRC_ENGINE_LIVEAUDIOENGINE_H
#define MOSRC_ENGINE_LIVEAUDIOENGINE_H

#include <QObject>

#include "types/float.h"
#include "types/int.h"

namespace MO {
namespace AUDIO { class Configuration; class AudioBuffer; }

class Scene;

class LiveAudioEngine : public QObject
{
    Q_OBJECT

    friend class AudioEngineOutThread;

public:
    explicit LiveAudioEngine(QObject *parent = 0);
    ~LiveAudioEngine();

    /** Assigned scene */
    Scene * scene() const;

    /** Assigned thread index */
    uint thread() const;

    /** Assigned audio configuration */
    const AUDIO::Configuration& config() const;

    /** Returns current sample position */
    SamplePos pos() const;

    /** Returns current second */
    Double second() const;

    /** Read access to the envelope values for each output */
    const F32 * outputEnvelope() const;

    bool isAudioConfigured() const;
    bool isAudioInitialized() const;
    bool isPlayback() const;
    bool isPause() const;

signals:

public slots:

    /** Creates the dsp path for the given scene.
        @p thread is the thread-index for which to query Parameters in the scene. */
    void setScene(Scene *, uint thread);

    /** Seeks to a certain position in the scene */
    void seek(SamplePos pos);
    /** Seeks to a certain position in the scene */
    void seek(Double second);

    /** Opens the audio streams as configured.
        Returns true when the device is ready. */
    bool initAudioDevice();

    void closeAudioDevice();

    /** Starts the audio playback.
        Initializes the audio device if necessary.
        Displays error messages to user.
        Returns true when device is playing. */
    bool start();

    /** Stops the audio thread and closes the device */
    void stop();

    /** Puts the audio thread on idle */
    void pause(bool enable);

private:

    class Private;
    Private * p_;

};


} // namespace MO

#endif // MOSRC_ENGINE_LIVEAUDIOENGINE_H
