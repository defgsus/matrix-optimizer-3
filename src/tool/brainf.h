/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/19/2016</p>
*/

/** @file brainf.h

    @brief A templated brainfuck interpreter

    @version started 3/21/2015

    <pre>
    The MIT License (MIT)

    Copyright (c) 2015, stefan.berke@modular-audio-graphics.com

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
    </pre>
*/

#include <cstddef>
#include <vector>
#include <string>

#ifndef MOSRC_TOOL_BRAINF_H_INCLUDED
#define MOSRC_TOOL_BRAINF_H_INCLUDED

namespace MO {


/** Opcodes of the brainfuck language */
enum BrainfOpcode
{
    BFO_NOP, ///< Not an actual opcode and not used either
    BFO_LEFT,
    BFO_RIGHT,
    BFO_INC,
    BFO_DEC,
    BFO_IN,
    BFO_OUT,
    BFO_BEGIN,
    BFO_END
};

/** Binary flags used to customize the brainfuck interpreter */
enum BrainfFlags
{
    BFF_EXPAND_LEFT =  1,
    BFF_EXPAND_RIGHT = 1<<1,
    BFF_INPUT_LOOP = 1<<2
};

/** A traits class to convert the internal type of the Brainf class
    to/from char and to signed integer.
    Generally there should be no need to specialize it, unless
    you want to use >64 bit or map characters differently. */
template <typename T>
struct Brainf_traits
{
    static char toChar(T v) { return v; }
    static T fromChar(char c) { return c; }
    static int64_t toNumber(T v) { return v; }
    static BrainfOpcode toOpcode(T v) { return (BrainfOpcode)std::max(0,std::min(8, int(v))); }
};




/** A brainfuck interpreter.
    The type @p T represents the type for cells (tape, input and output).

    Ascii programs get translated to BrainfOpcode before execution.
    The interpreter currently does not use function pointers but a plain switch statement.
    Also, the matching loop brackets are resolved on each encounter,
    this could be much speed-up with a look-up table.
*/
template <typename T>
class Brainf
{
public:

    /** Signed index type */
    typedef std::ptrdiff_t Index;

    /** Constructs a brainfuck engine.
        @p tapeLength is the initial positive length of the internal tape.
        @p tapeLengthNeg is the initial negative length of the internal tape.
        @p flags is an or-combination of BrainfFlags */
    explicit Brainf(Index tapeLength = 16,
                    Index tapeLengthNeg = 16,
                    int flags = BFF_EXPAND_RIGHT)
        : p_flags_(flags)
    { clear(tapeLength, tapeLengthNeg); }

    // ---------------- getter -------------------

    /** Returns the set flags (or-combination of BrainfFlags) */
    int flags() const { return p_flags_; }

    /** Returns read access to the code (in BrainfOpcode format) */
    const std::vector<BrainfOpcode>& code() const { return p_code_; }

    /** Returns read access to the input to the program */
    const std::vector<T>& input() const { return p_in_; }

    /** Returns read access to the output of the program */
    const std::vector<T>& output() const { return p_out_; }

    /** Returns read access to the tape */
    const std::vector<T>& tape() const { return p_tape_; }

    /** Returns the current position of the program counter. */
    Index programPosition() const { return p_code_p_; }

    /** Returns the current position on the tape.
        This might be negativ as well. */
    Index tapePosition() const { return p_tape_p_; }

    /** Returns the current position in the input */
    Index inputPosition() const { return p_in_p_; }

    // ------------ string conversion ------------

    /** Returns the current program as ascii representation (<>+-.,[]).
        (If there are unknown opcodes in the code, they will be represented as
        '/x', where x stands for the opcode number.) */
    std::string codeString() const;

    /** Returns the input as std::string.
        If @p ignore_control is true, only characters 32-127 are included. */
    std::string inputString(bool ignore_control = false) const
        { return toString(p_in_, ignore_control); }

    /** Returns the input as std::string with a number of each entry. */
    std::string inputStringNum() const { return toStringNum(p_in_); }

    /** Returns the output after run() as std::string.
        If @p ignore_control is true, only characters 32-127 are included. */
    std::string outputString(bool ignore_control = false) const
        { return toString(p_out_, ignore_control); }

    /** Returns the output after run() as std::string with a number of each entry. */
    std::string outputStringNum() const { return toStringNum(p_out_); }

    /** Returns the tape as std::string.
        If @p ignore_control is true, only characters 32-127 are included. */
    std::string tapeString(bool ignore_control = false) const
        { return toString(p_tape_, ignore_control); }

    /** Returns the tape as std::string with a number of each entry. */
    std::string tapeStringNum() const { return toStringNum(p_tape_); }

