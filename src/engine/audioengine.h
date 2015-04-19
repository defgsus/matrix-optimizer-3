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

    /** Read access to the envelope values for each output */
    const F32 * outputEnvelope() const;

signals:

public slots:

    /** Creates the dsp path for the given scene.
        @p thread is the thread-index for which to query Parameters in the scene. */
    void setScene(Scene *, const AUDIO::Configuration&, uint thread);

    /** Currently used for creating the udp objects for clients */
    void prepareUdp();

    /** Seeks to a certain position in the scene */
    void seek(SamplePos pos);

    /** Processes one audio block and advances position.
        @p inputs and @p outputs are expected to point
        at config().numChannels[In()|Out()] times length config().bufferSize() floats. */
    void process(const F32 * inputs, F32 * outputs);

    /** Processes one audio block and advances position.
        @p inputs and @p outputs are expected to point
        at config().numChannels[In()|Out()] times length config().bufferSize() floats.
        @p outputs order is ch0/s0,ch1/s0,ch2/s0,ch3/s0,...ch0/s1,ch1/s1,ch2/s1,ch3/s1...*/
    void processForDevice(const F32 * inputs, F32 * outputs);

private:

    class Private;
    Private * p_;

};


} // namespace MO

#endif // MOSRC_ENGINE_AUDIOENGINE_H
