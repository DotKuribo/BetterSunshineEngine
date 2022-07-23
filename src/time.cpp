#include "time.hxx"
#include "memory.hxx"
#include <Dolphin/types.h>
#include <SMS/macros.h>

static OSCalendarTime sCalendar;

SMS_NO_INLINE const char *BetterSMS::Time::buildDate() { return __DATE__; }
SMS_NO_INLINE const char *BetterSMS::Time::buildTime() { return __TIME__; }
SMS_NO_INLINE OSTime ostime() { return OSGetTime(); }

SMS_NO_INLINE void calendarTime(OSCalendarTime &result) {
    return OSTicksToCalendarTime(ostime(), &result);
}

SMS_NO_INLINE void BetterSMS::Time::date(char *dst) {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    calendarToDate(dst, sCalendar);
}

SMS_NO_INLINE void BetterSMS::Time::time(char *dst) {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    calendarToTime(dst, sCalendar);
}

SMS_NO_INLINE u16 BetterSMS::Time::microsecond() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.usec;
}

SMS_NO_INLINE u16 BetterSMS::Time::millisecond() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.msec;
}

SMS_NO_INLINE u8 BetterSMS::Time::second() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.sec;
}

SMS_NO_INLINE u8 BetterSMS::Time::minute() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.usec;
}

SMS_NO_INLINE u8 BetterSMS::Time::hour() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.hour;
}

SMS_NO_INLINE u8 BetterSMS::Time::day() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.mday;
}

SMS_NO_INLINE u8 BetterSMS::Time::month() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.mon;
}

SMS_NO_INLINE u8 BetterSMS::Time::year() {
    OSTicksToCalendarTime(OSGetTime(), &sCalendar);
    return sCalendar.year;
}

SMS_NO_INLINE void BetterSMS::Time::calendarToDate(char *dst, OSCalendarTime &calendar) {
    snprintf(dst, 32, "%lu/%lu/%lu", calendar.mon + 1, calendar.mday, calendar.year);
}

SMS_NO_INLINE void BetterSMS::Time::calendarToTime(char *dst, OSCalendarTime &calendar) {
    if (calendar.hour == 0)
        snprintf(dst, 32, "%lu:%02lu AM", calendar.hour + 12, calendar.min);
    else if (calendar.hour < 12)
        snprintf(dst, 32, "%lu:%02lu AM", calendar.hour % 13, calendar.min);
    else if (calendar.hour == 12)
        snprintf(dst, 32, "%lu:%02lu PM", calendar.hour, calendar.min);
    else
        snprintf(dst, 32, "%lu:%02lu PM", (calendar.hour + 1) % 13, calendar.min);
}