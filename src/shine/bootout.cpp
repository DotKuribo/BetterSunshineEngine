#include <Dolphin/string.h>
#include <JSystem/JGeometry.hxx>
#include <SMS/SMS.hxx>
#include <SMS/actor/item/Shine.hxx>
#include <SMS/game/Conductor.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/sound/MSBGM.hxx>

#include "music.hxx"

#include "module.hxx"

using namespace BetterSMS;

// 0x801BD664
// extern -> SME.cpp
static bool sIsShineShrinking = false;
static void manageShineVanish(TVec3f *marioPos) {
    TShine *shine;
    SMS_FROM_GPR(30, shine);

    const TVec3f step(0.007f, 0.007f, 0.007f);

    TVec3f size;
    TVec3f rotation;
    TVec3f position;

    shine->JSGGetScaling(reinterpret_cast<Vec *>(&size));
    shine->JSGGetRotation(reinterpret_cast<Vec *>(&rotation));
    shine->JSGGetTranslation(reinterpret_cast<Vec *>(&position));

    if (size.x - 0.011f <= 0) {
        rotation.y = 1.0f;
        size.set(1.0f, 1.0f, 1.0f);
        shine->JSGSetScaling(reinterpret_cast<Vec &>(size));
        shine->JSGSetRotation(reinterpret_cast<Vec &>(size));
        shine->mGlowSize.set(1.0f, 1.0f, 1.0f);
        setAnmFromIndex__12MActorAnmBckFiPUs(shine->mActorData->mBckInfo, -1, nullptr);
        // shine->mActorData->mBckInfo->setAnmFromIndex(-1, nullptr);
        shine->kill();
    } else if (gpMarioAddress->mState != static_cast<u32>(TMario::STATE_SHINE_C)) {
        if (!sIsShineShrinking) {
            shine->mGlowSize.set(1.0f, 1.0f, 1.0f);

            size.set(1.0f, 1.0f, 1.0f);
            shine->JSGSetScaling(reinterpret_cast<Vec &>(size));

            sIsShineShrinking = true;
        }
        rotation.y += 3.0f;
        position.y += 4.0f;
        size.sub(step);
        shine->JSGSetScaling(reinterpret_cast<Vec &>(size));
        shine->JSGSetRotation(reinterpret_cast<Vec &>(rotation));
        shine->JSGSetTranslation(reinterpret_cast<Vec &>(position));
        shine->mGlowSize.sub(step);
    } else {
        sIsShineShrinking = false;
        shine->JSGSetTranslation(reinterpret_cast<Vec &>(*marioPos));
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BD664, 0x801B551C, 0, 0), manageShineVanish);
SMS_WRITE_32(SMS_PORT_REGION(0x801BD668, 0x801B5520, 0, 0), 0x48000568);

// 0x802413E0
static void isKillEnemiesShine(TConductor *gpConductor, TVec3f *playerCoordinates, f32 range) {
    TMario *player;
    SMS_FROM_GPR(31, player);

    TShine *shine = (TShine *)player->mGrabTarget;
    if (!(shine->mType & 0x10))
        killEnemiesWithin__10TConductorFRCQ29JGeometry8TVec3_f(gpConductor, playerCoordinates,
                                                               range);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802413E0, 0x8023916C, 0, 0), isKillEnemiesShine);

static void restoreMario(TMarDirector *marDirector, u32 curState) {
    TShine *shine = marDirector->mCollectedShine;

    if (!shine || !(shine->mType & 0x10) || !marDirector->mpNextState)
        return;

    u8 *curSaveCard = reinterpret_cast<u8 *>(marDirector->mpNextState[0x118 / 4]);

    if (curState != TMarDirector::Status::NORMAL ||
        marDirector->mCurState != TMarDirector::Status::SAVE_CARD ||
        gpMarioAddress->mState != static_cast<u32>(TMario::STATE_SHINE_C))
        return;

    if (curSaveCard[0x2E9] != 1) {
        if (SMS_isDivingMap__Fv() || (gpMarioAddress->mPrevState & 0x20D0) == 0x20D0)
            gpMarioAddress->mState = gpMarioAddress->mPrevState;
        else
            gpMarioAddress->mState = static_cast<u32>(TMario::STATE_IDLE);

        gpCamera->endDemoCamera();
        marDirector->mCollectedShine = nullptr;

        Music::AudioStreamer *streamer = Music::getAudioStreamer();
        if (streamer->isPaused())
            streamer->play();
    } else
        marDirector->mGameState |= TMarDirector::State::WARP_OUT;
}

static void checkBootOut(TMarDirector *marDirector, u8 curState) {
    restoreMario(marDirector, curState);
    marDirector->currentStateFinalize(curState);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802995BC, 0x80291454, 0, 0), checkBootOut);

