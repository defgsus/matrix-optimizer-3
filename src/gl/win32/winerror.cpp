#ifdef MO_OS_WIN

#include <sstream>
#include <locale>

#include <Windows.H>
#include "winerror.h"

#ifdef _MSC_VER
// argument conversion, possible loss of data
#pragma warning(disable : 4267)
#endif

std::string getLastWinErrorString()
{
    return getWinErrorString(GetLastError());
}

std::string getWinErrorString(DWORD err)
{
    std::wstring buf;
    buf.resize(2048);

    DWORD res = FormatMessage(
                //FORMAT_MESSAGE_ALLOCATE_BUFFER
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                err,
                LANG_USER_DEFAULT,
                &buf[0],
                buf.size(),
                NULL);

    if (!res)
    {
        std::stringstream s;
        s << "error " << err;
        return s.str();
    }
#if defined(__GNUC__) && defined(MO_OS_WIN)
    /** @todo wstring_convert and codecvt not really found in my MinGW */
    {
        std::stringstream s;
        s << "error " << err;
        return s.str();
    }

#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
    return conv.to_bytes(&buf[0]);
#endif
}

#endif
