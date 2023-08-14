#include "time.hxx"
#include "memory.hxx"
#include <Dolphin/types.h>
#include <SMS/macros.h>

#include "module.hxx"

static OSCalendarTime sCalendar;

BETTER_SMS_FOR_EXPORT const char *BetterSMS::Time::buildDate() { return __DATE__; }
BETTER_SMS_FOR_EXPORT const char *BetterSMS::Time::buildTime() { return __TIME__; }
BETTER_SMS_FOR_EXPORT OSTime BetterSMS::Time::ostime() { return OSGetTime(); }

BETTER_SMS_FOR_EXPORT void BetterSMS::Time::getCalendar(OSCalendarTime &result) {
    return OSTicksToCalendarTime(ostime(), &result);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Time::calendarToDate(char *dst, const OSCalendarTime &calendar) {
    snprintf(dst, 32, "%lu/%lu/%lu", calendar.mon + 1, calendar.mday, calendar.year);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Time::calendarToTime(char *dst, const OSCalendarTime &calendar) {
    if (calendar.hour == 0)
        snprintf(dst, 32, "%lu:%02lu AM", calendar.hour + 12, calendar.min);
    else if (calendar.hour < 12)
        snprintf(dst, 32, "%lu:%02lu AM", calendar.hour % 13, calendar.min);
    else if (calendar.hour == 12)
        snprintf(dst, 32, "%lu:%02lu PM", calendar.hour, calendar.min);
    else
        snprintf(dst, 32, "%lu:%02lu PM", (calendar.hour + 1) % 13, calendar.min);
}