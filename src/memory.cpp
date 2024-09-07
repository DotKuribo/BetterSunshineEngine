#include <Dolphin/OS.h>
#include <Dolphin/mem.h>
#include <Dolphin/types.h>
#include <JSystem/JKernel/JKRHeap.hxx>
#include <SMS/macros.h>

#include "memory.hxx"
#include "module.hxx"

BETTER_SMS_FOR_EXPORT void BetterSMS::Cache::flush(void *addr, size_t size) {
    DCFlushRange(addr, size);
    ICInvalidateRange(addr, size);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Cache::flash() { ICFlashInvalidate(); }

BETTER_SMS_FOR_EXPORT void BetterSMS::Cache::store(void *addr, size_t size) {
    DCStoreRange(addr, size);
    ICInvalidateRange(addr, size);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Cache::zero(void *addr, size_t size) {
    DCZeroRange(addr, size);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Cache::enable() {
    DCEnable();
    ICEnable();
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Cache::disable() {
    DCDisable();
    ICDisable();
}

BETTER_SMS_FOR_EXPORT void *BetterSMS::Memory::malloc(const size_t size, const size_t alignment) {
    return JKRHeap::sCurrentHeap->alloc(size, alignment);
}

BETTER_SMS_FOR_EXPORT void *BetterSMS::Memory::hmalloc(JKRHeap *heap, const size_t size,
                                                       const size_t alignment) {
    return JKRHeap::alloc(size, alignment, heap);
}

BETTER_SMS_FOR_EXPORT void *BetterSMS::Memory::calloc(const size_t size, const size_t alignment) {
    void *obj = malloc(size, alignment);
    memset(obj, 0, size);
    return obj;
}

BETTER_SMS_FOR_EXPORT void *BetterSMS::Memory::hcalloc(JKRHeap *heap, const size_t size,
                                                       const size_t alignment) {
    void *obj = hmalloc(heap, size, alignment);
    memset(obj, 0, size);
    return obj;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Memory::free(const void *ptr) { delete (u8 *)ptr; }

BETTER_SMS_FOR_EXPORT u32 *BetterSMS::PowerPC::getBranchDest(u32 *bAddr) {
    s32 offset;
    u32 instr = *bAddr;

    offset = (instr & 0x3FFFFFD) - ((instr & 0x2000000) << 1);
    return static_cast<u32 *>(bAddr + (offset / 4));
}

BETTER_SMS_FOR_EXPORT void BetterSMS::PowerPC::writeU8(u8 *ptr, u8 value) {
    *ptr = value;
    BetterSMS::Cache::store(ptr, sizeof(u8));
}

BETTER_SMS_FOR_EXPORT void BetterSMS::PowerPC::writeU16(u16 *ptr, u16 value) {
    *ptr = value;
    BetterSMS::Cache::store(ptr, sizeof(u16));
}

BETTER_SMS_FOR_EXPORT void BetterSMS::PowerPC::writeU32(u32 *ptr, u32 value) {
    *ptr = value;
    BetterSMS::Cache::store(ptr, sizeof(u32));
}
