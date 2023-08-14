#pragma once

#include <Dolphin/OS.h>

namespace BetterSMS {
    namespace Time {
        const char *buildDate();
        const char *buildTime();
        OSTime ostime();
        void getCalendar(OSCalendarTime &result);
        void calendarToDate(char *dst, const OSCalendarTime &calendar);
        void calendarToTime(char *dst, const OSCalendarTime &calendar);
    }  // namespace Time
}  // namespace BetterSMS::Time