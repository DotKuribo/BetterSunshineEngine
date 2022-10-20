#pragma once

#include <Dolphin/types.h>
#include <Dolphin/string.h>
#include <Dolphin/printf.h>

#include <JSystem/JDrama/JDRActor.hxx>
#include <JSystem/JDrama/JDRDirector.hxx>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRNameRef.hxx>
#include <JSystem/JGadget/List.hxx>
#include <JSystem/JGeometry.hxx>
#include <JSystem/JStage/JSGObject.hxx>

#include <SMS/actor/Yoshi.hxx>
#include <SMS/actor/item/Coin.hxx>
#include <SMS/actor/item/Shine.hxx>
#include <SMS/game/GCConsole2.hxx>
#include <SMS/game/GCConsole2.hxx>
#include <SMS/game/PerformList.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/game/MarDirector.hxx>
#include <SMS/GC2D/Guide.hxx>
#include <SMS/mapobj/MapObjInit.hxx>
#include <SMS/npc/BaseNPC.hxx>
#include <SMS/object/DemoCannon.hxx>
#include <SMS/params/Params.hxx>
#include <SMS/screen/ShineFader.hxx>
#include <SMS/assert.h>

namespace BetterSMS {
    namespace Settings {
        class SettingsGroup;

        SettingsGroup *getGroup(const char *name);

        bool isGroupRegistered(const char *name);
        bool registerGroup(const char *name, SettingsGroup *group);
        bool deregisterGroup(const char *name);

        #pragma region SettingImplementation

        struct ValueRange {
            f32 mStart;
            f32 mStop;
            f32 mStep;
        };

        class SingleSetting {
        public:
            enum class ValueKind { BOOL, INT, FLOAT };
            typedef void (*ValueChangedCallback)(void *old, void *cur, ValueKind);

        public:
            SingleSetting() = delete;
            SingleSetting(const char *name, void *valuePtr)
                : mName(name), mValuePtr(valuePtr), mValueRange() {
                mValueChangedCB = nullptr;
            }
            virtual ~SingleSetting() {}

            virtual ValueKind getKind() const            = 0;
            virtual void getValueStr(char *dst) const    = 0;
            virtual void setValue(const void *val) const = 0;
            virtual void prevValue()                     = 0;
            virtual void nextValue()                     = 0;

            const char *getName() { return mName; }
            void setName(const char *name) { mName = name; }

            void *getValue() const { return mValuePtr; }
            bool getBool() const { return *reinterpret_cast<bool *>(mValuePtr); }
            int getInt() const { return *reinterpret_cast<int *>(mValuePtr); }
            float getFloat() const { return *reinterpret_cast<float *>(mValuePtr); }

            void setBool(bool active) const {
                auto kind = getKind();
                SMS_ASSERT(kind == ValueKind::BOOL,
                           "Mismatching setting types found, setting a non BOOL to BOOL value!");
                bool lc = active;
                if (*reinterpret_cast<bool *>(mValuePtr) != lc && mValueChangedCB)
                    mValueChangedCB(mValuePtr, &lc, getKind());
                *reinterpret_cast<bool *>(mValuePtr) = lc;
            }

            void setInt(int x) const {
                auto kind = getKind();
                SMS_ASSERT(kind == ValueKind::INT,
                           "Mismatching setting types found, setting a non INT to INT value!");
                int lc = x;
                if (*reinterpret_cast<int *>(mValuePtr) != lc && mValueChangedCB)
                    mValueChangedCB(mValuePtr, &lc, getKind());
                *reinterpret_cast<int *>(mValuePtr) = lc;
            }

            void setFloat(float f) const {
                auto kind = getKind();
                SMS_ASSERT(kind == ValueKind::FLOAT,
                           "Mismatching setting types found, setting a non FLOAT to FLOAT value!");
                float lc = f;
                if (*reinterpret_cast<float *>(mValuePtr) != lc && mValueChangedCB)
                    mValueChangedCB(mValuePtr, &lc, getKind());
                *reinterpret_cast<float *>(mValuePtr) = lc;
            }

            bool isRangeSingular() const {
                return mValueRange.mStart == mValueRange.mStop && getKind() != ValueKind::BOOL;
            }

            const ValueRange &getValueRange() const { return mValueRange; }
            void setValueRange(const ValueRange &range) { mValueRange = range; }

        protected:
            const char *mName;
            void *mValuePtr;
            ValueRange mValueRange;
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
            void prevValue() override {
                setBool(getBool() ^ true);
            }
            void nextValue() override {
                setBool(getBool() ^ true);
            }
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
            IntSetting(const char *name, void *valuePtr) : SingleSetting(name, valuePtr) {}
            ~IntSetting() override {}

            ValueKind getKind() const override { return ValueKind::INT; }
            void getValueStr(char *dst) const override { snprintf(dst, 11, "%i", getInt()); }
            void setValue(const void *val) const override {
                *reinterpret_cast<int *>(mValuePtr) = *reinterpret_cast<const int *>(val);
            }
            void prevValue() override {
                setInt(clampValueToRange(getInt() - mValueRange.mStep));
            }
            void nextValue() override {
                setInt(clampValueToRange(getInt() + mValueRange.mStep));
            }

        private:
            int clampValueToRange(int x) const {
                if (x > mValueRange.mStop) {
                    x = mValueRange.mStart + (x - mValueRange.mStop - 1);
                } else if (x < mValueRange.mStart) {
                    x = mValueRange.mStop + (x - mValueRange.mStart + 1);
                }
                return x;
            }
        };

        class FloatSetting : public SingleSetting {
        public:
            FloatSetting() = delete;
            FloatSetting(const char *name, void *valuePtr) : SingleSetting(name, valuePtr) {}
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

        private:
            f32 clampValueToRange(f32 x) const {
                if (x > mValueRange.mStop) {
                    x = mValueRange.mStart + (x - mValueRange.mStop);
                } else if (x < mValueRange.mStart) {
                    x = mValueRange.mStop + (x - mValueRange.mStart);
                }
                return x;
            }
        };

        class SettingsGroup {
        public:
            using SettingsList = JGadget::TList<SingleSetting *>;

        public:
            SettingsGroup() = delete;
            SettingsGroup(const char *name) : mName(name), mSettings(), mIsUserEditable(true) {}
            SettingsGroup(const char *name, const SettingsList &settings)
                : mName(name), mSettings(settings), mIsUserEditable(true) {}

            bool isUserEditable() const { return mIsUserEditable; }
            void setUserEditable(bool editable) { mIsUserEditable = editable; }

            const char *getName() const { return mName; }
            SingleSetting *getSetting(const char *name) {
                for (auto &setting : mSettings) {
                    if (strcmp(setting->getName(), name) == 0) {
                        return setting;
                    }
                }
                return nullptr;
            }
            SettingsList &getSettings() { return mSettings; }

            void setName(const char *name) { mName = name; }
            void setSettings(const SettingsList &settings) { mSettings = settings; }

            void addSetting(SingleSetting *setting) { mSettings.insert(mSettings.end(), setting); }
            void removeSetting(SingleSetting *setting) {
                for (auto iter = mSettings.begin(); iter != mSettings.end(); ++iter) {
                    if (*iter == setting) {
                        mSettings.erase(iter);
                        return;
                    }
                }
            };
            void removeSetting(const char *name) {
                for (auto iter = mSettings.begin(); iter != mSettings.end(); ++iter) {
                    if (iter->getName() == name) {
                        mSettings.erase(iter);
                        return;
                    }
                }
            }

        private:
            const char *mName;
            SettingsList mSettings;
            bool mIsUserEditable;
        };

#pragma endregion
    }
}