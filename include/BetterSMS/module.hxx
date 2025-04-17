#pragma once

#ifndef HW_DOL
#define HW_DOL
#endif

#include <Dolphin/types.h>
#include <SMS/macros.h>
#include <sdk.h>

#include "memory.hxx"
#include "settings.hxx"
#include "libs/optional.hxx"

#define BETTER_SMS_MODULE_NAME "Better Sunshine Engine"
#define BETTER_SMS_AUTHOR_NAME "JoshuaMK"

#ifndef BETTER_SMS_VERSION
#define BETTER_SMS_VERSION "(unknown version)"
#endif

#ifndef BETTER_SMS_EXTRA_AREAS
#define BETTER_SMS_EXTRA_AREAS 1
#endif

/* Misc compiler info */
#ifndef BETTER_SMS_VERSION
#define BETTER_SMS_VERSION "vUnknown"
#endif

#ifdef __VERSION__
#define BETTER_SMS_CC_VERSION __VERSION__
#elif defined(__CWCC__)
#define BETTER_SMS_CC_VERSION "CWCC " STRINGIZE(__CWCC__)
#else
#define BETTER_SMS_CC_VERSION "Unknown"
#endif

#if SMS_DEBUG
#define BETTER_SMS_VERSION_TAG                                                                     \
     BETTER_SMS_VERSION " (DEBUG)"
#else
#define BETTER_SMS_VERSION_TAG                                                                     \
    BETTER_SMS_VERSION " (RELEASE)"
#endif

/* CONFIGURATION DEFINES */

#ifndef BETTER_SMS_USE_PS_MATH
#define BETTER_SMS_USE_PS_MATH 1
#endif

#define BETTER_SMS_FOR_CALLBACK SMS_NO_INLINE
#define BETTER_SMS_FOR_EXPORT   SMS_NO_INLINE

namespace BetterSMS {

    struct ModuleInfo {
        ModuleInfo() = delete;
        ModuleInfo(const char *name, u16 major, u16 minor, Settings::SettingsGroup *settings)
            : mName(name), mVersionMajor(major), mVersionMinor(minor), mSettings(settings) {
            settings->mModule = this;
        }

        const char *mName;
        u16 mVersionMajor;
        u16 mVersionMinor;
        Settings::SettingsGroup *mSettings;
    };

    class BugsSetting final : public Settings::SwitchSetting {
    public:
        BugsSetting(const char *name)
            : SwitchSetting(name, &mBugsValue), mBugsValue(true), mIsUnlocked(true) {}
        ~BugsSetting() override {}

        bool isUnlocked() const override { return mIsUnlocked; }

        void load(JSUMemoryInputStream &in) override {
            in.read(&mIsUnlocked, 1);
            {
                bool b;
                in.read(&b, 1);
                setBool(b);
            }
        }
        void save(JSUMemoryOutputStream &out) override {
            out.write(&mIsUnlocked, 1);
            out.write(mValuePtr, 1);
        }

        inline void lock() { mIsUnlocked = false; }
        inline void unlock() { mIsUnlocked = true; }

    private:
        bool mIsUnlocked;
        bool mBugsValue;
    };

    class CollisionFixesSetting final : public Settings::SwitchSetting {
    public:
        CollisionFixesSetting(const char *name)
            : SwitchSetting(name, &mCollisionFixesFlag), mCollisionFixesFlag(true),
              mIsUnlocked(true) {
            mValueChangedCB = CollisionFixesSetting::valueChanged;
        }

        bool isUnlocked() const override { return mIsUnlocked; }

        void load(JSUMemoryInputStream &in) override {
            in.read(&mIsUnlocked, 1);
            {
                bool b;
                in.read(&b, 1);
                setBool(b);
            }
        }
        void save(JSUMemoryOutputStream &out) override {
            out.write(&mIsUnlocked, 1);
            out.write(mValuePtr, 1);
        }

        inline void lock() { mIsUnlocked = false; }
        inline void unlock() { mIsUnlocked = true; }

    private:
        static void valueChanged(void *old, void *cur, ValueKind kind) {
        }

        bool mIsUnlocked;
        bool mCollisionFixesFlag;
    };

    bool isGameEmulated();
    bool isWiiMode();
    bool isMusicBeingStreamed();
    bool isMusicStreamingAllowed();

    bool isDebugMode();
    void setDebugMode(bool);

    optional<ModuleInfo> getModuleInfo(const char *key);

    bool isModuleRegistered(const char *key);
    bool registerModule(const ModuleInfo &info);

    bool triggerAutoSave();

    int getScreenRenderWidth();
    int getScreenOrthoWidth();
    f32 getScreenToFullScreenRatio();
    f32 getScreenRatioAdjustX();
    f32 getFrameRate();
    BugsSetting *getBugFixesSetting();
    BugsSetting *getExploitFixesSetting();
    CollisionFixesSetting *getCollisionFixesSetting();

    // Necessary for dynamic linking (If a declaration is used instead, it crashes when the module
    // isn't present)
    template <typename T = void *> inline T getExportedFunctionPointer(const char *symbol) {
        return reinterpret_cast<T>(pp::Import(symbol));
    }
}  // namespace BetterSMS