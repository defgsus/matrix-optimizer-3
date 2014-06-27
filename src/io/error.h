/** @file error.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/26/2014</p>
*/

#ifndef MOSRC_IO_ERROR_H
#define MOSRC_IO_ERROR_H

#include <exception>
#include <sstream>

#include "io/streamoperators_qt.h"

namespace MO {

class Exception : public std::exception
{
public:

    enum
    {
        /** Basic Exception, given no cause. */
        UNKNOWN,
        /** Issued by an assertion MO_ASSERT() */
        ASSERT,
        /** Programmer's fault! */
        LOGIC,
        /** IO read error */
        READ,
        /** IO write error */
        WRITE,
        /** IO expected something else */
        VERSION_MISMATCH
    };

    Exception(int cause = UNKNOWN) throw() : cause_(cause) { }
    virtual ~Exception() throw() { }

    virtual const char * what() const throw() { return text_.c_str(); }
    int cause() const throw() { return cause_; }

    template <class T>
    Exception& operator << (const T& value) { addToStream(value); return *this; }

    template <class T>
    void addToStream(const T& value)
    { std::stringstream s; s << value; text_ += s.str(); }

protected:

    int cause_;
    std::string text_;
};


class LogicException : public Exception
{
public:
    LogicException(int cause = UNKNOWN) throw() : Exception(cause) { }
    ~LogicException() throw() { }

    template <class T>
    LogicException& operator << (const T& value) { addToStream(value); return *this; }
};

class IoException : public Exception
{
public:
    IoException(int cause = UNKNOWN) throw() : Exception(cause) { }
    ~IoException() throw() { }

    template <class T>
    IoException& operator << (const T& value) { addToStream(value); return *this; }
};


#define MO_ERROR(text__) \
{ throw ::MO::Exception() << text__; }

#define MO_IO_ERROR(cause__, text__) \
{ throw ::MO::IoException(::MO::Exception::cause__) << text__; }

#define MO_LOGIC_ERROR(text__) \
{ throw ::MO::LogicException(::MO::Exception::LOGIC) << text__; }

#define MO_ASSERT(cond__, text__) \
if (!(cond__)) \
{ \
    throw ::MO::Exception(::MO::Exception::ASSERT) \
        << "assertion in " << __FILE__ << ":" << __LINE__ \
        << "\n" << text__; \
}

} // namespace MO


#endif // MOSRC_IO_ERROR_H
