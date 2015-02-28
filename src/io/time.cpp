/** @file time.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.02.2015</p>
*/

#include <Qt>
#include "time.h"

#ifndef Q_OS_WIN
#   include <ctime>
#endif

#ifdef Q_OS_OSX
#   include <mach/clock.h>
#   include <mach/mach.h>
#endif


namespace MO {


namespace Private {

#ifdef Q_OS_WIN
    /** @brief precise system-time on windows platforms

        implementated from tutorial
        John Nilsson,
        "Implement a Continuously Updating, High-Resolution Time Provider for Windows"
        http://msdn.microsoft.com/en-us/magazine/cc163996.aspx

        @version 2012/10/02 started as part of MAG2
        */
    class WinClock
    {
        public:

        WinClock()
        {
            ::QueryPerformanceFrequency(&frequency);
            simplistic_synchronize(ref_point);
        }

        ~WinClock()
        { }

        double time()
        {
            get_time();
            return
                0.0000001 * current_time.dwLowDateTime;
        }

        private:

        struct reference_point
        {
            FILETIME file_time;
            LARGE_INTEGER counter;
        };

        reference_point ref_point;
        LARGE_INTEGER   frequency, li;
        FILETIME        current_time;

        // ____ implementation _____

        void simplistic_synchronize(reference_point& ref_point)
        {
          FILETIME      ft0 = { 0, 0 },
                        ft1 = { 0, 0 };

          //
          // Spin waiting for a change in system time. Get the matching
          // performance counter value for that time.
          //
          ::GetSystemTimeAsFileTime(&ft0);
          do
          {
            ::GetSystemTimeAsFileTime(&ft1);
            ::QueryPerformanceCounter(&li);
          }
          while ((ft0.dwHighDateTime == ft1.dwHighDateTime) &&
                 (ft0.dwLowDateTime == ft1.dwLowDateTime));

          ref_point.file_time = ft1;
          ref_point.counter = li;
        }



        void get_time()
        {
          LARGE_INTEGER li;

          ::QueryPerformanceCounter(&li);

          //
          // Calculate performance counter ticks elapsed
          //
          LARGE_INTEGER ticks_elapsed;

          ticks_elapsed.QuadPart = li.QuadPart -
              ref_point.counter.QuadPart;

          //
          // Translate to 100-nanoseconds intervals (FILETIME
          // resolution) and add to
          // reference FILETIME to get current FILETIME.
          //
          ULARGE_INTEGER filetime_ticks,
                         filetime_ref_as_ul;

          filetime_ticks.QuadPart =
              (ULONGLONG)((((double)ticks_elapsed.QuadPart/(double)
              frequency.QuadPart)*10000000.0)+0.5);
          filetime_ref_as_ul.HighPart = ref_point.file_time.dwHighDateTime;
          filetime_ref_as_ul.LowPart = ref_point.file_time.dwLowDateTime;
          filetime_ref_as_ul.QuadPart += filetime_ticks.QuadPart;

          //
          // Copy to result
          //
          current_time.dwHighDateTime = filetime_ref_as_ul.HighPart;
          current_time.dwLowDateTime = filetime_ref_as_ul.LowPart;
        }

    };

    static WinClock win_clock;
#endif

} // namespace Private


Double systemTime()
{
#ifdef Q_OS_WIN

    return Private::win_clock.time();

#elif defined(Q_OS_OSX)

    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);

    return mts.tv_sec + Double(0.000000001) * mts.tv_nsec;

#else

    timespec cls;
    clock_gettime(CLOCK_MONOTONIC, &cls);
    // second + nanoseconds
    return cls.tv_sec + Double(0.000000001) * cls.tv_nsec;

#endif
}


} // namespace MO
