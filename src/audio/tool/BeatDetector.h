/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/6/2016</p>
*/

#ifndef MOSRC_AUDIO_TOOL_BEATDETECTOR_H
#define MOSRC_AUDIO_TOOL_BEATDETECTOR_H

#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace AUDIO {

class BeatDetector
{
public:
    BeatDetector(size_t fftSize = 1024,
                 size_t numBins = 16,
                 size_t numHistory = 256,
                 F32 sampleRate = 44100.f);
    ~BeatDetector();

    // ---- getter -----

    F32 averageTime() const;
    F32 threshold() const;
    F32 sampleRate() const;
    F32 lowFrequency() const;
    F32 highFrequency() const;
    F32 convolutionAdaptSpeed() const;
    size_t fftSize() const;
    size_t numBins() const;
    size_t numHistory() const;
    size_t numConvolutions() const;
    size_t numFreqResponses() const;

    /** The length of the history in seconds */
    F32 historyLength() const;

    // --- setter ------

    void setSize(size_t fftSize, size_t numBins, size_t numHistory, F32 sampleRate);
    void setAverageTime(F32 seconds);
    void setThreshold(F32 t);
    void setBeatsBinary(bool);
    void setFrequencyRange(F32 low, F32 high);
    void setNumConvolutions(size_t num);
    void setConvolutionAdaptSpeed(F32);

    // --- processing --

    void push(const F32* data, size_t count);

    // --- read-back ---

    const F32* currentLevel() const;
    const F32* averageLevel() const;
    const F32* beat() const;
    const F32* beatHistory(size_t bin) const;
    /** numHistory() samples of convoluted history per bin */
    const F32* convolution(size_t bin) const;
    /** numFreqResponses() samples of fourier transform of the
        convoluted history per bin */
    const F32* freqResponse(size_t bin) const;
    /** numHistory() samples of candidate points per bin */
    const F32* candidates(size_t bin) const;

    /** The number of similiar messures among all bins */
    size_t matchCount(size_t bin) const;

    /** The distance in history-samples to the beginning of the next period */
    size_t lengthBuffers(size_t bin) const;
    /** The determined frequency per bin in Hz */
    F32 beatsPerSecond(size_t bin) const;
    /** The determined length of a period in seconds */
    F32 lengthSeconds(size_t bin) const;
    /** The determined length of a priod normalized to [0,1) */
    F32 lengthNormalized(size_t bin) const;

    /** The distance in history-samples to the beginning of the next period */
    size_t sortedLengthBuffers(size_t bin) const;
    /** The determined frequency per bin in Hz */
    F32 sortedBeatsPerSecond(size_t bin) const;
    /** The determined length of a period in seconds */
    F32 sortedLengthSeconds(size_t bin) const;
    /** The determined length of a priod normalized to [0,1) */
    F32 sortedLengthNormalized(size_t bin) const;

private:
    struct Private;
    Private* p_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_BEATDETECTOR_H
