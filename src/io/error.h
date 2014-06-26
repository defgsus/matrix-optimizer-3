/** @file error.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/26/2014</p>
*/

#ifndef MOSRC_IO_ERROR_H
#define MOSRC_IO_ERROR_H

#include <exception>
#include <sstream>

namespace MO {

class Exception : public std::exception
{
public:

    enum
    {
        UNKNOWN,
        READ,
        WRITE,
        VERSION_MISMATCH,

    };

    Exception(int cause = UNKNOWN) throw() : cause_(cause) { }
    ~Exception() throw() { }

    virtual const char * what() const throw() { return text_.c_str(); }
    int cause() const throw() { return cause_; }

    template <class T>
    Exception& operator << (const T& value)
    { std::stringstream s; s << value; text_ += s.str(); return *this; }

protected:

    int cause_;
    std::string text_;
};



#define MO_ERROR(text__) \
{ throw ::MO::Exception() << text__; }

#define MO_IO_ERROR(cause__, text__) \
{ throw ::MO::Exception(::MO::Exception::cause__) << text__; }

} // namespace MO


#endif // MOSRC_IO_ERROR_H
