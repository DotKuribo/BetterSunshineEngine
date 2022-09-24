#include <Dolphin/GX.h>
#include <Dolphin/OS.h>

#include <SMS/collision/MapCollisionData.hxx>
#include <SMS/macros.h>

#include "collision/warp.hxx"
#include "common_sdk.h"
#include "globals.hxx"
#include "logging.hxx"
#include "math.hxx"
#include "module.hxx"

/*This works by taking the target id and matching it to the
/ ID of the first entry to have the same home ID as the target.
/
/ This makes a half decent linking system for the collision
/ triangles for functionalities like linked warping!
*/

#if BETTER_SMS_EXTRA_COLLISION

using namespace BetterSMS;
using namespace BetterSMS::Collision;

static void parseWarpLinks(TMapCollisionData *col, TWarpCollisionList *links, u32 validID,
                           u32 idGroupSize = 0) {
    u32 curDataIndex = 0;

    for (u32 i = 0; i < col->mFloorArraySize; ++i) {
        if (TCollisionLink::isValidWarpCol(&col->mColTable[i])) {

            TCollisionLink link(&col->mColTable[i], (u8)(col->mColTable[i].mValue4 >> 8),
                                (u8)col->mColTable[i].mValue4,
                                TCollisionLink::getSearchModeFrom(&col->mColTable[i]));

            links->addLink(link);
            if (curDataIndex > 0xFF)
                break;
        }
    }
}

// 0x802B8B20
static u32 initCollisionWarpLinks(const char *name) {
    Console::debugLog("Initializing warp collision data...\n");
    TWarpCollisionList *warpDataArray         = new TWarpCollisionList(2048);
    TWarpCollisionList *warpDataPreserveArray = new TWarpCollisionList(1);
    BetterSMS::sWarpColArray                  = warpDataArray;
    BetterSMS::sWarpColPreserveArray          = warpDataPreserveArray;

    if (warpDataArray) {
        parseWarpLinks(gpMapCollisionData, warpDataArray, 16040, 4);
    }

    return JDrama::TNameRef::calcKeyCode(name);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802B8B20, 0x802B0AF0, 0, 0), initCollisionWarpLinks);

#endif