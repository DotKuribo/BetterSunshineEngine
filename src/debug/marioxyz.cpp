#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <SMS/MSound/MSound.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>

#include "debug.hxx"
#include "libs/cheathandler.hxx"
#include "libs/constmath.hxx"
#include "logging.hxx"
#include "module.hxx"
#include "raw_fn.hxx"

#include "p_debug.hxx"

void DebugMoveMarioXYZ(TMario *player) {
    constexpr f32 baseSpeed             = 15.0f;
    const JUTGamePad::CStick &mainStick = player->mController->mControlStick;
    const f32 speedMultiplier           = baseSpeed * (gIsFastMovement ? 3 : 1);

    const f32 cameraRotY = (f32)(gpCamera->mHorizontalAngle) / 182.0f;

    player->mTranslation.x +=
        (-sinf(angleToRadians(cameraRotY)) * speedMultiplier) * mainStick.mStickY;
    player->mTranslation.z +=
        (-cosf(angleToRadians(cameraRotY)) * speedMultiplier) * mainStick.mStickY;
    player->mTranslation.x -=
        (-sinf(angleToRadians(cameraRotY + 90.0f)) * speedMultiplier) *
        mainStick.mStickX;
    player->mTranslation.z -=
        (-cosf(angleToRadians(cameraRotY + 90.0f)) * speedMultiplier) *
        mainStick.mStickX;

    player->mTranslation.y += GetMarioYVelocity(player->mController) * speedMultiplier;
}