/** @file convolvebuffer.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.05.2015</p>
*/

#ifndef MOSRC_AUDIO_TOOL_CONVOLVEBUFFER_H
#define MOSRC_AUDIO_TOOL_CONVOLVEBUFFER_H

#include <vector>

#include "types/float.h"

namespace MO {
namespace AUDIO {

class AudioBuffer;

/** Buffer that convolves it's input in chunks */
class ConvolveBuffer
{
public:
    ConvolveBuffer();
    ~ConvolveBuffer();

    void setKernel(const F32 * data, size_t num);

    /** Process the input into output.
        Blocksize must match! */
    void process(const AudioBuffer * in, AudioBuffer * out);

    /** Call this to prepare the complex kernel, once the blockSize
        of the AudioBuffer in process() is known.
        When not called, this will be executed in process() automatically. */
    void prepareComplexKernel(size_t blockSize);

private:
    struct Private;
    Private* p_;
};


} // namespace AUDIO
} // namespace DELAY

#endif // MOSRC_AUDIO_TOOL_CONVOLVEBUFFER_H
