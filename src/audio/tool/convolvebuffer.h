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

    /** Call this to prepare the complex kernel, once the blockSize
        if the AudioBuffer in process() is known.
        When not called, this will be executed in process() automatically. */
    void prepareComplexKernel(size_t blockSize);

    /** Process the input into output.
        Blocksize must match! */
    void process(const AudioBuffer * in, AudioBuffer * out);

private:

#if 0 // naive version (too slow)
    std::vector<F32>
            p_kernel_,
            p_kernel_fft_,
            p_scratch_;
    AUDIO::AudioBuffer * p_input_buf_;
    size_t p_pos_, p_cur_blockSize_;
    bool p_kernel_changed_;
    F32 p_amp_;

#else

    std::vector<F32>
            p_kernel_,
            p_scratch_,
            p_input_;
    std::vector<std::vector<F32>>
            p_kernel_fft_;
    AUDIO::AudioBuffer * p_out_buf_;
    size_t p_windowSize_, p_windows_, p_pos_, p_cur_blockSize_;
    bool p_kernel_changed_;
    F32 p_amp_;

#endif
};


} // namespace AUDIO
} // namespace DELAY

#endif // MOSRC_AUDIO_TOOL_CONVOLVEBUFFER_H
