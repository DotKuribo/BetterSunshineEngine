#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <SMS/Camera/CamConnector.hxx>
#include <SMS/Camera/CamSaveKindParam.hxx>
#include <SMS/Camera/CameraKindParam.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>

#include "module.hxx"
#include "p_settings.hxx"

extern ViewportSetting gViewportSetting;

static void applySpecialCamera(TCameraKindParam *data, const TCamSaveKindParam &params) {
    data->copySaveParam(params);

    switch (gViewportSetting.getInt()) {
    default:
    case ViewportSetting::SMS:
        break;
    case ViewportSetting::SM64:
        data->mSLFovy *= 0.9f;
        data->mSLDistMin *= 1.4f;
        data->mSLDistMax *= 0.68f;
        data->mSLXAngleMin *= 0.78f;
        data->mSLXAngleMax *= 0.78f;
        data->mSLHoldOffsetAngleXMin *= 0.75f;
        data->mSLHoldOffsetAngleXMax *= 0.9f;
        data->mSLAtOffsetY *= 0.9f;
        data->mSLXRotRatioAtOffsetY *= 0.05f;
        break;
    case ViewportSetting::SMG:
        data->mSLFovy *= 0.8f;
        data->mSLDistMin *= 2.0f;
        data->mSLDistMax *= 1.25f;
        data->mSLXAngleMin *= 1.05f;
        data->mSLXAngleMax *= 0.6f;
        data->mSLHoldOffsetAngleXMin *= 1.15f;
        data->mSLHoldOffsetAngleXMax *= 0.75f;
        break;
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8002374C, 0, 0, 0), applySpecialCamera);