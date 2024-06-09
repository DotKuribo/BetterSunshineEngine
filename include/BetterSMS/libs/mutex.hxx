#pragma once

#include <Dolphin/OS.h>
#include <Dolphin/types.h>

namespace BetterSMS {

    class TMutex {
    public:
        TMutex() { OSInitMutex(&mMutex); }

        TMutex(const TMutex &)            = delete;
        TMutex &operator=(const TMutex &) = delete;

        TMutex(TMutex &&)            = delete;
        TMutex &operator=(TMutex &&) = delete;

        void lock() { OSLockMutex(&mMutex); }
        void tryLock() { OSTryLockMutex(&mMutex); }
        void unlock() { OSUnlockMutex(&mMutex); }

        ~TMutex() {}

    private:
        OSMutex mMutex;
    };

}  // namespace BetterSMS