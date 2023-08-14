#include <Dolphin/string.h>

#include <SMS/Manager/FlagManager.hxx>
#include <SMS/MapObj/MapObjTree.hxx>
#include <SMS/macros.h>

#include "module.hxx"

#if BETTER_SMS_EXTRA_OBJECTS

// make tree leaf count dynamic, based on number of leaf col files
static TMapObjTree *getLeafCount(TMapObjTree *tree) {
    char cacheBuffer[128];
    char buffer[128];

    snprintf(cacheBuffer, 96, "/scene/MapObj/%sLeaf", tree->mRegisterName);
    strcat(cacheBuffer, "%02d.col");

    s32 num             = 0;
    const s32 maxLeaves = tree->mLeafCount;
    while (num < maxLeaves) {
        snprintf(buffer, 100, cacheBuffer, num + 1);
        if (!JKRFileLoader::getGlbResource(buffer)) {
            tree->mLeafCount = num;
            return tree;
        }
        num += 1;
    }
    return tree;
}
SMS_PATCH_B(SMS_PORT_REGION(0x801F6AE4, 0x801EE9BC, 0, 0), getLeafCount);
SMS_PATCH_B(SMS_PORT_REGION(0x801F6B20, 0x801EE9F8, 0, 0), getLeafCount);
SMS_PATCH_B(SMS_PORT_REGION(0x801F6B5C, 0x801EEA34, 0, 0), getLeafCount);
SMS_PATCH_B(SMS_PORT_REGION(0x801F6B98, 0x801EEA70, 0, 0), getLeafCount);
SMS_PATCH_B(SMS_PORT_REGION(0x801F6BD4, 0x801EEAAC, 0, 0), getLeafCount);

#endif