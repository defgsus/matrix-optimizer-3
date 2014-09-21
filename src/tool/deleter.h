/** @file deleter.h

    @brief Automatic scoped deleter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/21/2014</p>
*/

#ifndef MOSRC_TOOL_DELETER_H
#define MOSRC_TOOL_DELETER_H

namespace MO {

template <class T>
class ScopedDeleter
{
public:
    ScopedDeleter(T * object)
        : o_    (object)
    { }

    ~ScopedDeleter()
    {
        delete o_;
    }

    void detach()
    {
        o_ = 0;
    }

private:

    T * o_;
};


} // namespace MO


#endif // MOSRC_TOOL_DELETER_H
