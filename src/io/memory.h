/** @file memory.h

    @brief Memory managment / counter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/11/2014</p>
*/

#ifndef MOSRC_IO_MEMORY_H
#define MOSRC_IO_MEMORY_H

namespace MO
{
    class Memory
    {
    public:

        static unsigned long int allocated();

    };

} // namespace MO

#if (0)

#include <new>

void * operator new (std::size_t size);
void * operator new (std::size_t size, const std::nothrow_t& nothrow_value) throw();

void * operator new[] (std::size_t size);
void * operator new[] (std::size_t size, const std::nothrow_t& nothrow_value) throw();

void operator delete (void* ptr) throw();
void operator delete (void* ptr, const std::nothrow_t& nothrow_constant) throw();

void operator delete[] (void* ptr) throw();
void operator delete[] (void* ptr, const std::nothrow_t& nothrow_constant) throw();

namespace MO
{
    class Memory
    {
    public:

        static std::size_t allocated();
        static std::size_t deallocated();
        static std::size_t peak();
        static std::ptrdiff_t lost();

    private:
        friend void * ::operator new (::std::size_t size);
        friend void   ::operator delete (void* ptr) throw();

        Memory();
        ~Memory();
    };

} // namespace MO

#endif



#endif // MOSRC_IO_MEMORY_H
