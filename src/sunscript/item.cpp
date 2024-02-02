#include <SMS/Manager/ItemManager.hxx>
#include <SMS/MoveBG/Coin.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/MoveBG/Shine.hxx>

#include "p_sunscript.hxx"

void Spc::spawnObjByID(TSpcInterp *interp, u32 argc) {
    const char *name = reinterpret_cast<const char *>(Stack::popItem(interp).mValue);
    const u32 id     = Stack::popItem(interp).mValue;
    TMapObjBase *obj = TItemManager::newAndRegisterObjByEventID(id, name);
    Stack::pushItem(interp, reinterpret_cast<u32>(obj), ValueType::INT);
    if (!obj) {
        SpcTrace("Item could not be registered. ID(%lu)", id);
        return;
    }
}