/** @file objectfilter.cpp

    @brief Filter for Object lists

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.10.2014</p>
*/

#include "objectfilter.h"
#include "object/object.h"
#include "io/log.h"

namespace MO {


ObjectFilter::ObjectFilter()
    : expand_       (false),
      typeFilter_   (Object::TG_ALL),
      parentFilter_ (0),
      belongToFilter_(0)
{
}

bool ObjectFilter::isInFilter(const Object * o) const
{
//    MO_DEBUG("check " << o->idName());

    if (!(o->type() & typeFilter_))
        return false;

    if (parentFilter_ && o->parentObject() != o)
        return false;

    if (belongToFilter_ && o != belongToFilter_)
    {
//        MO_DEBUG("- " << o->idName() << " == " << belongToFilter_->idName() << " ?");
        Object * p = o->parentObject();
        while (p != belongToFilter_)
        {
            if (!p)
                return false;
            p = p->parentObject();
        }
    }

    return true;
}

bool ObjectFilter::transform(const QList<Object *> &input, QList<Object *> &output) const
{
    QSet<Object*> set;

    bool r = transform(input, set);

    for (auto o : set)
        output.append(o);

    return r;
}

bool ObjectFilter::transform(QList<Object *> &list) const
{
    QSet<Object*> set;

    if (!transform(list, set))
        return false;

    // replace list
    list.clear();
    for (auto o : set)
        list.append(o);

    return true;
}

bool ObjectFilter::transform(const QList<Object *> &list, QSet<Object *> &set) const
{
    // -- copy objects --

    // determine change
    bool r = false;

    for (auto o : list)
        if (isInFilter(o))
        {
            r |= set.contains(o); // change because of dupl. object
            set.insert(o);
        }
        else
            r = true; // change from filter

    if (!r)
        return false;

    // expand
    expand(set);

    return true;
}

bool ObjectFilter::expand(QSet<Object *> &set) const
{
    if (!expand_)
        return false;

    QSet<Object*> out;

    for (auto o : set)
    {
        if (isInFilter(o))
        {
            out.insert(o);
            expand(o, out);
        }
    }

    if (out.isEmpty())
        return false;

    set.swap(out);

    return true;
}


bool ObjectFilter::expand(Object* o, QSet<Object*>& set) const
{
    if (!expand_ || !isInFilter(o))
        return false;

    bool r = false;
    for (auto c : o->childObjects())
    {
        if (isInFilter(c))
            r |= expand(c, set);
    }

    return r;
}

} // namespace MO
