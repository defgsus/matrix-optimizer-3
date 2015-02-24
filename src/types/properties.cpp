/** @file properties.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#include "properties.h"
#include "io/log.h"

namespace MO {


Properties::Properties()
{
}


QVariant Properties::get(const QString &id) const
{
    auto i = p_props_.find(id);

#ifdef MO_DO_DEBUG
    if (i == p_props_.end())
        MO_DEBUG("Properties::get(\"" << id << "\") unknown");
#endif

    return i == p_props_.end() ? QVariant() : i.value();
}

QVariant Properties::get(const QString &id, const QVariant& def) const
{
    auto i = p_props_.find(id);

    return i == p_props_.end() ? def : i.value();
}

void Properties::set(const QString &id, const QVariant & v)
{
    p_props_.insert(id, v);
}


} // namespace MO
