/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/5/2016</p>
*/

#include <atomic>

#include "refcounted.h"
#include "object/object.h"
#include "io/log.h"

#if 0
#   define MO_REF_DEBUG(arg__) MO_PRINT(arg__)
#else
#   define MO_REF_DEBUG(unused__) { }
#endif

namespace MO {

namespace
{
    QString typeNameUnsave(const RefCounted* ref)
    {
        //if (auto o = dynamic_cast<const Object*>(ref))
        //    return QString("Object('%1')").arg(o->idName());
        return QString("0x%1")
                .arg(uint64_t(ref), 16, 16, QChar('0'));
    }

    QString typeName(const RefCounted* ref)
    {
        if (auto o = dynamic_cast<const Object*>(ref))
            return QString("Object('%1')").arg(o->idName());
        return QString("0x%1")
                .arg(uint64_t(ref), 16, 16, QChar('0'));
    }
}

struct RefCounted::P_RefCountedPrivate
{
    P_RefCountedPrivate()
        : refcount(1)
    { }

    std::atomic_int refcount;
    QString destroyReason;
};

RefCounted::RefCounted()
    : p_refcount_private_(new P_RefCountedPrivate)
{
    MO_REF_DEBUG("REF CREATE " << typeName(this));
}

RefCounted::~RefCounted()
{
    MO_REF_DEBUG("REF DESTROY " << typeNameUnsave(this) << " '"
                 << p_refcount_private_->destroyReason << "'");
    delete p_refcount_private_;
}

void RefCounted::addRef(const QString& reason)
{
    Q_UNUSED(reason);
    MO_REF_DEBUG("REF ADD " << typeName(this) << " '" << reason << "' NOW "
             << (p_refcount_private_->refcount+1));
    ++p_refcount_private_->refcount;
}

void RefCounted::releaseRef(const QString& reason)
{
    Q_UNUSED(reason);
    MO_REF_DEBUG("REF REL " << typeName(this) << " '" << reason << "' NOW "
             << (p_refcount_private_->refcount-1));
    if (--p_refcount_private_->refcount == 0)
    {
        p_refcount_private_->destroyReason = reason;
        delete this;
    }
}

int RefCounted::refCount() const
{
    return p_refcount_private_->refcount;
}



} // namespace MO


