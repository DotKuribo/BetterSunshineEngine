#include "module.hxx"

#include <JDrama/JDRViewObjPtrListT.hxx>

//"/scene/mapObj/mon_bri_rope.bti" Swingboard - All 3
SMS_WRITE_32(SMS_PORT_REGION(0x801b7628, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801b760c, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801b7660, 0, 0, 0), 0x60000000);

//"/scene/mapObj/cogwheel_rope.bti"
SMS_WRITE_32(SMS_PORT_REGION(0x801b7644, 0, 0, 0), 0x60000000);

// Credits to theAzack9
static void deduplicateObjectsInPerformList(
    TPerformList *that, JDrama::TViewObjPtrListT<JDrama::TViewObj, JDrama::TViewObj> *mapObj,
    u32 flags) {

    auto beginIter = mapObj->mViewObjList.begin();
    auto endIter   = mapObj->mViewObjList.end();
    JGadget::TList<JDrama::TViewObj *>::iterator iter =
        *reinterpret_cast<JGadget::TList<JDrama::TViewObj *>::iterator *>(&beginIter);
    JGadget::TList<JDrama::TViewObj *>::iterator end =
        *reinterpret_cast<JGadget::TList<JDrama::TViewObj *>::iterator *>(&endIter);
    auto *mapGroup = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Map group objs");
    while (iter != end) {

        if (strcmp((const char *)((u32)(*iter) - 12), "MapObjBase") != 0 &&
            strcmp((const char *)((u32)(*iter) - 12), "RiccoSwitch") != 0 &&
            strcmp((const char *)((u32)(*iter) - 16), "PinnaCoaster") != 0) {
            mapGroup->mViewObjList.insert(mapGroup->mViewObjList.end(), *iter);
        }
        iter++;
    }
    that->push_back(mapGroup, flags);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8029c2fc, 0, 0, 0), deduplicateObjectsInPerformList);