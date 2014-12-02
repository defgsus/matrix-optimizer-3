/** @file audioengine.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#ifndef MOSRC_ENGINE_AUDIOENGINE_H
#define MOSRC_ENGINE_AUDIOENGINE_H

#include <QObject>

#include "types/float.h"
#include "types/int.h"

namespace MO {
namespace AUDIO { class Configuration; class AudioBuffer; }

class Scene;

class AudioEngine : public QObject
{
    Q_OBJECT
public:
    explicit AudioEngine(QObject *parent = 0);
    ~AudioEngine();

    // -------------- getter -------------

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

signals:

public slots:

    /** Creates the dsp path for the given scene.
        @p thread is the thread-index for which to query Parameters in the scene. */
    void setScene(Scene *, const AUDIO::Configuration&, uint thread);

    /** Seeks to a certain position in the scene */
    void seek(SamplePos pos);

    /** Processes one audio block and advances position.
        @p inputs and @p outputs are expected to point
        at config().numChannelsIn()/Out() times blocks of length config().bufferSize() */
    void process(const F32 * inputs, F32 * outputs);

private:

    class Private;
    Private * p_;

};


} // namespace MO

#endif // MOSRC_ENGINE_AUDIOENGINE_H