static void shineObjectStringMod(JSUInputStream *stream, u8 *dst, u32 size) {
    TShine *shine;
    SMS_FROM_GPR(30, shine);

    if (shine->mType == 1) {
        if (strcmp("nbnorm", (const char *)(dst + 4)) == 0)
            shine->mType = 0x10;
        else if (strcmp("nbquik", (const char *)(dst + 4)) == 0)
            shine->mType = 0x12;
        else if (strcmp("nbdemo", (const char *)(dst + 4)) == 0)
            shine->mType = 0x11;
    }
    stream->read(dst, size);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BCC98, 0x801B4B50, 0, 0), shineObjectStringMod);

static void thinkSetBootFlag(TShineFader *shineFader, u32 unk_1, u32 unk_2) {
    TMarDirector *gpMarDirector;
    SMS_FROM_GPR(31, gpMarDirector);

    if (!(gpMarDirector->mCollectedShine->mType & 0x10)) {
        for (s32 i = 0; i < 3; ++i) {
            if ((3 >> i) == 0)
                break;

            MSBgm::stopTrackBGM(i, 0);
        }
        shineFader->registFadeout(unk_1, unk_2);
        gpMarDirector->mGameState |= TMarDirector::State::WARP_OUT;
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80297BD8, 0x8028FA70, 0, 0), thinkSetBootFlag);

static void thinkSetNextSequence(TGameSequence *sequence, u8 area, u8 episode,
                                 JDrama::TFlagT<u16> flag) {
    if (!(gpMarDirector->mCollectedShine->mType & 0x10)) {
#if SMS_DEMO
        area    = 1;
        episode = 5;
#endif
        sequence->set(area, episode, flag);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80297C84, 0x8028FB1C, 0, 0), thinkSetNextSequence);

static u32 loadAfterMaskState() {
    TShine *shine;
    SMS_FROM_GPR(31, shine);

    return shine->mType & 0xF;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BCD20, 0x801B4BD8, 0, 0), loadAfterMaskState);

static void setKillState() {
    TShine *shine;
    SMS_FROM_GPR(31, shine);

    shine->mType = (shine->mType & 0x10) | 1;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BCEEC, 0x801B4DA4, 0, 0), setKillState);

static SMS_ASM_FUNC void thinkCloseCamera() {
#if defined(PAL)
    SMS_ASM_BLOCK("lbz       0, 0x190 (31)        \n\t"
                  "lwz       4, 0x154 (31)        \n\t"
                  "rlwinm.   4, 4, 0, 27, 27     \n\t"
                  "bne       .Ltmp0              \n\t"
                  "li        0, 0                \n\t"

                  ".Ltmp0:                       \n\t"
                  "blr                           \n\t");
#else
    SMS_ASM_BLOCK("lbz       0, 0x190 (4)        \n\t"
                  "lwz       4, 0x154 (4)        \n\t"
                  "rlwinm.   4, 4, 0, 27, 27     \n\t"
                  "bne       .Ltmp0              \n\t"
                  "li        0, 0                \n\t"

                  ".Ltmp0:                       \n\t"
                  "blr                           \n\t");
#endif
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8029A590, 0x80292460, 0, 0), thinkCloseCamera);
SMS_WRITE_32(SMS_PORT_REGION(0x8029A594, 0x80292464, 0, 0), 0x28000000);

static SMS_ASM_FUNC void animationFreezeCheck() {
    SMS_ASM_BLOCK("lbz       0, 0x64(26)         \n\t"
                  "cmpwi     0, 10               \n\t"
                  "beq-      .loc_0x38           \n\t"
                  "bge-      .loc_0x18           \n\t"
                  "cmpwi     0, 5                \n\t"
                  "bne-      .loc_0x3C           \n\t"

                  ".loc_0x18:                    \n\t"
                  "cmpwi     0, 13               \n\t"
                  "bge-      .loc_0x3C           \n\t"
                  "lis 3,    gpMarioAddress@ha   \n\t"
                  "lwz 3,    gpMarioAddress@l (3)\n\t"
                  "lwz       3, 0x7C(3)          \n\t"
                  "cmpwi     3, 0x1302           \n\t"
                  "bne-      .loc_0x38           \n\t"
                  "cmpwi     0, 11               \n\t"
                  "beq-      .loc_0x3C           \n\t"

                  ".loc_0x38:                    \n\t"
                  "ori       27, 27, 0x3         \n\t"

                  ".loc_0x3C:                    \n\t"
                  "blr                           \n\t");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802999D8, 0x80291870, 0, 0), animationFreezeCheck);
SMS_WRITE_32(SMS_PORT_REGION(0x802999DC, 0x80291874, 0, 0), 0x48000034);

// Remove auto disable sound
SMS_WRITE_32(SMS_PORT_REGION(0x800169B0, 0x80016A0C, 0, 0), 0x60000000);

SMS_WRITE_32(SMS_PORT_REGION(0x80297BE8, 0x8028FA80, 0, 0), 0x60848200);