#pragma once

#include <Dolphin/printf.h>
#include <Dolphin/string.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRActor.hxx>
#include <JSystem/JDrama/JDRDirector.hxx>
#include <JSystem/JDrama/JDRNameRef.hxx>
#include <JSystem/JGadget/List.hxx>
#include <JSystem/JGadget/UnorderedMap.hxx>
#include <JSystem/JGeometry/JGMVec.hxx>
#include <JSystem/JStage/JSGObject.hxx>

#include <SMS/GC2D/GCConsole2.hxx>
#include <SMS/GC2D/Guide.hxx>
#include <SMS/GC2D/ShineFader.hxx>
#include <SMS/MapObj/MapObjInit.hxx>
#include <SMS/MoveBG/Coin.hxx>
#include <SMS/MoveBG/DemoCannon.hxx>
#include <SMS/MoveBG/Shine.hxx>
#include <SMS/NPC/NpcBase.hxx>
#include <SMS/Player/Yoshi.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/System/Params.hxx>
#include <SMS/System/PerformList.hxx>
#include <SMS/assert.h>

#include "libs/global_list.hxx"

namespace BetterSMS {

    struct ModuleInfo;

    namespace Settings {
        enum class Priority { CORE, GAME, MODE };

        class SettingsGroup;

#pragma region SettingImplementation

        template <typename T> struct ValueRange {
            T mStart;
            T mStop;
            T mStep;
        };

        class SingleSetting {
        public:
            enum class ValueKind { BOOL, INT, FLOAT };
            typedef void (*ValueChangedCallback)(void *old, void *cur, ValueKind);

        public:
            SingleSetting() = delete;
            SingleSetting(const char *name, void *valuePtr)
                : mName(name), mValuePtr(valuePtr), mIsUserEditable(true),
                  mEditPriority(Priority::MODE) {
                mValueChangedCB = nullptr;
            }
            virtual ~SingleSetting() {}

            virtual bool isUnlocked() const { return true; }

            virtual ValueKind getKind() const             = 0;
            virtual void getValueStr(char *dst) const     = 0;
            virtual void setValue(const void *val) const  = 0;
            virtual void prevValue()                      = 0;
            virtual void nextValue()                      = 0;
            virtual void load(JSUMemoryInputStream &in)   = 0;
            virtual void save(JSUMemoryOutputStream &out) = 0;

            bool isUserEditable() const { return mIsUserEditable; }
            void setUserEditable(bool editable, Priority priority) {
                if (static_cast<int>(priority) <= static_cast<int>(mEditPriority)) {
                    mIsUserEditable = editable;
                    mEditPriority   = priority;
                }
            }

            const char *getName() { return mName; }
            void setName(const char *name) { mName = name; }

            void *getValue() const { return mValuePtr; }
            bool getBool() const { return *reinterpret_cast<bool *>(mValuePtr); }
            int getInt() const { return *reinterpret_cast<int *>(mValuePtr); }
            float getFloat() const { return *reinterpret_cast<float *>(mValuePtr); }

            void setBool(bool active) const {
                auto kind = getKind();
                SMS_DEBUG_ASSERT(
                    kind == ValueKind::BOOL,
                    "Mismatching setting types found, setting a non BOOL to BOOL value!");
                bool lc = active;
                if (*reinterpret_cast<bool *>(mValuePtr) != lc && mValueChangedCB)
                    mValueChangedCB(mValuePtr, &lc, getKind());
                *reinterpret_cast<bool *>(mValuePtr) = lc;
            }

            void setInt(int x) const {
                auto kind = getKind();
                SMS_DEBUG_ASSERT(
                    kind == ValueKind::INT,
                    "Mismatching setting types found, setting a non INT to INT value!");
                int lc = x;
                if (*reinterpret_cast<int *>(mValuePtr) != lc && mValueChangedCB)
                    mValueChangedCB(mValuePtr, &lc, getKind());
                *reinterpret_cast<int *>(mValuePtr) = lc;
            }

