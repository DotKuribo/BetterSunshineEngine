#include "time.hxx"
#include "memory.hxx"
#include <Dolphin/types.h>
#include <SMS/macros.h>

#include "module.hxx"

static OSCalendarTime sCalendar;

BETTER_SMS_FOR_EXPORT const char *BetterSMS::Time::buildDate() { return __DATE__; }
BETTER_SMS_FOR_EXPORT const char *BetterSMS::Time::buildTime() { return __TIME__; }
BETTER_SMS_FOR_EXPORT OSTime BetterSMS::Time::ostime() { return OSGetTime(); }

BETTER_SMS_FOR_EXPORT void BetterSMS::Time::calendarTime(OSCalendarTime &result) {
    return OSTicksToCalendarTime(ostime(), &result);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Time::date(char *dst) {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    calendarToDate(dst, sCalendar);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Time::time(char *dst) {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    calendarToTime(dst, sCalendar);
}

BETTER_SMS_FOR_EXPORT u16 BetterSMS::Time::nanosecond() {
    return OSTicksToNanoseconds(OSGetTime());
}

BETTER_SMS_FOR_EXPORT u16 BetterSMS::Time::microsecond() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.usec;
}

BETTER_SMS_FOR_EXPORT u16 BetterSMS::Time::millisecond() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.msec;
}

BETTER_SMS_FOR_EXPORT u8 BetterSMS::Time::second() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.sec;
}

BETTER_SMS_FOR_EXPORT u8 BetterSMS::Time::minute() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.usec;
}

BETTER_SMS_FOR_EXPORT u8 BetterSMS::Time::hour() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.hour;
}

BETTER_SMS_FOR_EXPORT u8 BetterSMS::Time::day() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.mday;
}

BETTER_SMS_FOR_EXPORT u8 BetterSMS::Time::month() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.mon;
}

BETTER_SMS_FOR_EXPORT u8 BetterSMS::Time::year() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.year;
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