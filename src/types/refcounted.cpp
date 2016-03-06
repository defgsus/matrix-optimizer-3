/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/5/2016</p>
*/

#include <atomic>

#include <QMutex>
#include <QMutexLocker>
#include <QMap>

#include "refcounted.h"
#include "io/error.h"
#include "io/log.h"

#if 0
#   define MO_REF_DEBUG(arg__) MO_PRINT(arg__)
#   define MO_ENABLE_REF_STATS
#else
#   define MO_REF_DEBUG(unused__) { }
//#   define MO_ENABLE_REF_STATS
#endif

namespace MO {

namespace
{
#ifdef MO_ENABLE_REF_STATS
    struct RefDesc_
    {
        QString className;
    };

    static QMutex refMapMutex_(QMutex::Recursive);

    static QMap<const RefCounted*, struct RefDesc_> & refMap_()
    {
        static QMap<const RefCounted*, struct RefDesc_> * refMap = 0;
        if (!refMap)
            refMap = new QMap<const RefCounted*, struct RefDesc_>();
        return *refMap;
    }
#endif
}

struct RefCounted::P_RefCountedPrivate
{
    P_RefCountedPrivate()
        : refcount(1)
    { }

    std::atomic_int refcount;
    QString destroyReason;
};

RefCounted::RefCounted(const QString& className)
    : p_refcount_private_(new P_RefCountedPrivate)
{
    Q_UNUSED(className);
#ifdef MO_ENABLE_REF_STATS
    RefDesc_ info;
    info.className = className;
    {
        QMutexLocker lock(&refMapMutex_);

        if (refMap_().contains(this))
            MO_WARNING("DUPLICATE REF " << refInstanceName());

        refMap_().insert(this, info);
    }
#endif
    MO_REF_DEBUG("REF CREATE " << refInstanceName(this));
}

RefCounted::~RefCounted()
{
    MO_REF_DEBUG("REF DESTROY " << refInstanceName(this) << " '"
                 << p_refcount_private_->destroyReason << "'");
    delete p_refcount_private_;

#ifdef MO_ENABLE_REF_STATS
    QMutexLocker lock(&refMapMutex_);
    refMap_().remove(this);
#endif
}

void RefCounted::addRef(const QString& reason)
{
    Q_UNUSED(reason);
    MO_REF_DEBUG("REF ADD " << refInstanceName(this) << " '" << reason << "' NOW "
             << (p_refcount_private_->refcount+1));
    ++p_refcount_private_->refcount;
}

void RefCounted::releaseRef(const QString& reason)
{
    Q_UNUSED(reason);
    MO_REF_DEBUG("REF REL " << refInstanceName(this) << " '" << reason << "' NOW "
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



const  QString RefCounted::refClassName() const
{
    return refClassName(this);
}

QString RefCounted::refInstanceName() const
{
    return refInstanceName(this);
}

QString RefCounted::refInstanceName(const RefCounted* ref)
{
    QString s =
        QString("%1[0x%2]")
            .arg(refClassName(ref))
            .arg(uint64_t(ref), 16, 16, QChar('.'));
    return s;
}

QString RefCounted::refClassName(const RefCounted* ref)
{
    Q_UNUSED(ref);
#ifdef MO_ENABLE_REF_STATS
    QMutexLocker lock(&refMapMutex_);
    auto i = refMap_().find(ref);
    if (i != refMap_().end())
        return i.value().className;
#else
    return "RefCounted";
#endif
}


} // namespace MO


