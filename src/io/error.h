/** @file error.h

    @brief Exception classes and error macros

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/26/2014</p>
*/

#ifndef MOSRC_IO_ERROR_H
#define MOSRC_IO_ERROR_H

#include <exception>
#include <iostream>
#include <sstream>

#include "io/streamoperators_qt.h"
#include "io/applicationtime.h"
#include "io/currentthread.h"
#include "io/isclient.h"
// to pass MO_WARNING messages to the server
#include "network/netlog.h"
// also include QTextStream << stuff operators
// for NetworkLogger::operator <<
#include "io/qtextstreamoperators.h"

#ifndef NDEBUG
/** Enables MO_ASSERT() */
#   define MO_ENABLE_ASSERT
#endif

#define MO_ENABLE_WARNING

/** *currently* adds a lot of infos to exceptions by means
    of many catch/rethrow on different levels. */
#define MO_EXTENDED_EXCEPTIONS



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
        VERSION_MISMATCH,
        /** parser expected something not found */
        PARSE,
        /** Some third party error */
        API
    };

    Exception(int cause = UNKNOWN) throw() : cause_(cause)
        { text_ = "[" + currentThreadName().toStdString()
                + "/" + applicationTimeString().toStdString() + "] "; }

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

class GlException : public Exception
{
public:
    GlException(int cause = UNKNOWN) throw() : Exception(cause) { }
    ~GlException() throw() { }

    template <class T>
    GlException& operator << (const T& value) { addToStream(value); return *this; }
};

class AudioException : public Exception
{
public:
    AudioException(int cause = UNKNOWN) throw() : Exception(cause) { }
    ~AudioException() throw() { }

    template <class T>
    AudioException& operator << (const T& value) { addToStream(value); return *this; }
};

// --------------- exception extension ----------------------

#ifdef MO_EXTENDED_EXCEPTIONS
#   define MO_EXTEND_EXCEPTION(command__, text__)                       \
        try { command__; }                                              \
        catch (::MO::Exception & e__) { e__ << "\n" << text__; throw; }
#else
#   define MO_EXTEND_EXCEPTION(command__, unused__) command__;
#endif

// ---------------------- error -----------------------------

#define MO_ERROR(text__) \
{ throw ::MO::Exception() << text__; }

#define MO_IO_ERROR(cause__, text__) \
{ throw ::MO::IoException(::MO::Exception::cause__) << text__; }

#define MO_GL_ERROR(text__) \
{ throw ::MO::GlException() << text__; }

#define MO_LOGIC_ERROR(text__) \
{ throw ::MO::LogicException(::MO::Exception::LOGIC) << text__; }

#define MO_AUDIO_ERROR(cause__, text__) \
{ throw ::MO::AudioException(::MO::Exception::cause__) << text__; }

// ----------------------- warning -------------------------

#ifdef MO_ENABLE_WARNING
#   define MO_WARNING_IMPL_(text__) \
        { std::cerr << "[" << ::MO::applicationTimeString() << "] " << text__ << std::endl; \
          if (isClient()) \
            MO_NETLOG(APP_WARNING, text__); }
#else
#   define MO_WARNING_IMPL_(unused__) { }
#endif


#define MO_WARNING(text__) \
    MO_WARNING_IMPL_("WARNING: " << text__)

#define MO_IO_WARNING(cause__, text__) \
    MO_WARNING_IMPL_("IO-WARNING: " << text__)

#define MO_GL_WARNING(text__) \
    MO_WARNING_IMPL_("OPENGL-WARNING: " << text__)

// ----------------------- assert ---------------------------

#ifdef MO_ENABLE_ASSERT
    #define MO_ASSERT(cond__, text__) \
    if (!(cond__)) \
    { \
        throw ::MO::Exception(::MO::Exception::ASSERT) \
            << "assertion '" << #cond__ << "' in " << __FILE__ << ":" << __LINE__ \
            << "\n" << text__; \
    }
#else
    #define MO_ASSERT(unused__, unused___) { }
#endif

} // namespace MO


#endif // MOSRC_IO_ERROR_H
