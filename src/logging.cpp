#include <Dolphin/OS.h>
#include <Dolphin/printf.h>
#include <Dolphin/stdarg.h>
#include <SMS/macros.h>

#include "logging.hxx"
#include "module.hxx"

BETTER_SMS_FOR_EXPORT void BetterSMS::Console::log(const char *msg, ...) {
    va_list vargs;
    va_start(vargs, msg);
    OSReport(msg, vargs);
    va_end(vargs);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Console::hardwareLog(const char *msg, ...) {
    if (BetterSMS::isGameEmulated())
        return;

    va_list vargs;
    va_start(vargs, msg);
    OSReport(msg, vargs);
    va_end(vargs);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Console::emulatorLog(const char *msg, ...) {
    if (!BetterSMS::isGameEmulated())
        return;

    va_list vargs;
    va_start(vargs, msg);
    OSReport(msg, vargs);
    va_end(vargs);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Console::debugLog(const char *msg, ...) {
    if (!BetterSMS::isDebugMode())
        return;

    va_list vargs;
    va_start(vargs, msg);
    OSReport(msg, vargs);
    va_end(vargs);
}