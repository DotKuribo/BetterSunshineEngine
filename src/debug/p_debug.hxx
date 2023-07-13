#pragma once

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

enum DebugState { NONE, XYZ_MODE, CAM_MODE };

constexpr auto gActivateMask  = TMarioGamePad::Z;
constexpr auto gSecondaryMask = TMarioGamePad::Z;

constexpr auto gControlToggleDebugActive = TMarioGamePad::DPAD_UP;    // Activate
constexpr auto gControlToggleDebugState  = TMarioGamePad::DPAD_UP;    // Secondary
constexpr auto gControlToggleDebugUI     = TMarioGamePad::DPAD_DOWN;  // Secondary
constexpr auto gControlToggleGameUI      = TMarioGamePad::DPAD_DOWN;

constexpr auto gControlToggleFastMovement = TMarioGamePad::START;
//constexpr auto gControlXYZMoveUp   = TMarioGamePad::A;
//constexpr auto gControlXYZMoveDown = TMarioGamePad::B;

constexpr auto gControlToggleCameraFrozen  = TMarioGamePad::A;
constexpr auto gControlToggleCameraTrack   = TMarioGamePad::X;
constexpr auto gControlIncreaseCameraFOV   = TMarioGamePad::DPAD_UP;
constexpr auto gControlDecreaseCameraFOV   = TMarioGamePad::DPAD_DOWN;
constexpr auto gControlResetCameraRotation = TMarioGamePad::Y;

constexpr auto gControlNozzleSwitchR          = TMarioGamePad::DPAD_RIGHT;
constexpr auto gControlNozzleSwitchL          = TMarioGamePad::DPAD_LEFT;
constexpr auto gControlIncreaseAnimationID    = TMarioGamePad::DPAD_RIGHT;
constexpr auto gControlDecreaseAnimationID    = TMarioGamePad::DPAD_LEFT;
constexpr auto gControlIncreaseAnimationSpeed = TMarioGamePad::DPAD_RIGHT;  // Secondary
constexpr auto gControlDecreaseAnimationSpeed = TMarioGamePad::DPAD_LEFT;   // Secondary

#define GetMarioYVelocity(controller) ((controller)->mButtons.mAnalogR - (controller)->mButtons.mAnalogL)

#define GetCameraPan(controller) ((controller)->mButtons.mAnalogR - (controller)->mButtons.mAnalogL)
#define GetCameraRoll(controller)                                                                  \
    ((controller)->mButtons.mAnalogR - (controller)->mButtons.mAnalogL)  // Secondary
#define GetCameraFOV(controller) ((controller)->mCStick.mStickY)  // Secondary

#define ButtonsFramePressed(controller, buttons)                                                   \
    (((controller->mButtons.mFrameInput) & (buttons)) == (buttons))
#define ButtonsPressed(controller, buttons)                                                        \
    (((controller->mButtons.mInput) & (buttons)) == (buttons))
#define ButtonsRapidPressed(controller, buttons)                                                   \
    (((controller->mButtons.mRapidInput) & (buttons)) == (buttons))
#define ButtonsFramePressedEx(controller, buttons) ((controller->mButtons.mFrameInput & 0x7FFF) == (buttons))
#define ButtonsPressedEx(controller, buttons)      ((controller->mButtons.mInput & 0x7FFF) == (buttons))
#define ButtonsRapidPressedEx(controller, buttons)                                                 \
    ((controller->mButtons.mRapidInput & 0x7FFF) == (buttons))

extern u32 XYZState;

extern TCheatHandler gDebugHandler;
extern int gDebugState;
extern int gDebugUIPage;
extern bool gIsFastMovement;
extern bool gIsDebugActive;
extern int gMarioAnimation;
extern bool gIsDebugActive;
extern int gAnimationID;
extern float gAnimationSpeed;
extern bool gIsCameraTracking;
extern bool gIsCameraFrozen;
extern TVec3f gCamPosition, gCamRotation;
extern float gCamFOV;