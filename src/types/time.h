/** @file types/time.h

    @brief A precise sample/float-second unit at the bottom

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_TYPES_TIME_H
#define MOSRC_TYPES_TIME_H


#include "float.h"
#include "int.h"

namespace MO
{

    struct RenderTime
    {
        // ######### getter #########

        // --- seconds ---

        /** Current second */
        Double second() const { return p_second_; }
        /** Time between last and this frame. */
        Double delta() const { return p_delta_; }

        /** Returns true when delta() > 0. */
        bool hasDelta() const { return p_delta_ > 0.; }

        // --- samples ---

        /** Current sample position */
        SamplePos sample() const { return p_sample_; }
        /** Sampling rate in Hertz */
        uint sampleRate() const { return p_sampleRate_; }
        /** The inverse of the sampling rate: 1. / sampleRate() */
        Double sampleRateInv() const { return p_sampleRateInv_; }
        /** DSP block size */
        uint bufferSize() const { return p_bufferSize_; }

        /** Returns true if sampleRate() is set */
        bool hasSampleRate() const { return p_sampleRate_ > 0; }
        /** Returns true if bufferSize() is set */
        bool hasBufferSize() const { return p_bufferSize_ > 0; }

        // --- thread ---

        /** The thread number of the request */
        uint thread() const { return p_thread_; }

        // ######### setter #########

        void setSecond(Double second) { p_second_ = second; }
        void setDelta(Double delta) { p_delta_ = delta; }
        void setSample(SamplePos pos) { p_sample_ = pos; }
        void setSampleRate(uint sr) { p_sampleRate_ = sr; p_sampleRateInv_ = sr > 0 ? 1. / sr : 0.; }
        void setBufferSize(uint bs) { p_bufferSize_ = bs; }
        void setThread(uint t) { p_thread_ = t; }

        // ####### arithmetic #######

        RenderTime& operator += (Double sec)
            { p_second_ += sec; p_sample_ += sec * sampleRate(); return *this; }
        RenderTime& operator -= (Double sec)
            { p_second_ -= sec; p_sample_ -= sec * sampleRate(); return *this; }
        RenderTime operator + (Double sec) const
            { RenderTime t(*this); t += sec; return t; }
        RenderTime operator - (Double sec) const
            { RenderTime t(*this); t -= sec; return t; }

        RenderTime& operator += (SamplePos sam)
            { p_second_ += sam * sampleRateInv(); p_sample_ += sam; return *this; }
        RenderTime& operator -= (SamplePos sam)
            { p_second_ -= sam * sampleRateInv(); p_sample_ -= sam; return *this; }
        RenderTime operator + (SamplePos sam) const
            { RenderTime t(*this); t += sam; return t; }
        RenderTime operator - (SamplePos sam) const
            { RenderTime t(*this); t -= sam; return t; }

        // ########## CTOR ##########

        explicit RenderTime()
            : RenderTime(0., 0., 0, 0, 0, 0)
        { }

        explicit RenderTime(Double second, uint thread)
            : RenderTime(second, 0., 0, 0, 0, thread)
        { }

        explicit RenderTime(Double second, Double delta, uint thread)
            : RenderTime(second, delta, 0, 0, 0, thread)
        { }

        explicit RenderTime(SamplePos sample, uint sampleRate, uint bufferSize, uint thread)
            : RenderTime(0., 0., sample, sampleRate, bufferSize, thread)
        { }

        explicit RenderTime(Double second, Double delta,
                            SamplePos sample, uint sampleRate, uint bufferSize, uint thread)
            : p_second_(second), p_delta_(delta)
            , p_sampleRateInv_(sampleRate > 0 ? 1. / sampleRate : 0.)
            , p_sample_(sample), p_sampleRate_(sampleRate), p_bufferSize_(bufferSize)
            , p_thread_(thread)
        { }

    private:
        Double p_second_, p_delta_, p_sampleRateInv_;
        SamplePos p_sample_;
        uint p_sampleRate_, p_bufferSize_, p_thread_;
    };


} // namespace MO


#endif // MOSRC_TYPES_TIME_H
