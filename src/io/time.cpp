/** @file time.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.02.2015, updated 2016</p>
*/

#ifdef _MSC_VER
// argument conversion, possible loss of data
#pragma warning(disable : 4244)
#endif

#if 0 // c++14 version

    // XXX Note: As usual :( the windows clock is not at all high resolution
    // so the code further below is used instead

    #include <chrono>
    #include <thread>
    #include "time.h"

namespace MO {

    double systemTime()
    {
        auto tp = std::chrono::high_resolution_clock::now();
        return
          std::chrono::duration_cast<std::chrono::nanoseconds>
                (tp.time_since_epoch()).count() * double(0.000000001);
    }

    void setTimerMethod(int) { }

#else // old non-c++14 version

    #include <thread>
    #include <chrono>
    #include "time.h"

    #ifdef __gnu_linux__
    #   include <ctime>
    #   define MO_USE_UNIX
    #endif

    #if defined(_WIN32) || defined(WIN32)
    #   include <atomic>
    #   include <iostream>
    #   include <iomanip>
    #   include <windows.h>
    #   include <winbase.h>
    #   define MO_USE_WIN
    #   undef min
    #   undef max
    #endif

    #if defined(__APPLE__) || defined(__MACH__)
    #   include <mach/clock.h>
    #   include <mach/mach.h>
    #   define MO_USE_APPLE
    #endif

namespace MO {

    namespace Private {

    #ifdef MO_USE_WIN
        /** @brief precise system-time on windows platforms

            implementated from tutorial
            John Nilsson,
            "Implement a Continuously Updating, High-Resolution Time Provider for Windows"
            http://msdn.microsoft.com/en-us/magazine/cc163996.aspx

            @version 2012/10/02 started as part of MAG2
            */
        class WinClock
        {
            double offset_;
            public:

            WinClock()
                : offset_   (0.)
            {
                ::QueryPerformanceFrequency(&frequency);
                simplistic_synchronize(ref_point);
            }

            ~WinClock()
            { }

            double time()
            {
                get_time();

                uint64_t tik = (uint64_t(current_time.dwHighDateTime) << (sizeof(DWORD)*8))
                        | uint64_t(current_time.dwLowDateTime);
                //std::cout << tik << std::endl;
                long double ti = (long double).0000001 * tik + offset_;
#if 0
                // XXX Hack-around for wrap in system counter
                static std::atomic<long double> prevTime(0.);
                long double p = prevTime;
                prevTime = ti;
                if (ti < p)// - 0.0000001)
                {
                    std::cerr << "TIMEWRAP ti="
                              << std::setprecision(30) << ti
                              << ", prev=" << p
                              << ", offs=" << offset_
                              << std::endl;
                    offset_ = p - ti;
                    ti += offset_;
                }
#endif
                return ti;
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

    double systemTime()
    {
    #ifdef MO_USE_WIN

        return Private::win_clock.time();

    #elif defined(MO_USE_APPLE)

        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);

        return mts.tv_sec + double(0.000000001) * mts.tv_nsec;

    #elif defined(MO_USE_UNIX)

        timespec cls;
        clock_gettime(CLOCK_MONOTONIC, &cls);
        // second + nanoseconds
        return cls.tv_sec + double(0.000000001) * cls.tv_nsec;

    #else

    #   error Platform not supported

    #endif
    }



#endif

#if 0
void init_sleep(double sec)
{
    for (int64_t i = 1; i < 10; ++i)
    {
        std::cout << "-- minus " << i << " -- " << std::endl;

        for (int k=0; k<100;++k)
        {
            TimeMessure tm;

            int64_t ms = sec*1000-1;
            if (ms > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));

            double t = tm.time();
            while (t < sec) { t = tm.time(); }
            if (t > sec)
            {
                std::cout << "overwaited " << (t-sec) << "\n";
            }
        }
    }
}
#endif

void sleep_seconds(double sec)
{
    TimeMessure tm;
#if 1
    int64_t ms = sec*1000.-16.;
    if (ms > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
    while (tm.time() < sec);
}

void sleep_seconds_lowres(double sec)
{
    int64_t ms = sec*1000;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


} // namespace MO

#include <iostream>
#include <functional>
#include <algorithm>
#include <string>

namespace MO {

namespace
{
    double testSystemTime()
    {
        double s = systemTime(), s1 = s;
        while (s1 == s)
            s1 = systemTime();
        return s1 - s;
    }

    void testDelta(const std::string& name, std::function<double()> foo)
    {
        double dmin = foo(),
               dmax = dmin;
        for (int i=0; i<1000; ++i)
        {
            double d = foo();
            dmin = std::min(dmin, d);
            dmax = std::max(dmax, d);
        }
        std::cout << name << " delta = "
                  << long(dmin*1000000) << "ns - "
                  << long(dmax*1000000) << "ns" << std::endl;
    }
}


void testClock()
{
    for (int i=0;i<10;++i)
        testDelta("systemTime", testSystemTime);
}

} // namespace MO
