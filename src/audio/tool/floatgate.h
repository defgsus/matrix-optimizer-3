/** @file FloatGate.h

    @brief Simple float-stream to gate converter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_FLOATGATE_H
#define MOSRC_AUDIO_TOOL_FLOATGATE_H

namespace MO {
namespace AUDIO {

template <typename F>
class FloatGate
{
    public:

    FloatGate(F threshold = F(0))
        : thresh_       (threshold),
          last_value_   (0)
    { }

    /** Input a signal, gate it, and return state.
        The @p gate will only change the internal state, if it is
        A) larger than the threshold and the gate is not already on, or
        B) smaller than or equal to the threshold.
        In either case, the internal state will be set to the value in @p gate.
        The return value will be @p gate in case of A and threshold() otherwise. */
    F input(F gate)
    {
        if (gate > thresh_)
        {
            // if gate was off
            if (last_value_ <= thresh_)
            {
                // set gate on
                last_value_ = gate;
                // return gate on
                return last_value_;
            }
        }
        else
            // gate off
            last_value_ = gate;

        return thresh_;
    }

    /** Returns internal state */
    F state() const { return last_value_; }

    /** Returns set threshold */
    F threshold() const { return thresh_; }

    /** Sets the threshold of the gate.
        Causes no change to internal state. */
    void setThreshold(F threshold) { thresh_ = threshold; }

    private:

    F	thresh_,
        last_value_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_FLOATGATE_H
