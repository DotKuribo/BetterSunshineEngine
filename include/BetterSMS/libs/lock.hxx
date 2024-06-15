#pragma once

#include <Dolphin/OS.h>
#include <Dolphin/types.h>

#include <JSystem/utility.hxx>

namespace BetterSMS {

    template <class MutexT> class TLock {
    public:
        TLock(MutexT &mutex) {
            mMutex = JSystem::addressof(mutex);
            mMutex->lock();
        }

        ~TLock() { mMutex->unlock(); }

        TLock(const TLock &)            = delete;
        TLock &operator=(const TLock &) = delete;
        TLock(TLock &&)                 = delete;
        TLock &operator=(TLock &&)      = delete;

    private:
        MutexT *mMutex;
    };

    template <class MutexT> class TTryLock {
    public:
        TTryLock(MutexT &mutex) {
            mMutex = JSystem::addressof(mutex);
            mMutex->tryLock();
        }

        ~TTryLock() { mMutex->unlock(); }

        TTryLock(const TTryLock &)            = delete;
        TTryLock &operator=(const TTryLock &) = delete;
        TTryLock(TTryLock &&)                 = delete;
        TTryLock &operator=(TTryLock &&)      = delete;

    private:
        MutexT *mMutex;
    };

    class TAtomicGuard {
    public:
        TAtomicGuard() { mAtomicState = OSDisableInterrupts(); }
        ~TAtomicGuard() { OSRestoreInterrupts(mAtomicState); }

        TAtomicGuard(const TAtomicGuard &)            = delete;
        TAtomicGuard &operator=(const TAtomicGuard &) = delete;
        TAtomicGuard(TAtomicGuard &&)                 = delete;
        TAtomicGuard &operator=(TAtomicGuard &&)      = delete;

    private:
        bool mAtomicState;
    };

}  // namespace BetterSMS