    // ---------------- setter -------------------

    /** Clear everything (code, input, output, tape, pointers).
        @p tapeLength is the initial positive length of the internal tape.
        @p tapeLengthNeg is the initial negative length of the internal tape. */
    void clear(Index tapeLength = 16, Index tapeLengthNeg = 16);

    /** Sets the interpreter flags (or-combination of BrainfFlags) */
    void setFlags(int flags) { p_flags_ = flags; }

    /** Sets one interpreter flag */
    void setFlag(BrainfFlags option, bool enable) { p_flags_ =
                enable ? p_flags_ | (int)option
                       : p_flags_ & (~(int)option); }

    /** Sets the code from the ascii representation (<>+-.,[]).
        Resets the program counter. */
    void setCode(const std::string& s);

    /** Sets the code directly from the opcodes given in @p code.
        Resets the program counter. */
    void setCode(const std::vector<BrainfOpcode>& code) { p_code_ = code; p_code_p_ = 0; }

    /** Sets the input to use on next run().
        The input position is reset to 0. */
    void setInput(const std::string& input) { p_in_ = fromString(input); p_in_p_ = 0; }

    /** Sets the input to use on next run().
        The input position is reset to 0. */
    void setInput(const std::vector<T>& input) { p_in_ = input; p_in_p_ = 0; }

    /** Sets the contents of the tape.
        The tape position is reset to 0 and the tape will start at 0
        (e.g. no negative expansion) */
    void setTape(const std::vector<T>& tape) { p_tape_ = tape; p_tape_p_ = p_tape_0_ = 0; }

    // -------------- execute --------------------

    /** Runs the program.
        The execution stops, when the program counter moves past the last
        opcode, or when @p max_steps != 0 and the number of processed opcodes
        equals @p max_steps. */
    void run(size_t max_steps = 0);

    // ---------- processing/opcodes -------------

    /** Returns a reference of the given tape entry.
        Expands the tape memory if necessary and flags allow it. */
    T& tapeAt(Index i);

    // all opcodes of brainfuck as member functions

    void o_left() { --p_tape_p_; }
    void o_right() { ++p_tape_p_; }

    void o_inc() { ++tapeAt(p_tape_p_); }
    void o_dec() { --tapeAt(p_tape_p_); }

    void o_in() { tapeAt(p_tape_p_) = readInFwd(); }
    void o_out() { p_out_.push_back(tapeAt(p_tape_p_)); }

    void o_begin();
    void o_end();

    // pseudo opcode
    T readInFwd() { return p_in_p_ < Index(p_in_.size())
                        ? p_in_[p_in_p_++]
                        : (flags() & BFF_INPUT_LOOP) ? p_in_[p_in_p_ = 0] : 0; }

    // ------------- helper ----------------------

    /** Convertes the values in @p v to a string with a character for each entry.
        If @p ignore_control is true, only characters 32-127 are included. */
    static std::string toString(const std::vector<T>& v, bool ignore_control = false);

    /** Convertes the characters in @p str to a vector of internal values. */
    static std::vector<T> fromString(const std::string& str);

    /** Convertes the values in @p v to a string with comma separated integers
        for each entry. */
    static std::string toStringNum(const std::vector<T>& v);

private:

