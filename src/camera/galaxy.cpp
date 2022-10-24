#include <Dolphin/MTX.h>
#include <Dolphin/types.h>


#include <SMS/Camera/CamConnector.hxx>
#include <SMS/Camera/CamSaveKindParam.hxx>
#include <SMS/Camera/CameraKindParam.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>


#include "module.hxx"

static void scaleFOV(TCameraKindParam *data, const TCamSaveKindParam &params) {
    data->copySaveParam(params);

    data->mSLFovy *= 0.8f;
    data->mSLDistMin *= 2.0f;
    data->mSLDistMax *= 1.25f;
    data->mSLXAngleMin *= 1.05f;
    data->mSLXAngleMax *= 0.6f;
    data->mSLHoldOffsetAngleXMin *= 1.15f;
    data->mSLHoldOffsetAngleXMax *= 0.75f;
}
//SMS_PATCH_BL(SMS_PORT_REGION(0x8002374C, 0, 0, 0), scaleFOV);