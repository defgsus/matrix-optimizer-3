#ifndef MOSRC_IO_ARCHITECTURE_H
#define MOSRC_IO_ARCHITECTURE_H

// http://stackoverflow.com/questions/1505582/determining-32-vs-64-bit-in-c

// Check windows
#if _WIN32 || _WIN64
   #if _WIN64
     #define MO_OS_64BIT
  #else
    #define MO_OS_32BIT
  #endif
#endif

// Check GCC
#if __GNUC__
  #if __x86_64__ || __ppc64__
    #define MO_OS_64BIT
  #else
    #define MO_OS_32BIT
  #endif
#endif


#if !defined(MO_OS_32BIT) && !defined(MO_OS_64BIT)
#   error Unknown bitsize of system
#endif


#endif // MOSRC_IO_ARCHITECTURE_H

