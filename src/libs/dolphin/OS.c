#include <Dolphin/OS.h>
#include <Dolphin/math.h>

#if 0
OSTime __OSTimeToSystemTime(OSTime time) {
  u32 _atomic_state = OSDisableInterrupts();
  time += *(OSTime *)0x800030D8;
  OSRestoreInterrupts(_atomic_state);
  return time;
}

void OSSetPeriodicAlarm(OSAlarm *alarm, OSTime start, OSTime period,
                        OSAlarmHandler handler) {
  u32 _atomic_state = OSDisableInterrupts();

  alarm->mPeriod = period;

  OSTime transTime = __OSTimeToSystemTime(start);
  alarm->mStart = transTime;

  InsertAlarm(alarm, (OSTick)transTime, 0, handler);

  OSRestoreInterrupts(_atomic_state);
}
#else
void OSSetPeriodicAlarm(OSAlarm *alarm, OSTime start, OSTime period, OSAlarmHandler handler) {
    const u32 iStatus = OSDisableInterrupts();

    alarm->mStart  = start;
    alarm->mPeriod = period;

    InsertAlarm(alarm, start - OSGetTime(), handler);

    OSRestoreInterrupts(iStatus);
}
#endif

void OSDumpStopwatch(OSStopwatch *watch) {
    OSReport("============================\n"
             "Stopwatch [%s]\n"
             "    Total: %llu us\n"
             "    Hits:  %u\n"
             "    Min:   %llu us\n"
             "    Max:   %llu us\n"
             "    Mean:  %llu us\n"
             "============================\n",
             watch->mName, watch->mTotal, watch->mHits, watch->mMin, watch->mMax,
             watch->mTotal / watch->mHits);
}

__attribute__((naked)) void DCDisable() {
    __asm__ volatile("sync                           \n\t"
                     "mfspr      3, 1008             \n\t"
                     "rlwinm     3, 3, 0, 18, 16     \n\t"
                     "mtspr      1008, 3             \n\t"
                     "blr                            \n\t");
}

__attribute__((naked)) void ICDisable() {
    __asm__ volatile("sync                           \n\t"
                     "mfspr      3, 1008             \n\t"
                     "rlwinm     3, 3, 0, 17, 15     \n\t"
                     "mtspr      1008, 3             \n\t"
                     "blr                            \n\t");
}