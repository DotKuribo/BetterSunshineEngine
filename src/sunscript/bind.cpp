#include <SMS/SPC/SpcBinary.hxx>

#include "p_sunscript.hxx"
#include "module.hxx"

#define BIND_SYMBOL(binary, symbol, func)                                                          \
    (binary)->bindSystemDataToSymbol((symbol), reinterpret_cast<u32>(&(func)))
static void initCustomFunctions(TSpcBinary *spcBinary, const char *symbol, u32 address) {
    spcBinary->bindSystemDataToSymbol(symbol, address);
    BIND_SYMBOL(spcBinary, "vectorTranslate", Spc::vectorTranslate);
    BIND_SYMBOL(spcBinary, "vectorTranslatef", Spc::vectorTranslatef);
    BIND_SYMBOL(spcBinary, "vectorScalef", Spc::vectorScalef);
    BIND_SYMBOL(spcBinary, "vectorMagnitude", Spc::vectorMagnitude);
    BIND_SYMBOL(spcBinary, "vectorNormalize", Spc::vectorNormalize);
    BIND_SYMBOL(spcBinary, "setActorPosToOther", Spc::setActorPosToOther);
    BIND_SYMBOL(spcBinary, "setActorRotToOther", Spc::setActorRotToOther);
    BIND_SYMBOL(spcBinary, "getActorPos", Spc::getActorPos);
    BIND_SYMBOL(spcBinary, "setActorPos", Spc::setActorPos);
    BIND_SYMBOL(spcBinary, "setActorPosf", Spc::setActorPosf);
    BIND_SYMBOL(spcBinary, "getActorRot", Spc::getActorRot);
    BIND_SYMBOL(spcBinary, "setActorRot", Spc::setActorRot);
    BIND_SYMBOL(spcBinary, "setActorRotf", Spc::setActorRotf);
    BIND_SYMBOL(spcBinary, "spawnObjByID", Spc::spawnObjByID);
    BIND_SYMBOL(spcBinary, "isMultiplayerActive", Spc::isDebugMode);
    BIND_SYMBOL(spcBinary, "getActivePlayers", Spc::getActivePlayers);
    BIND_SYMBOL(spcBinary, "getMaxPlayers", Spc::getMaxPlayers);
    BIND_SYMBOL(spcBinary, "getPlayerByIndex", Spc::getPlayerByIndex);
    BIND_SYMBOL(spcBinary, "getDateAsStr", Spc::getDateAsStr);
    BIND_SYMBOL(spcBinary, "getTimeAsStr", Spc::getTimeAsStr);
    BIND_SYMBOL(spcBinary, "getPlayerInputByIndex", Spc::getPlayerInputByIndex);
    BIND_SYMBOL(spcBinary, "read8", Spc::read8);
    BIND_SYMBOL(spcBinary, "read16", Spc::read16);
    BIND_SYMBOL(spcBinary, "read32", Spc::read32);
    BIND_SYMBOL(spcBinary, "write8", Spc::write8);
    BIND_SYMBOL(spcBinary, "write16", Spc::write16);
    BIND_SYMBOL(spcBinary, "write32", Spc::write32);
    BIND_SYMBOL(spcBinary, "memcpy", Spc::memcpy_);
    BIND_SYMBOL(spcBinary, "memmove", Spc::memmove_);
    BIND_SYMBOL(spcBinary, "memcmp", Spc::memcmp_);
    BIND_SYMBOL(spcBinary, "memset", Spc::memset_);
    BIND_SYMBOL(spcBinary, "formatStrBySpec", Spc::formatStrBySpec);
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
SMS_PATCH_BL(SMS_PORT_REGION(0x80219380, 0x802112D4, 0, 0), initCustomFunctions);
SMS_PATCH_BL(SMS_PORT_REGION(0x80289920, 0x802816AC, 0, 0), initCustomFunctions);
#undef BIND_SYMBOL