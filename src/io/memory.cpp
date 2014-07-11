/** @file memory.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/11/2014</p>
*/
#if (0)

#include <cstdlib>

#include "memory.h"

namespace {

    static std::size_t allocated_ = 0;
    static std::size_t deallocated_ = 0;

    static std::unordered_map<void*, std::size_t> hash_;
}

namespace MO
{

    Memory::Memory()
    {
    }

    Memory::~Memory()
    {
    }


    std::size_t Memory::allocated() { return allocated_; }
    std::size_t Memory::deallocated() { return deallocated_; }
    std::size_t Memory::peak() { return allocated_; }
    std::ptrdiff_t Memory::lost()
    {
        return (std::ptrdiff_t)allocated() - (std::ptrdiff_t)deallocated();
    }

} // namespace MO







void * operator new (std::size_t size)
{
    void * ptr = malloc(size);

    allocated_ += size;

    return ptr;
}

void * operator new (std::size_t size, std::nothrow_t&) throw()
{
    try
    {
        return operator new (size);
    }
    catch (...)
    {
        return 0;
    }
}

void * operator new[] (std::size_t size)
{
    return operator new (size);
}

void * operator new[] (std::size_t size, std::nothrow_t& nothrow_value) throw()
{
    return operator new (size, nothrow_value);
}


void operator delete (void* ptr) throw()
{
    if (ptr)
    {
        free(ptr);
    }
}

void operator delete (void* ptr, const std::nothrow_t&) throw()
{
    operator delete (ptr);
}

void operator delete[] (void* ptr) throw()
{
    operator delete (ptr);
}

void operator delete[] (void* ptr, const std::nothrow_t&) throw()
{
    operator delete (ptr);
}


#endif
