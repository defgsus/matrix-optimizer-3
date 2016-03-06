/** @file refcounted.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.01.2015</p>
*/

#ifndef MOSRC_TYPES_REFCOUNTED_H
#define MOSRC_TYPES_REFCOUNTED_H

#include <QString>

namespace MO {


/** Basic strong reference counting type */
class RefCounted
{
public:

    RefCounted();

    void addRef(const QString& reason);

    void releaseRef(const QString& reason);

    int refCount() const;

protected:
    virtual ~RefCounted();

private:
    struct P_RefCountedPrivate;
    P_RefCountedPrivate* p_refcount_private_;
};


/** Deleter for smart-pointer of RefCounted types */
struct RefCountedDeleter
{
    RefCountedDeleter(const QString& reason) : reason_(reason) { }
    void operator()(RefCounted * r) { r->releaseRef("AUTODELETE " + reason_); }
    QString reason_;
};


} // namespace MO


#endif // MOSRC_TYPES_REFCOUNTED_H
