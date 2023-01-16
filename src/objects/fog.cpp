#include <Dolphin/GX.h>
#include <Dolphin/MTX.h>
#include <Dolphin/types.h>

#include <JSystem/J3D/J3DModel.hxx>
#include <JSystem/J3D/J3DModelLoaderDataBase.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/Enemy/Conductor.hxx>
#include <SMS/M3DUtil/MActor.hxx>
#include <SMS/M3DUtil/MActorKeeper.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Manager/ModelWaterManager.hxx>
#include <SMS/Map/BGCheck.hxx>
#include <SMS/MapObj/MapObjBall.hxx>
#include <SMS/MapObj/MapObjInit.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Player/Watergun.hxx>
#include <SMS/raw_fn.hxx>

#include "libs/constmath.hxx"
#include "objects/fog.hxx"

void TSimpleFog::load(JSUMemoryInputStream &in) {
    JDrama::TViewObj::load(in);

    in.readData(&mType, 4);
    in.readData(&mStartZ, 4);
    in.readData(&mEndZ, 4);
    in.readData(&mNearZ, 4);
    in.readData(&mFarZ, 4);
    in.readData(&mColor, 4);

    OSReport("augggah\n");
}

void TSimpleFog::perform(u32 flags, JDrama::TGraphics *graphics) {
    GXSetFog(mType, mStartZ, mEndZ, mNearZ, mFarZ, mColor);
    OSReport("augggah\n");
}