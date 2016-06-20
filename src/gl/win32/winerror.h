#ifndef MCWSRC_GL_WIN32_WINERROR_H
#define MCWSRC_GL_WIN32_WINERROR_H

#ifdef MO_OS_WIN

#include <string>
#include <WinBase.h>

/** Returns the string from GetLastError() windows function */
std::string getLastWinErrorString();

/** Returns the string for the given windows error code */
std::string getWinErrorString(DWORD err);

#endif

#endif // WINERROR_H
