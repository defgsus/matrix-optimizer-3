/** @file selection.h

    @brief Templated class to select a bunch of objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.10.2014</p>
*/

#ifndef MOSRC_TOOL_SELECTION_H
#define MOSRC_TOOL_SELECTION_H

#include <set>

namespace MO {

template <class T>
class Selection
{
public:

    // --------- container ----------

    typedef typename std::set<T>::const_iterator const_iterator;

    const_iterator begin() const { return s_.cbegin(); }
    const_iterator end() const { return s_.cend(); }

    // ---------- getter ------------

    /** Returns true if @p o is selected */
    bool isSelected(T o) const
    {
        return s_.find(o) != s_.end();
    }

    // --------- setter -------------

    /** Clears all selections */
    void clear() { s_.clear(); }

    /** Selects the object */
    void select(T o) { s_.insert(o); }

    /** Flips selected state */
    void flip(T o)
    {
        auto i = s_.find(o);
        if (i == s_.end())
            s_.insert(o);
        else
            s_.erase(i);
    }

private:

    std::set<T> s_;
};




} // namespace MO



#endif // MOSRC_TOOL_SELECTION_H
