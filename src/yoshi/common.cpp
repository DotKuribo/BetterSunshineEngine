#include <SMS/actor/Mario.hxx>
#include <SMS/actor/Yoshi.hxx>

#include "yoshi.hxx"

SMS_NO_INLINE bool BetterSMS::Yoshi::isMounted(TYoshi *yoshi) {
    return yoshi->mState == TYoshi::MOUNTED;
}

SMS_NO_INLINE bool BetterSMS::Yoshi::isMounted(TMario *player) {
    return player->mYoshi->mState == TYoshi::MOUNTED;
}

SMS_NO_INLINE bool BetterSMS::Yoshi::isGreenYoshi(TYoshi *yoshi) {
    return yoshi->mType == TYoshi::GREEN;
}

SMS_NO_INLINE bool BetterSMS::Yoshi::isGreenYoshi(TMario *player) {
    return isGreenYoshi(player->mYoshi);
}

SMS_NO_INLINE bool BetterSMS::Yoshi::isGreenYoshiMounted(TYoshi *yoshi) {
    return isGreenYoshi(yoshi) && yoshi->mState == TYoshi::MOUNTED;
}

SMS_NO_INLINE bool BetterSMS::Yoshi::isGreenYoshiMounted(TMario *player) {
    return isGreenYoshiMounted(player->mYoshi);
}

SMS_NO_INLINE bool BetterSMS::Yoshi::isGreenYoshiAscendingWater(TMario *player) {
    return player->mAttributes.mIsWater &&
           player->mController->mButtons.mInput & TMarioGamePad::EButtons::A &&
           isGreenYoshiMounted(player->mYoshi);
}