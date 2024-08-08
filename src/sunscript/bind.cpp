#include <SMS/SPC/SpcBinary.hxx>

#include "module.hxx"
#include "sunscript.hxx"
#include "p_sunscript.hxx"

#include "libs/global_vector.hxx"

using namespace BetterSMS;

struct BuiltinData {
    const char *mKey;
    Spc::SpcFunction mFn;
};

static const BuiltinData sBuiltinDataBSMS[] = {
    {"spawnObjByID", Spc::spawnObjByID},
    {"getPlayerInputByIndex", Spc::getPlayerInputByIndex},
    {"getStageBGM", Spc::getStageBGM},
    {"queueStream", Spc::queueStream},
    {"playStream", Spc::playStream},
    {"pauseStream", Spc::pauseStream},
    {"stopStream", Spc::stopStream},
    {"seekStream", Spc::seekStream},
    {"nextStream", Spc::nextStream},
    {"skipStream", Spc::skipStream},
    {"getStreamVolume", Spc::getStreamVolume},
    {"setStreamVolume", Spc::setStreamVolume},
    {"getStreamLooping", Spc::getStreamLooping},
    {"setStreamLooping", Spc::setStreamLooping},
};

static TGlobalVector<BuiltinData> sBuiltinDatas;

bool BetterSMS::Spc::registerBuiltinFunction(const char *key, SpcFunction function) {
    for (const BuiltinData &data : sBuiltinDatas) {
        if (data.mKey == key) {
            return false;
        }
    }
    sBuiltinDatas.push_back({key, function});
    return true;
}

#define BIND_SYMBOL(binary, symbol, func)                                                          \
    (binary)->bindSystemDataToSymbol((symbol), reinterpret_cast<u32>(&(func)))

static void initModuleFunctions(TSpcBinary* spcBinary) {
    spcBinary->initUserBuiltin();

    for (const BuiltinData &data : sBuiltinDataBSMS) {
        BIND_SYMBOL(spcBinary, data.mKey, data.mFn);
    }

    for (const BuiltinData &data : sBuiltinDatas) {
        BIND_SYMBOL(spcBinary, data.mKey, data.mFn);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80222584, 0, 0, 0), initModuleFunctions);

#undef BIND_SYMBOL
