#pragma once

namespace BetterSMS::Console {
    void log(const char *msg, ...);
    void hardwareLog(const char *msg, ...);
    void emulatorLog(const char *msg, ...);
    void debugLog(const char *msg, ...);
}  // namespace BetterSMS::Console