            void setFloat(float f) const {
                auto kind = getKind();
                SMS_DEBUG_ASSERT(
                    kind == ValueKind::FLOAT,
                    "Mismatching setting types found, setting a non FLOAT to FLOAT value!");
                float lc = f;
                if (*reinterpret_cast<float *>(mValuePtr) != lc && mValueChangedCB)
                    mValueChangedCB(mValuePtr, &lc, getKind());
                *reinterpret_cast<float *>(mValuePtr) = lc;
            }
            
            // Signals the changed callback which can update arbitrary memory
            void emit() { mValueChangedCB(mValuePtr, mValuePtr, getKind()); }

        protected:
            const char *mName;
            void *mValuePtr;
            bool mIsUserEditable;
            Priority mEditPriority;
            ValueChangedCallback mValueChangedCB;
        };

        class BoolSetting : public SingleSetting {
        public:
            BoolSetting() = delete;
            BoolSetting(const char *name, void *valuePtr) : SingleSetting(name, valuePtr) {}
            ~BoolSetting() override {}

            ValueKind getKind() const override { return ValueKind::BOOL; }
            void getValueStr(char *dst) const override {
                getBool() ? strncpy(dst, "TRUE", 5) : strncpy(dst, "FALSE", 6);
            }
            void setValue(const void *val) const override {
                *reinterpret_cast<bool *>(mValuePtr) = *reinterpret_cast<const bool *>(val);
            }
            void prevValue() override { setBool(getBool() ^ true); }
            void nextValue() override { setBool(getBool() ^ true); }
            void load(JSUMemoryInputStream &in) override {
                u8 b;
                in.read(&b, 1);
                setBool(b > 1 ? false : b);  // Reset value if corrupt
            }
            void save(JSUMemoryOutputStream &out) override { out.write(mValuePtr, 1); }
        };

        class SwitchSetting : public BoolSetting {
        public:
            SwitchSetting() = delete;
            SwitchSetting(const char *name, void *valuePtr) : BoolSetting(name, valuePtr) {}
            ~SwitchSetting() override {}

            void getValueStr(char *dst) const override {
                getBool() ? strncpy(dst, "ON", 3) : strncpy(dst, "OFF", 4);
            }
        };

        class IntSetting : public SingleSetting {
        public:
            IntSetting() = delete;
            IntSetting(const char *name, void *valuePtr)
                : SingleSetting(name, valuePtr), mValueRange() {}
            ~IntSetting() override {}

            ValueKind getKind() const override { return ValueKind::INT; }
            void getValueStr(char *dst) const override { snprintf(dst, 11, "%i", getInt()); }
            void setValue(const void *val) const override {
                *reinterpret_cast<int *>(mValuePtr) = *reinterpret_cast<const int *>(val);
            }
            void prevValue() override { setInt(clampValueToRange(getInt() - mValueRange.mStep)); }
            void nextValue() override { setInt(clampValueToRange(getInt() + mValueRange.mStep)); }
            void load(JSUMemoryInputStream &in) override {
                int x;
                in.read(&x, 4);

                if (x < mValueRange.mStart || x > mValueRange.mStop)
                    x = mValueRange.mStart;

                setInt(x);  // Reset value if corrupt
            }
            void save(JSUMemoryOutputStream &out) override { out.write(mValuePtr, 4); }

            const ValueRange<int> &getValueRange() const { return mValueRange; }
            void setValueRange(const ValueRange<int> &range) { mValueRange = range; }

        protected:
            ValueRange<int> mValueRange;

        private:
            int clampValueToRange(int x) const {
                if (x > mValueRange.mStop) {
                    x = mValueRange.mStart + (x - mValueRange.mStop - 1);
                } else if (x < mValueRange.mStart) {
                    x = mValueRange.mStop + (x - mValueRange.mStart + 1);
                }
                if (x < mValueRange.mStart && x > mValueRange.mStop)
                    x = mValueRange.mStart;
                return x;
            }
        };

        class FloatSetting : public SingleSetting {
        public:
            FloatSetting() = delete;
            FloatSetting(const char *name, void *valuePtr)
                : SingleSetting(name, valuePtr), mValueRange() {}
            ~FloatSetting() override {}

