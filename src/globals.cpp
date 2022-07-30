#include <Dolphin/types.h>

#include "collision/warp.hxx"
#include "globals.hxx"
#include "module.hxx"

void *BetterSMS::sPRMFile                                                  = nullptr;
BetterSMS::Collision::TWarpCollisionList *BetterSMS::sWarpColArray         = nullptr;
BetterSMS::Collision::TWarpCollisionList *BetterSMS::sWarpColPreserveArray = nullptr;
bool BetterSMS::sIsAudioStreaming                                          = false;
bool BetterSMS::sIsAudioStreamAllowed                                      = false;
bool BetterSMS::sIsDebugMode                                               = SMS_DEBUG;

bool BetterSMS::isDebugMode() { return sIsDebugMode; }
bool BetterSMS::isMusicBeingStreamed() { return sIsAudioStreaming; }
bool BetterSMS::isMusicStreamingAllowed() { return sIsAudioStreamAllowed; }
void BetterSMS::setDebugMode(bool active) { sIsDebugMode = active; }