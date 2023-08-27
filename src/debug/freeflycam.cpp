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
#include "libs/geometry.hxx"
#include "logging.hxx"
#include "module.hxx"
#include "raw_fn.hxx"

#include "p_debug.hxx"

bool gIsCameraTracking = false;
bool gIsCameraFrozen   = false;

TVec3f gCamPosition = {0, 0, 0}, gCamRotation = {0, 0, 0};
float gCamFOV = 70.0f;

using namespace BetterSMS::Geometry;

void DebugFreeFlyCamera(TMario *player, CPolarSubCamera *camera) {
    constexpr f32 baseSpeed = 15.0f;

    const JUTGamePad::CStick &mainStick = player->mController->mControlStick;
    const JUTGamePad::CStick &cStick    = player->mController->mCStick;

    const float cameraPan  = GetCameraPan(player->mController);
    const float cameraRoll = GetCameraRoll(player->mController);

    const bool shouldToggleFrozen =
        ButtonsFramePressed(player->mController, gControlToggleCameraFrozen);

    const bool shouldToggleTrack =
        ButtonsFramePressed(player->mController, gControlToggleCameraTrack);

    const bool shouldResetRotation =
        ButtonsFramePressed(player->mController, gControlResetCameraRotation);

    const float cameraFOV = GetCameraFOV(player->mController);

    const bool isSecondaryControl = ButtonsPressed(player->mController, TMarioGamePad::Z);

    const f32 speedMultiplier = gIsFastMovement ? baseSpeed * 3 : baseSpeed;

    if (shouldToggleTrack) {
        gIsCameraTracking ^= true;
    }

    if (shouldToggleFrozen)
        gIsCameraFrozen ^= true;

    if (isSecondaryControl) {
        gCamFOV += cameraFOV;
        camera->mProjectionFovy = gCamFOV;
    }

    // apply position here
    f32 dx = mainStick.mStickX * speedMultiplier;
    f32 dy = cStick.mStickY * speedMultiplier;
    f32 dz = mainStick.mStickY * speedMultiplier;

    // convert cam rotation to radians if in degrees
    f32 camRotPitchInRad = angleToRadians(gCamRotation.x);
    f32 camRotYawInRad   = angleToRadians(gCamRotation.y);
    f32 camRotRollInRad  = angleToRadians(gCamRotation.z);

    TVec3f forwardVec = {cosf(camRotPitchInRad) * sinf(camRotYawInRad), sinf(camRotPitchInRad),
                         cosf(camRotPitchInRad) * cosf(camRotYawInRad)};

    TVec3f rightVec;
    TVec3f upVec = TVec3f::up();
    PSVECCrossProduct(upVec, forwardVec, rightVec);
    PSVECNormalize(rightVec, rightVec);

    // Recalculate upVec as the cross product of forwardVec and rightVec
    PSVECCrossProduct(forwardVec, rightVec, upVec);
    PSVECNormalize(upVec, upVec);

    // Apply the roll
    TVec3f localRightVec = TVec3f::zero();
    TVec3f localUpVec    = TVec3f::zero();
    {
        TVec3f cRF = rightVec;
        TVec3f sRF = rightVec;
        TVec3f cUF = upVec;
        TVec3f sUF = upVec;

        cRF.scale(cosf(camRotRollInRad));
        sRF.scale(sinf(camRotRollInRad));
        cUF.scale(cosf(camRotRollInRad));
        sUF.scale(sinf(camRotRollInRad));

        localRightVec += cRF;
        localRightVec -= sUF;
        localUpVec += cUF;
        localUpVec += sRF;
    }

    // Convert stick input to local camera space
    TVec3f scaledFVec = forwardVec;
    scaledFVec.scale(dz);

    TVec3f scaledRVec = localRightVec;
    scaledRVec.scale(dx);

    // rotation applied here
    if (!gIsCameraTracking) {
        if (shouldResetRotation) {
            gCamRotation = {0, 0, 0};
        } else {
            if (!isSecondaryControl) {
                f32 yawChangeX =
                    cStick.mStickX * cosf(camRotRollInRad) - cStick.mStickY * sinf(camRotRollInRad);
                f32 yawChangeY =
                    cStick.mStickX * sinf(camRotRollInRad) + cStick.mStickY * cosf(camRotRollInRad);

                gCamRotation.y -= yawChangeX;
                gCamRotation.x += yawChangeY;
            } else {
                gCamRotation.z -= cameraRoll;
            }
        }
    }

    gCamRotation.x = clamp<f32>(gCamRotation.x, -89.9f, 89.9f);

    TVec3f localTranslation = TVec3f::zero();
    localTranslation += scaledFVec;
    localTranslation -= scaledRVec;

    gCamPosition += localTranslation;

    if (!isSecondaryControl) {
        TVec3f scaledUVec = localUpVec;
        scaledUVec.scale(cameraPan * speedMultiplier);
        gCamPosition += scaledUVec;  // No change required for Y
    }

    camera->mTranslation      = gCamPosition;
    camera->mWorldTranslation = gCamPosition;

    C_MTXPerspective(camera->mProjectionMatrix, camera->mProjectionFovy, camera->mProjectionAspect,
                     camera->mProjectionNear, camera->mProjectionFar);

    if (!gIsCameraTracking) {
        C_MTXLookAt(camera->mTRSMatrix, gCamPosition, localUpVec, forwardVec + gCamPosition);
    } else {
        C_MTXLookAt(camera->mTRSMatrix, gCamPosition, upVec, *gpMarioPos);
        gCamRotation = Vector3::eulerFromMatrix(gpCamera->mTRSMatrix);
    }
}

 static bool shouldDebugPassThroughCameraPerform() {
     JDrama::TGraphics *graphics;
     SMS_FROM_GPR(31, graphics);

     if (gDebugState == DebugState::CAM_MODE && gIsDebugActive)
         return true;
     else if (gIsCameraFrozen)
         return true;

     return (graphics->_00[1] & 1);
 }
 SMS_PATCH_BL(SMS_PORT_REGION(0x80023034, 0, 0, 0), shouldDebugPassThroughCameraPerform);
 SMS_WRITE_32(SMS_PORT_REGION(0x80023038, 0, 0, 0), 0x2C030000);

 static bool shouldDebugControlCameraPerform(const Mtx a, Mtx b) {
     CPolarSubCamera *camera;
     SMS_FROM_GPR(29, camera);

     PSMTXCopy(a, b);

     if (gDebugState == DebugState::CAM_MODE && gIsDebugActive)
         return true;
     else if (gIsCameraFrozen)
         return true;

     camera->mUpVector = {0, 1, 0};

     return false;
 }
 SMS_PATCH_BL(SMS_PORT_REGION(0x800230F8, 0, 0, 0), shouldDebugControlCameraPerform);
 SMS_WRITE_32(SMS_PORT_REGION(0x800230FC, 0, 0, 0), 0x2C030001);
 SMS_WRITE_32(SMS_PORT_REGION(0x80023100, 0, 0, 0), 0x418201F8);
 SMS_WRITE_32(SMS_PORT_REGION(0x80023108, 0, 0, 0), 0x60000000);
 SMS_WRITE_32(SMS_PORT_REGION(0x8002310C, 0, 0, 0), 0x60000000);
 SMS_WRITE_32(SMS_PORT_REGION(0x80023110, 0, 0, 0), 0x60000000);
 SMS_WRITE_32(SMS_PORT_REGION(0x80023114, 0, 0, 0), 0x60000000);
 SMS_WRITE_32(SMS_PORT_REGION(0x80023118, 0, 0, 0), 0x60000000);

 static void shouldCtrlCamera(CPolarSubCamera *camera) {
     if (gDebugState != DebugState::CAM_MODE)
         camera->ctrlNormalOrTowerCamera_();
 }
 SMS_PATCH_BL(SMS_PORT_REGION(0x800238fc, 0, 0, 0), shouldCtrlCamera);

 static bool isSimpleCamera(u32 r3, bool isSimple) {
     if (gDebugState != DebugState::CAM_MODE)
         return isSimple;
     return false;
 }
 SMS_PATCH_B(SMS_PORT_REGION(0x80032934, 0, 0, 0), isSimpleCamera);