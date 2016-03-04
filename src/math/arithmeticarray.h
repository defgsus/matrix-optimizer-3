/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/3/2016</p>
*/

#ifndef MOSRC_MATH_ARITHMETICARRAY_H
#define MOSRC_MATH_ARITHMETICARRAY_H

#include <cstddef>
#include <vector>

#if 1
#   include <cassert>
#   define MO_AA_ASSERT(x__) assert(x__)
#elif 1
#   include <iostream>
#   define MO_AA_ASSERT(x__) \
        if (!(x__)) { std::cout << __FILE__ << ":" << __LINE__ \
            << ": assertion " #x__ << std::endl; \
#else
#   define MO_AA_ASSERT(x__)
#endif

namespace MO {
namespace MATH {

/** N-Dimensional vector with arithmetic operators */
template <typename T>
class ArithmeticArray
{
public:
    ArithmeticArray(size_t dim, T fill = T(0))
        : p_dim_    (dim)
    {
        p_data_.resize(p_dim_, fill);
    }

    ArithmeticArray(const ArithmeticArray& other)
        : p_dim_    (other.p_dim_)
    {
        p_data_.resize(p_dim_, T(0));
        *this = other;
    }

    // ----------- getter -------------

    size_t numDimensions() const { return p_dim_; }

    // ----------- setter -------------

    void setDimensions(size_t d) { p_dim_ = d; p_data_.resize(d); fill(0); }

    void fill(T v) { for (auto& d : p_data_) d = v; }

    // ---------- operators -----------

    T  operator[] (size_t idx) const { MO_AA_ASSERT(idx < p_dim_); return p_data_[idx]; }
    T& operator[] (size_t idx) { MO_AA_ASSERT(idx < p_dim_); return p_data_[idx]; }

    ArithmeticArray& operator= (const ArithmeticArray& rhs)
    {
        p_dim_ = rhs.p_dim_;
        p_data_.resize(p_dim_);
        for (size_t i=0; i<p_dim_; ++i)
            p_data_[i] = rhs.p_data_[i];
        return *this;
    }

    // --- aa += aa ---

    ArithmeticArray& operator += (const ArithmeticArray& rhs)
    {
        MO_AA_ASSERT(p_dim_ == rhs.p_dim_);
        for (size_t i=0; i<p_dim_; ++i)
            p_data_[i] += rhs.p_data_[i];
        return *this;
    }

    ArithmeticArray& operator -= (const ArithmeticArray& rhs)
    {
        MO_AA_ASSERT(p_dim_ == rhs.p_dim_);
        for (size_t i=0; i<p_dim_; ++i)
            p_data_[i] -= rhs.p_data_[i];
        return *this;
    }

    ArithmeticArray& operator *= (const ArithmeticArray& rhs)
    {
        MO_AA_ASSERT(p_dim_ == rhs.p_dim_);
        for (size_t i=0; i<p_dim_; ++i)
            p_data_[i] *= rhs.p_data_[i];
        return *this;
    }

    ArithmeticArray& operator /= (const ArithmeticArray& rhs)
    {
        MO_AA_ASSERT(p_dim_ == rhs.p_dim_);
        for (size_t i=0; i<p_dim_; ++i)
            p_data_[i] /= rhs.p_data_[i];
        return *this;
    }

#if interpreters_would_be_that_clever_and_nice
    please do {the above} with {scalars}
    thank you
#endif

    // --- aa += scalar ---

    ArithmeticArray& operator += (const T rhs)
    {
        for (size_t i=0; i<p_dim_; ++i)
            p_data_[i] += rhs;
        return *this;
    }

    ArithmeticArray& operator -= (const T rhs)
    {
        for (size_t i=0; i<p_dim_; ++i)
            p_data_[i] -= rhs;
        return *this;
    }

    ArithmeticArray& operator *= (const T rhs)
    {
        for (size_t i=0; i<p_dim_; ++i)
            p_data_[i] *= rhs;
        return *this;
    }

    ArithmeticArray& operator /= (const T rhs)
    {
        for (size_t i=0; i<p_dim_; ++i)
            p_data_[i] /= rhs;
        return *this;
    }

    // --- unary ops ---

    const ArithmeticArray& operator+() const
    {
        return *this;
    }

    ArithmeticArray operator-() const
    {
        ArithmeticArray a(*this);
        for (size_t i=0; i<p_dim_; ++i)
            a.p_data_[i] = -a.p_data_[i];
        return a;
    }

    // --- aa + aa ---

    ArithmeticArray operator+ (const ArithmeticArray& rhs) const
    {
        MO_AA_ASSERT(p_dim_ == rhs.p_dim_);
        ArithmeticArray a(*this);
        for (size_t i=0; i<p_dim_; ++i)
            a.p_data_[i] += rhs.p_data_[i];
        return a;
    }

    ArithmeticArray operator- (const ArithmeticArray& rhs) const
    {
        MO_AA_ASSERT(p_dim_ == rhs.p_dim_);
        ArithmeticArray a(*this);
        for (size_t i=0; i<p_dim_; ++i)
            a.p_data_[i] -= rhs.p_data_[i];
        return a;
    }

    ArithmeticArray operator* (const ArithmeticArray& rhs) const
    {
        MO_AA_ASSERT(p_dim_ == rhs.p_dim_);
        ArithmeticArray a(*this);
        for (size_t i=0; i<p_dim_; ++i)
            a.p_data_[i] *= rhs.p_data_[i];
        return a;
    }

    ArithmeticArray operator/ (const ArithmeticArray& rhs) const
    {
        MO_AA_ASSERT(p_dim_ == rhs.p_dim_);
        ArithmeticArray a(*this);
        for (size_t i=0; i<p_dim_; ++i)
            a.p_data_[i] /= rhs.p_data_[i];
        return a;
    }

    // --- aa + scalar ---

    ArithmeticArray operator+ (const T rhs) const
    {
        ArithmeticArray a(*this);
        for (size_t i=0; i<p_dim_; ++i)
            a.p_data_[i] += rhs;
        return a;
    }

    ArithmeticArray operator- (const T rhs) const
    {
        ArithmeticArray a(*this);
        for (size_t i=0; i<p_dim_; ++i)
            a.p_data_[i] -= rhs;
        return a;
    }

    ArithmeticArray operator* (const T rhs) const
    {
        ArithmeticArray a(*this);
        for (size_t i=0; i<p_dim_; ++i)
            a.p_data_[i] *= rhs;
        return a;
    }

    ArithmeticArray operator/ (const T rhs) const
    {
        ArithmeticArray a(*this);
        for (size_t i=0; i<p_dim_; ++i)
            a.p_data_[i] /= rhs;
        return a;
    }

    // --- scalar + aa ---

    friend ArithmeticArray operator+ (const T lhs, const ArithmeticArray& rhs)
    {
        ArithmeticArray a(rhs.p_dim_);
        for (size_t i=0; i<a.p_dim_; ++i)
            a.p_data_[i] = lhs + a.p_data_[i];
        return a;
    }

    friend ArithmeticArray operator- (const T lhs, const ArithmeticArray& rhs)
    {
        ArithmeticArray a(rhs.p_dim_);
        for (size_t i=0; i<a.p_dim_; ++i)
            a.p_data_[i] = lhs - a.p_data_[i];
        return a;
    }

    friend ArithmeticArray operator* (const T lhs, const ArithmeticArray& rhs)
    {
        ArithmeticArray a(rhs.p_dim_);
        for (size_t i=0; i<a.p_dim_; ++i)
            a.p_data_[i] = lhs * a.p_data_[i];
        return a;
    }

    friend ArithmeticArray operator/ (const T lhs, const ArithmeticArray& rhs)
    {
        ArithmeticArray a(rhs.p_dim_);
        for (size_t i=0; i<a.p_dim_; ++i)
            a.p_data_[i] = lhs / a.p_data_[i];
        return a;
    }

private:
    size_t p_dim_;
    std::vector<T> p_data_;
};




} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_ARITHMETICARRAY_H

