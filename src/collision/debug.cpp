#include <Dolphin/types.h>
#include <SMS/Player/Mario.hxx>
#include <SMS/Player/MarioGamePad.hxx>
#include <SMS/collision/BGCheck.hxx>

#include "p_globals.hxx"
#include "module.hxx"

static u8 sHomeID   = 0;
static u8 sTargetID = 254;
static TBGCheckData *sHomeTriangle;

void updateDebugCollision(TMario *player, bool isMario) {
    if (!isMario)
        return;

    constexpr u32 SetHomeTriangleButtons   = TMarioGamePad::EButtons::DPAD_LEFT;
    constexpr u32 SetTargetTriangleButtons = TMarioGamePad::EButtons::DPAD_RIGHT;

    if (!BetterSMS::isDebugMode())
        return;

    const JUTGamePad::CButton &buttons = player->mController->mButtons;

    if ((buttons.mInput & TMarioGamePad::EButtons::Z) == 0)
        return;

    if ((buttons.mFrameInput & SetHomeTriangleButtons)) {
        sHomeTriangle = const_cast<TBGCheckData *>(player->mFloorTriangle);
        sHomeID       = (sHomeID + 1) % 255;
    } else if ((buttons.mFrameInput & SetTargetTriangleButtons) && sHomeTriangle) {
        if (sHomeTriangle == player->mFloorTriangle)
            return;

        TBGCheckData *currentTri = const_cast<TBGCheckData *>(player->mFloorTriangle);

        sHomeTriangle->mCollisionType = 3061;
        sHomeTriangle->mValue4        = s16((sTargetID << 8) | (sHomeID - 1));
        currentTri->mCollisionType    = 3061;
        currentTri->mValue4           = s16(((sHomeID - 1) << 8) | sTargetID);
        BetterSMS::sWarpColArray->addLink(sHomeTriangle, player->mFloorTriangle);
        BetterSMS::sWarpColArray->addLink(player->mFloorTriangle, sHomeTriangle);
        sTargetID = sTargetID != 0 ? (sTargetID - 1) : 254;
    }

    return;
}