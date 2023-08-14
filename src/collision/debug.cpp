#include <Dolphin/types.h>
#include <SMS/Map/BGCheck.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Player/MarioGamePad.hxx>

#include "module.hxx"
#include "p_globals.hxx"

static u8 sHomeID   = 0;
static u8 sTargetID = 254;
static TBGCheckData *sHomeTriangle;

#if 0
BETTER_SMS_FOR_CALLBACK void updateDebugCollision(TMario *player, bool isMario) {
    if (!isMario)
        return;

    constexpr u32 SetHomeTriangleButtons   = TMarioGamePad::DPAD_LEFT;
    constexpr u32 SetTargetTriangleButtons = TMarioGamePad::DPAD_RIGHT;

    if (!BetterSMS::isDebugMode())
        return;

    const JUTGamePad::CButton &buttons = player->mController->mButtons;

    if ((buttons.mInput & TMarioGamePad::Z) == 0)
        return;

    if ((buttons.mFrameInput & SetHomeTriangleButtons)) {
        sHomeTriangle = const_cast<TBGCheckData *>(player->mFloorTriangle);
        sHomeID       = (sHomeID + 1) % 255;
    } else if ((buttons.mFrameInput & SetTargetTriangleButtons) && sHomeTriangle) {
        if (sHomeTriangle == player->mFloorTriangle)
            return;

        TBGCheckData *currentTri = const_cast<TBGCheckData *>(player->mFloorTriangle);

        sHomeTriangle->mType = 3061;
        sHomeTriangle->mValue        = s16((sTargetID << 8) | (sHomeID - 1));
        currentTri->mType    = 3061;
        currentTri->mValue           = s16(((sHomeID - 1) << 8) | sTargetID);
        BetterSMS::sWarpColArray->addLink(sHomeTriangle, player->mFloorTriangle);
        BetterSMS::sWarpColArray->addLink(player->mFloorTriangle, sHomeTriangle);
        sTargetID = sTargetID != 0 ? (sTargetID - 1) : 254;
    }

    return;
}
#endif