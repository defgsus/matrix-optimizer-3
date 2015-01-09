/** @file refcounted.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.01.2015</p>
*/

#ifndef MOSRC_TYPES_REFCOUNTED_H
#define MOSRC_TYPES_REFCOUNTED_H

#include <atomic>

namespace MO {


class RefCounted
{
public:

    RefCounted() : p_refcount_(1) { }

    void addRef() { ++p_refcount_; }

    void releaseRef() { if (--p_refcount_ == 0) delete this; }

    int referenceCount() const { return p_refcount_; }

protected:
    virtual ~RefCounted() { }

private:

    std::atomic_int p_refcount_;
};



} // namespace MO


#endif // MOSRC_TYPES_REFCOUNTED_H
