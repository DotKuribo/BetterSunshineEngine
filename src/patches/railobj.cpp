#include <JSystem/J3D/J3DTransform.hxx>

#include <SMS/MapObj/MapObjRailBlock.hxx>
#include <SMS/macros.h>
#include "module.hxx"

static void checkResetCollisionMtx(TGraphTracer *tracer, int index) {
    TRailBlock *block;
    SMS_FROM_GPR(30, block);

    tracer->setTo(index);

    block->calcRootMatrix();
    block->mActorData->calc();
    block->mActorData->viewCalc();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801F12A4, 0, 0, 0), checkResetCollisionMtx);