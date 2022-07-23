#include <SMS/actor/Mario.hxx>
#include <SMS/actor/Yoshi.hxx>

#include "common_sdk.h"
#include "module.hxx"

#if BETTER_SMS_GREEN_YOSHI

static void fixYoshiJuiceDecrement() {
    TYoshi *yoshi;
    SMS_FROM_GPR(30, yoshi);

    TMario *player = yoshi->mMario;
    if (player->mFludd->mIsEmitWater && yoshi->mState == TYoshi::MOUNTED)
        yoshi->mCurJuice -= player->mFludd->mEmitInfo->mNum.get();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8026E810, 0x8026659C, 0, 0), fixYoshiJuiceDecrement);

#endif