#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <JSystem/J2D/J2DPrint.hxx>
#include <JSystem/JKernel/JKRFileLoader.hxx>
#include <SMS/GC2D/ConsoleStr.hxx>
#include <SMS/actor/Mario.hxx>
#include <SMS/mapobj/MapObjNormalLift.hxx>
#include <SMS/mapobj/MapObjTree.hxx>

#include <SMS/actor/HitActor.hxx>
#include <SMS/actor/LiveActor.hxx>
#include <SMS/macros.h>
#include <SMS/option/CardManager.hxx>
#include <SMS/raw_fn.hxx>

#include "common_sdk.h"
#include "module.hxx"

#if BETTER_SMS_EXTRA_OBJECTS

void checkInstantReset_NormalLift(u32 *railflags) {
    s16 *mRailObj;
    SMS_FROM_GPR(31, mRailObj);

    u32 flag = railflags[2];
    if (flag & 0x2000) {
        mRailObj[0x14A / 2] = 0;
    } else {
        mRailObj[0x14A / 2] = 180;
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801F0B90, 0x801E8A68, 0, 0), checkInstantReset_NormalLift);

s16 *checkInstantReset_RailObj(s16 *railObj, u32 *railflags) {
    u32 flag = railflags[2];
    if (flag & 0x2000) {
        railObj[0x14A / 2] = 0;
    } else {
        railObj[0x14A / 2] = 180;
    }
    return railObj;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801F1054, 0x801E8F2C, 0, 0), checkInstantReset_RailObj);

void checkResetToNode(TNormalLift *lift) {
    TGraphWeb *graph = lift->mGraphTracer->mGraph;
    TRailNode *node;
    {
        s32 nodeIdx = lift->mGraphTracer->mPreviousNode;
        node        = reinterpret_cast<TRailNode *>(graph->mNodes[nodeIdx << 2]);
    }
    if (node->mFlags & 0x2000) {
        lift->mPosition.set(graph->getNearestPosOnGraphLink(lift->mInitialPosition));
        lift->mRotation.set(lift->mInitialRotation);
        lift->mRailStatus     = 0;
        lift->mContextTimer   = 180;
        lift->mLastRailStatus = 1;
        {
            int idx = graph->findNearestNodeIndex(lift->mPosition, 0xFFFFFFFF);
            lift->mGraphTracer->setTo(idx);
        }
        lift->readRailFlag();
    } else {
        lift->resetPosition();
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801EFBDC, 0x801E7AB4, 0, 0), checkResetToNode);
SMS_WRITE_32(SMS_PORT_REGION(0x801EFBE0, 0x801E7AB8, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801EFBE4, 0x801E7ABC, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801EFBE8, 0x801E7AC0, 0, 0), 0x60000000);
SMS_PATCH_BL(SMS_PORT_REGION(0x801F13FC, 0x801E92D4, 0, 0), checkResetToNode);
SMS_WRITE_32(SMS_PORT_REGION(0x801F1400, 0x801E92D8, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801F1404, 0x801E92DC, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801F1408, 0x801E92E0, 0, 0), 0x60000000);

#endif