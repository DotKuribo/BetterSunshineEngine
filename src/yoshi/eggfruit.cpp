#include <JSystem/J3D/J3DAnimation.hxx>

#include <SMS/macros.h>


#include "module.hxx"
#include "player.hxx"
#include "stage.hxx"

using namespace BetterSMS;

#if BETTER_SMS_GREEN_YOSHI

static bool isYoshiEggNeedFruit(THitActor *gpFruit) {
    if (!Stage::getStageConfiguration()->mIsEggFree.get())
        return TMapObjBase::isFruit(gpFruit);

    return true;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BC868, 0x801B4720, 0, 0), isYoshiEggNeedFruit);

#define EGG_IGNORE  0
#define EGG_SUCCESS 1
#define EGG_REJECT  2

static u8 isYoshiEggFree(TEggYoshi *gpEgg, THitActor *gpFruit) {
    auto data = Player::getRegisteredData(gpMarioAddress, "__better_sms");
    if (!data)
        return gpEgg->mWantedFruit == gpFruit->mObjectID ? EGG_SUCCESS : EGG_REJECT;

    auto *playerData = reinterpret_cast<Player::TPlayerData *>(data);

    if (gpEgg->mState == 14 || gpEgg->mState == 6)
        return EGG_IGNORE;
    else if (!playerData->getParams()->mCanRideYoshi.get())
        return EGG_REJECT;
    else if (!Stage::getStageConfiguration()->mIsEggFree.get())
        return gpEgg->mWantedFruit == gpFruit->mObjectID ? EGG_SUCCESS : EGG_REJECT;
    else
        return EGG_SUCCESS;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BC8B4, 0x801B476C, 0, 0), isYoshiEggFree);
SMS_WRITE_32(SMS_PORT_REGION(0x801BC8B8, 0x801B4770, 0, 0), 0xA01E00FC);
SMS_WRITE_32(SMS_PORT_REGION(0x801BC8BC, 0x801B4774, 0, 0), 0x2C00000B);
SMS_WRITE_32(SMS_PORT_REGION(0x801BC8C0, 0x801B4778, 0, 0), 0x418200E4);
SMS_WRITE_32(SMS_PORT_REGION(0x801BC8C4, 0x801B477C, 0, 0), 0x2C030001);
SMS_WRITE_32(SMS_PORT_REGION(0x801BC8C8, 0x801B4780, 0, 0), 0x4182003C);
SMS_WRITE_32(SMS_PORT_REGION(0x801BC8CC, 0x801B4784, 0, 0), 0x418100D8);
SMS_WRITE_32(SMS_PORT_REGION(0x801BC8D0, 0x801B4788, 0, 0), 0x48000134);

#undef EGG_IGNORE
#undef EGG_SUCCESS
#undef EGG_REJECT

#define EGG_CARD_EXCLAIM 11.0f

static void initFreeEggCard(J3DFrameCtrl *frameCtrl) {
    if (!Stage::getStageConfiguration()->mIsEggFree.get())
        return;

    frameCtrl->mCurFrame = EGG_CARD_EXCLAIM;
}
SMS_PATCH_B(SMS_PORT_REGION(0x801BC128, 0x801B3FE0, 0, 0), initFreeEggCard);

static bool updateFreeEggCard(J3DFrameCtrl *frameCtrl) {
    if (!Stage::getStageConfiguration()->mIsEggFree.get())
        return true;

    frameCtrl->mCurFrame = EGG_CARD_EXCLAIM;
    return true;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BC2A0, 0x801B4570, 0, 0), updateFreeEggCard);

#undef EGG_CARD_EXCLAIM

#endif