            ValueKind getKind() const override { return ValueKind::FLOAT; }
            void getValueStr(char *dst) const override { snprintf(dst, 16, "%f", getFloat()); }
            void setValue(const void *val) const override {
                *reinterpret_cast<float *>(mValuePtr) = *reinterpret_cast<const float *>(val);
            }
            void prevValue() override {
                setFloat(clampValueToRange(getFloat() - mValueRange.mStep));
            }
            void nextValue() override {
                setFloat(clampValueToRange(getFloat() + mValueRange.mStep));
            }
            void load(JSUMemoryInputStream &in) override {
                float f;
                in.read(&f, 4);

                if (f < mValueRange.mStart || f > mValueRange.mStop)
                    f = mValueRange.mStart;

                setFloat(f);  // Reset value if corrupt
            }
            void save(JSUMemoryOutputStream &out) override { out.write(mValuePtr, 4); }

            const ValueRange<f32> &getValueRange() const { return mValueRange; }
            void setValueRange(const ValueRange<f32> &range) { mValueRange = range; }

        protected:
            ValueRange<f32> mValueRange;

        private:
            int clampValueToRange(int x) const {
                if (x > mValueRange.mStop) {
                    x = mValueRange.mStart + (x - mValueRange.mStop - 1);
                } else if (x < mValueRange.mStart) {
                    x = mValueRange.mStop + (x - mValueRange.mStart + 1);
                }
                if (x < mValueRange.mStart && x > mValueRange.mStop)
                    x = mValueRange.mStart;
                return x;
            }
        };

        struct SettingsSaveInfo {
            const char *mSaveName;
            size_t mBlocks;
            u32 mGameCode;
            u16 mCompany;
            u8 mBannerFmt;
            const ResTIMG *mBannerImage;
            u16 mIconFmt;
            u16 mIconSpeed;
            size_t mIconCount;
            const ResTIMG *mIconTable;  // Should be one BTI image vertically stacked for each icon
            bool mSaveGlobal;
        };

        const char *getGroupName(const SettingsGroup &);

        class SettingsGroup {
        public:
            friend struct ::BetterSMS::ModuleInfo;
            using SettingsList = TGlobalList<SingleSetting *>;

        public:
            SettingsGroup() = delete;
            SettingsGroup(u8 major, u8 minor, Priority prio)
                : mModule(), mVersion((major << 8) | minor), mOrderPriority(prio), mSettings() {}
            SettingsGroup(u8 major, u8 minor, const SettingsList &settings,
                          Priority prio)
                : mModule(), mVersion((major << 8) | minor), mOrderPriority(prio),
                  mSettings(settings) {}

            friend const char *Settings::getGroupName(const SettingsGroup &group);

            u8 getMajorVersion() const { return (mVersion >> 8) & 0xFF; }
            u8 getMinorVersion() const { return mVersion & 0xFF; }

            SingleSetting *getSetting(const char *name) {
                for (auto &setting : mSettings) {
                    if (strcmp(setting->getName(), name) == 0) {
                        return setting;
                    }
                }
                return nullptr;
            }

            SettingsList &getSettings() { return mSettings; }
            const SettingsList &getSettings() const { return mSettings; }

            SettingsSaveInfo &getSaveInfo() { return mSaveInfo; }
            const SettingsSaveInfo &getSaveInfo() const { return mSaveInfo; }

            Priority getPriority() const { return mOrderPriority; }

            void setSettings(const SettingsList &settings) { mSettings = settings; }

            void addSetting(SingleSetting *setting) {
                mSettings.insert(mSettings.end(), setting);
            }

            void removeSetting(SingleSetting *setting) {
                for (auto iter = mSettings.begin(); iter != mSettings.end(); ++iter) {
                    if (*iter == setting) {
                        mSettings.erase(iter);
                        return;
                    }
                }
            }

            void removeSetting(const char *name) {
                for (auto iter = mSettings.begin(); iter != mSettings.end(); ++iter) {
                    if (strcmp((*iter)->getName(), name) == 0) {
                        mSettings.erase(iter);
                        return;
                    }
                }
            }

        private:
            ModuleInfo *mModule;
            u16 mVersion;
            SettingsList mSettings;
            SettingsSaveInfo mSaveInfo;
            Priority mOrderPriority;
        };

#pragma endregion
    }  // namespace Settings
}  // namespace BetterSMS