#include <SMS/SPC/SpcBinary.hxx>

#include "module.hxx"
#include "p_sunscript.hxx"

#define BIND_SYMBOL(binary, symbol, func)                                                          \
    (binary)->bindSystemDataToSymbol((symbol), reinterpret_cast<u32>(&(func)))

static void initEclipseFunctions(TSpcBinary* spcBinary) {
    spcBinary->initUserBuiltin();
    BIND_SYMBOL(spcBinary, "spawnObjByID", Spc::spawnObjByID);
    BIND_SYMBOL(spcBinary, "getPlayerInputByIndex", Spc::getPlayerInputByIndex);
    BIND_SYMBOL(spcBinary, "getStageBGM", Spc::getStageBGM);
    BIND_SYMBOL(spcBinary, "queueStream", Spc::queueStream);
    BIND_SYMBOL(spcBinary, "playStream", Spc::playStream);
    BIND_SYMBOL(spcBinary, "pauseStream", Spc::pauseStream);
    BIND_SYMBOL(spcBinary, "stopStream", Spc::stopStream);
    BIND_SYMBOL(spcBinary, "seekStream", Spc::seekStream);
    BIND_SYMBOL(spcBinary, "nextStream", Spc::nextStream);
    BIND_SYMBOL(spcBinary, "skipStream", Spc::skipStream);
    BIND_SYMBOL(spcBinary, "getStreamVolume", Spc::getStreamVolume);
    BIND_SYMBOL(spcBinary, "setStreamVolume", Spc::setStreamVolume);
    BIND_SYMBOL(spcBinary, "getStreamLooping", Spc::getStreamLooping);
    BIND_SYMBOL(spcBinary, "setStreamLooping", Spc::setStreamLooping);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80222584, 0, 0, 0), initEclipseFunctions);
#undef BIND_SYMBOL