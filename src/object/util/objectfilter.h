/** @file objectfilter.h

    @brief Filter for Object lists

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.10.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_OBJECTFILTER_H
#define MOSRC_OBJECT_UTIL_OBJECTFILTER_H

#include <QSet>
#include <QObject>

#include "object/object_fwd.h"
#include "io/log.h"

namespace MO {

class ObjectFilter
{
public:
    ObjectFilter();

    // ------ static helper --------

    /** Appends all objects the influence objects in @p input to @p output.
        Each modulator is only added once. */
    template <class OBJ>
    static void addAllModulators(const QList<OBJ*>& input, QList<OBJ*>& output);

    // ---------- setter -----------

    /** Should child objects be added? */
    void setExpandObjects(bool enable) { expand_ = enable; }

    /** Sets the Object::Type mask of included objects */
    void setTypeFilter(int type_mask) { typeFilter_ = type_mask; }

    /** Sets a required parent object, or NULL for don't-care */
    void setParentFilter(Object * o) { parentFilter_ = o; }

    /** If @p o is != NULL, objects are required to belonging to o. */
    void setBelongToFilter(Object * o) { belongToFilter_ = o; }

    // ---------- getter -----------

    bool expandObjects() const { return expand_; }
    int typeFilter() const { return typeFilter_; }
    Object * parentFilter() const { return parentFilter_; }
    Object * belongToFilter() const { return belongToFilter_; }

    // ---------- filter -----------

    /** Returns if object should be included */
    bool isInFilter(const Object *) const;

    /** Apply filter and expansion of all objects
        in the @p input list into @p output list.
        @p output is not cleared, everything is appended.
        Each object will only be appended once to @p output.
        Returns true when the objects added to @p output are any different from @p input. */
    bool transform(const QList<Object*>& input, QList<Object*>& output) const;
    template <class OBJ>
    bool transform(const QList<OBJ*>& input, QList<OBJ*>& output) const;
    /** Also performs filtering by class */
    template <class OBJ, class OBJ2>
    bool transform(const QList<OBJ*>& input, QList<OBJ2*>& output) const;

    /** Apply filter and expansion to all objects in the list.
        Each object will only be contained once after this call.
        Returns true when @p list is changed. */
    bool transform(QList<Object*>& list) const;

    /** Filters/Expands the set of objects in @p list into @p set, if settings require it.
        Returns true when the objects added to @p set are any different from @p list. */
    bool transform(const QList<Object*> & list, QSet<Object*> & set) const;
    template <class OBJ>
    bool transform(const QList<OBJ*> & list, QSet<OBJ*> & set) const;

    /** Expands the set of objects if settings require it.
        Filters will be applied before expansion but filtered objects in @p set remain.
        Returns true when @p set is changed. */
    bool expand(QSet<Object*> & set) const;
    template <class OBJ>
    bool expand(QSet<OBJ*> & set) const;

    /** Expands object @p o into the @p set if settings require it.
        @p o itself is not inserted by this function.
        Returns true when @p set is changed,
        e.g. when @p o is in the filter and has children who are in the filter. */
    bool expand(Object * o, QSet<Object*> & set) const;
    template <class OBJ>
    bool expand(OBJ * o, QSet<OBJ*> & set) const;

private:

    bool expand_;
    int typeFilter_;
    Object * parentFilter_,
           * belongToFilter_;
};




// -------------------------- templ impl -------------------------

template <class OBJ>
bool ObjectFilter::transform(const QList<OBJ*>& input, QList<OBJ*>& output) const
{
    QSet<OBJ*> set;

    bool r = transform(input, set);

    for (auto o : set)
        output.append(o);

    return r;
}

template <class OBJ, class OBJ2>
bool ObjectFilter::transform(const QList<OBJ*>& input, QList<OBJ2*>& output) const
{
    QSet<OBJ*> set;

    bool r = transform(input, set);

    for (auto i : set)
        if (auto o = dynamic_cast<OBJ2*>(i))
            output.append(o);

    return r;
}

template <class OBJ>
bool ObjectFilter::transform(const QList<OBJ *> &list, QSet<OBJ *> &set) const
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

template <class OBJ>
bool ObjectFilter::expand(QSet<OBJ*> &set) const
{
    if (!expand_)
        return false;

    QSet<OBJ*> out;

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

template <class OBJ>
bool ObjectFilter::expand(OBJ * o, QSet<OBJ*>& set) const
{
    if (!expand_ || !isInFilter(o))
        return false;

    bool r = false;
    for (auto c : o->childObjects())
    {
        if (isInFilter(c))
            r |= expand(static_cast<OBJ*>(c), set);
    }

    return r;
}


template <class OBJ>
void ObjectFilter::addAllModulators(const QList<OBJ*>& input, QList<OBJ*>& output)
{
    QSet<OBJ*> set;
    for (auto c : input)
    {
        auto list = c->getModulatingObjects();
        for (auto i : list)
        {
            MO_DEBUG(c->idName() << " <- " << i->idName());
            if (OBJ * o = qobject_cast<OBJ*>(i))
                set.insert(o);
        }
    }

    for (auto o : set)
        output.append(o);
}

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_OBJECTFILTER_H