    std::vector<BrainfOpcode> p_code_;
    std::vector<T> p_tape_, p_in_, p_out_;
    Index p_code_p_, p_in_p_, p_tape_p_, p_tape_0_;
    int p_flags_;
};


// some default typedefs

typedef Brainf<uint8_t> Brainf_uint8;
typedef Brainf<int8_t> Brainf_int8;
typedef Brainf<uint16_t> Brainf_uint16;
typedef Brainf<int16_t> Brainf_int16;







// ############################## impl #################################


template <typename T>
void Brainf<T>::clear(Index tapeLength, Index tapeLengthNeg)
{
    p_code_.clear();
    p_in_.clear();
    p_out_.clear();
    p_tape_.clear();
    p_code_p_ =
    p_tape_p_ =
    p_in_p_ = 0;
    p_tape_0_ = tapeLengthNeg; // initial negative space
    p_tape_.resize(tapeLengthNeg + tapeLength);
}

template <typename T>
void Brainf<T>::setCode(const std::string &s)
{
    p_code_.clear();
    p_code_p_ = 0;

    for (auto & c : s)
    {
        BrainfOpcode op;
        switch (c)
        {
            // skip anything else
            default: continue;
            case '<': op = BFO_LEFT; break;
            case '>': op = BFO_RIGHT; break;
            case '+': op = BFO_INC; break;
            case '-': op = BFO_DEC; break;
            case ',': op = BFO_IN; break;
            case '.': op = BFO_OUT; break;
            case '[': op = BFO_BEGIN; break;
            case ']': op = BFO_END; break;
        }
        p_code_.push_back( op );
    }
}

template <typename T>
std::string Brainf<T>::codeString() const
{
    std::string s;
    for (auto & op : p_code_)
    {
        switch (op)
        {
            default: s += "/" + std::to_string(op); break;
            case BFO_LEFT: s += '<'; break;
            case BFO_RIGHT: s += '>'; break;
            case BFO_INC: s += '+'; break;
            case BFO_DEC: s += '-'; break;
            case BFO_IN: s += ','; break;
            case BFO_OUT: s += '.'; break;
            case BFO_BEGIN: s += '['; break;
            case BFO_END: s += ']'; break;
        }
    }

    return s;
}

template <typename T>
std::string Brainf<T>::toString(const std::vector<T>& v, bool ignore_control)
{
    std::string s;
    for (auto & o : v)
    {
        auto c = Brainf_traits<T>::toChar(o);
        if (!ignore_control
            || (c >= 32 && c <= 127))
            s += c;
    }
    return s;
}

template <typename T>
std::vector<T> Brainf<T>::fromString(const std::string& str)
{
    std::vector<T> v;
    for (auto & c : str)
        v.push_back( Brainf_traits<T>::fromChar(c) );
    return v;
}

template <typename T>
std::string Brainf<T>::toStringNum(const std::vector<T>& v)
{
    std::string s;
    for (size_t i = 0; i < v.size(); ++i)
    {
        if (i > 0)
            s += ", ";
        s += std::to_string(v[i]);
    }
    return s;
}


template <typename T>
void Brainf<T>::run(size_t max_steps)
{
    size_t steps = 0;
    // run to end of program or max_steps
    while (p_code_p_ < (Index)p_code_.size()
           && (max_steps == 0 || steps++ < max_steps))
    {
        const BrainfOpcode op = p_code_[p_code_p_];

        // this part is easy...
        switch (op)
        {
            default: break;

            case BFO_LEFT:  o_left(); break;
            case BFO_RIGHT: o_right(); break;
            case BFO_INC:   o_inc(); break;
            case BFO_DEC:   o_dec(); break;
            case BFO_IN:    o_in(); break;
            case BFO_OUT:   o_out(); break;
            case BFO_BEGIN: o_begin(); break;
            case BFO_END:   o_end(); break;
        }

        ++p_code_p_;
    }
}


/** @todo expansion not completely tested */
template <typename T>
T& Brainf<T>::tapeAt(Index i)
{
    i += p_tape_0_;
    // expand right?
    if (i >= (Index)p_tape_.size())
    {
        if (p_flags_ & BFF_EXPAND_RIGHT)
            p_tape_.resize(i + 16);
        else
            i = i % p_tape_.size();
    }
    // expand left?
    else if (i < 0)
    {
        if (p_flags_ & BFF_EXPAND_LEFT)
        {
            Index si = p_tape_.size(), grow = -i + 16;
            p_tape_.resize(si + grow);
            // move right
            for (Index j=si-1; j>=grow; --j)
                p_tape_[j+grow] = p_tape_[j];
            // clear new left
            for (Index j=0; j<grow; ++j)
                p_tape_[j] = T(0);
            // adjust pointers
            p_tape_0_ += grow;
            i += grow;
        }
        else
            i = p_tape_.size() - 1 - ((-i) % p_tape_.size());
    }

    return p_tape_[i];
}


template <typename T>
void Brainf<T>::o_begin()
{
    // break if zero
    if (!tapeAt(p_tape_p_))
    {
        // move to end bracket
        Index i = p_code_p_,
              lvl = 1;
        while (i < (Index)p_code_.size())
        {
            ++i;
            if (p_code_[i] == BFO_BEGIN)
                ++lvl;
            else
            if (p_code_[i] == BFO_END && --lvl == 0)
            {
                p_code_p_ = i;
                return;
            }
        }
    }
}


template <typename T>
void Brainf<T>::o_end()
{
    // jump back if not zero
    if (tapeAt(p_tape_p_))
    {
        // move to start bracket
        Index i = p_code_p_,
              lvl = 1;
        while (i >= 0)
        {
            --i;
            if (p_code_[i] == BFO_END)
                ++lvl;
            else
            if (p_code_[i] == BFO_BEGIN && (--lvl == 0))
            {
                p_code_p_ = i;
                return;
            }
        }
    }
}

} // namespace MO

#endif // MOSRC_TOOL_BRAINF_H_INCLUDED

