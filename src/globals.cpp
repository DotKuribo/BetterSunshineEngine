#include <Dolphin/OS.h>
#include <Dolphin/types.h>
#include <SMS/macros.h>

#include "collision/warp.hxx"
#include "globals.hxx"
#include "module.hxx"

SMS_EXTERN_C OSBootInfo BootInfo;

void *BetterSMS::sPRMFile                                                  = nullptr;
BetterSMS::Collision::TWarpCollisionList *BetterSMS::sWarpColArray         = nullptr;
BetterSMS::Collision::TWarpCollisionList *BetterSMS::sWarpColPreserveArray = nullptr;
bool BetterSMS::sIsAudioStreaming                                          = false;
bool BetterSMS::sIsAudioStreamAllowed                                      = false;

#ifdef NDEBUG
bool BetterSMS::sIsDebugMode = false;
#else
bool BetterSMS::sIsDebugMode = true;
#endif

#if BETTER_SMS_WIDESCREEN
f32 BetterSMS::sScreenWidth = 700.0f;
#else
f32 BetterSMS::sScreenWidth  = 600.0f;
#endif

f32 BetterSMS::sFrameRate = 30.0f;

bool BetterSMS::isGameEmulated() { return BootInfo.mConsoleType == OS_CONSOLE_DEV_KIT3; }
bool BetterSMS::isDebugMode() { return true/*sIsDebugMode*/; }
bool BetterSMS::isMusicBeingStreamed() { return sIsAudioStreaming; }
bool BetterSMS::isMusicStreamingAllowed() { return sIsAudioStreamAllowed; }
void BetterSMS::setDebugMode(bool active) { sIsDebugMode = active; }
f32 BetterSMS::getScreenWidth() { return sScreenWidth; }
f32 BetterSMS::getScreenToFullScreenRatio() { return sScreenWidth / 600.0f; }
f32 BetterSMS::getFrameRate() { return sFrameRate; }