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

    /** Initialize with a name. @p className is only used for debugging. */
    RefCounted(const QString& className);

    /** Adds a reference */
    void addRef(const QString& reason);

    /** Releases a reference */
    void releaseRef(const QString& reason);

    /** Number of references on this object */
    int refCount() const;

    // ------- debugging --------

    /** Returns the name of this refcounted object */
    const  QString refClassName() const;
    static QString refClassName(const RefCounted*);
    /** Returns the name and pointer of this refcounted object as string */
           QString refInstanceName() const;
    static QString refInstanceName(const RefCounted*);

    /** Overwrites the name given in the RefCounted constructor */
    void setRefClassName(const QString&);

protected:
    /** Non-public destructor.
        Make sure that destructors in derived classes are also protected! */
    virtual ~RefCounted();

private:
    struct P_RefCountedPrivate;
    P_RefCountedPrivate* p_refcount_private_;
};



/** Takes a RefCounted object at creation and
    releases the ref on destruction. */
class ScopedRefCounted
{
    RefCounted* o_;
    QString reason_;
public:
    ScopedRefCounted(RefCounted*o, const QString& reason)
        : o_(o), reason_(reason) { }
    ~ScopedRefCounted() { o_->releaseRef("AUTOREL " + reason_); }
};


/** Deleter for smart-pointer of RefCounted types */
struct RefCountedDeleter
{
    RefCountedDeleter(const QString& reason) : reason_(reason) { }
    void operator()(RefCounted * r) { r->releaseRef("AUTOREL " + reason_); }
    QString reason_;
};


} // namespace MO


#endif // MOSRC_TYPES_REFCOUNTED_H
