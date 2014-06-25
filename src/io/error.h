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

class BasicException : public std::exception
{
public:
    BasicException() { }

    virtual const char * what() const throw() { return text.c_str(); }

    template <class T>
    BasicException& operator << (const T& value)
    { std::stringstream s; s << value; text += s.str(); return *this; }

    std::string text;
};

class IoException : public BasicException
{

};


#define MO_ERROR(text__) \
{ ::MO::BasicException e; e << text__; throw e; }

#define MO_IO_ERROR(text__) \
{ ::MO::IoException e; e << text__; throw e; }

} // namespace MO


#endif // MOSRC_IO_ERROR_H
