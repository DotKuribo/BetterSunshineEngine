#pragma once

#include <Dolphin/types.h>
#include <Dolphin/string.h>

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
#include <SMS/game/Application.hxx>
#include <SMS/game/MarDirector.hxx>
#include <SMS/GC2D/Guide.hxx>
#include <SMS/mapobj/MapObjInit.hxx>
#include <SMS/npc/BaseNPC.hxx>
#include <SMS/object/DemoCannon.hxx>
#include <SMS/params/Params.hxx>
#include <SMS/screen/ShineFader.hxx>

#pragma region SettingImplementation

struct ValueRange {
    f32 mStart;
    f32 mStop;
    f32 mStep;
};

class SingleSetting {
public:
    enum class ValueKind { BOOL, INT, FLOAT };

public:
    SingleSetting() = delete;
    SingleSetting(const char *name, void *valuePtr);
    virtual ~SingleSetting() {}

    virtual ValueKind getKind() const = 0;

    const char *getName() { return mName; }
    void setName(const char *name) { mName = name; } 

    bool getBool() const { return *reinterpret_cast<bool *>(mValuePtr); }
    int getInt() const { return *reinterpret_cast<int *>(mValuePtr); }
    float getFloat() const { return *reinterpret_cast<float *>(mValuePtr); }

    void setBool(bool active) const { *reinterpret_cast<bool *>(mValuePtr) = active; }
    void setInt(int x) const { *reinterpret_cast<int *>(mValuePtr) = x; }
    void setFloat(int x) const { *reinterpret_cast<float *>(mValuePtr) = x; }

    bool isRangeSingular() const {
        return mValueRange.mStart == mValueRange.mStop && getKind() != ValueKind::BOOL;
    }

    const ValueRange &getValueRange() const { return mValueRange; }
    void setValueRange(const ValueRange &range) { mValueRange = range; }

private:
    const char *mName;
    void *mValuePtr;
    ValueRange mValueRange;
};

class BoolSetting final : SingleSetting {
    ValueKind getKind() const override { return ValueKind::BOOL; }
};

class IntSetting final : SingleSetting {
    ValueKind getKind() const override { return ValueKind::INT; }
};

class FloatSetting final : SingleSetting {
    ValueKind getKind() const override { return ValueKind::FLOAT; }
};

class SettingsGroup {
public:
    using SettingsList = JGadget::TList<SingleSetting *>;

public:
    SettingsGroup() = delete;
    SettingsGroup(const char *name) : mName(name), mSettings(), mIsUserEditable(true) {}
    SettingsGroup(const char *name, const SettingsList &settings) : mName(name), mSettings(settings), mIsUserEditable(true) {}

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
    SettingsList getSettings() const { return mSettings; }

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

namespace BetterSMS {
    namespace Settings {
        JGadget::TList<SettingsGroup *> getGroups();
        SettingsGroup *getGroup(const char *groupName);
    }
}