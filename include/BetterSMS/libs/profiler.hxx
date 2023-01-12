#pragma once

#include <Dolphin/OS.h>

#include <SMS/macros.h>

namespace BetterSMS {
    class TProfiler {
    public:
        TProfiler()                  = delete;
        TProfiler(const TProfiler &) = delete;
        TProfiler(TProfiler &&)      = delete;

        TProfiler(const char *function) : mFunction(function) {
            OSInitStopwatch(&mStopwatch, mFunction);
            start();
        }

        ~TProfiler() {
            report();
            stop();
        }

        void start() { OSStartStopwatch(&mStopwatch); }
        void stop() { OSStopStopwatch(&mStopwatch); }
        void report() { OSDumpStopwatch(&mStopwatch); }

    private:
        const char *mFunction;
        OSStopwatch mStopwatch;
    };
}

#define BETTERSMS_START_PROFILE BetterSMS::TProfiler __profiler = BetterSMS::TProfiler(SMS_FUNC_SIG)
#define BETTERSMS_STOP_PROFILE  __profiler.~TProfiler()