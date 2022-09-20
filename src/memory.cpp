#include <Dolphin/OS.h>
#include <Dolphin/mem.h>
#include <Dolphin/types.h>
#include <JSystem/JKernel/JKRHeap.hxx>
#include <SMS/macros.h>

#include "memory.hxx"

SMS_NO_INLINE void BetterSMS::Cache::flush(void *addr, size_t size) {
    DCFlushRange(addr, size);
    ICInvalidateRange(addr, size);
}

SMS_NO_INLINE void BetterSMS::Cache::flash() { ICFlashInvalidate(); }

SMS_NO_INLINE void BetterSMS::Cache::store(void *addr, size_t size) {
    DCStoreRange(addr, size);
    ICInvalidateRange(addr, size);
}

SMS_NO_INLINE void BetterSMS::Cache::zero(void *addr, size_t size) { DCZeroRange(addr, size); }

SMS_NO_INLINE void BetterSMS::Cache::enable() {
    DCEnable();
    ICEnable();
}

SMS_NO_INLINE void BetterSMS::Cache::disable() {
    DCDisable();
    ICDisable();
}

SMS_NO_INLINE void *BetterSMS::Memory::malloc(const size_t size, const size_t alignment) {
    return JKRHeap::sCurrentHeap->alloc(size, alignment);
}

SMS_NO_INLINE void *BetterSMS::Memory::hmalloc(JKRHeap *heap, const size_t size,
                                               const size_t alignment) {
    return JKRHeap::alloc(size, alignment, heap);
}

SMS_NO_INLINE void *BetterSMS::Memory::calloc(const size_t size, const size_t alignment) {
    void *obj = malloc(size, alignment);
    memset(obj, 0, size);
    return obj;
}

SMS_NO_INLINE void *BetterSMS::Memory::hcalloc(JKRHeap *heap, const size_t size,
                                               const size_t alignment) {
    void *obj = hmalloc(heap, size, alignment);
    memset(obj, 0, size);
    return obj;
}

SMS_NO_INLINE void BetterSMS::Memory::free(const void *ptr) { delete (u8 *)ptr; }

SMS_NO_INLINE u32 *BetterSMS::PPC::getBranchDest(u32 *bAddr) {
    s32 offset;
    u32 instr = *bAddr;

    if (instr & 0x2000000)
        offset = (instr & 0x3FFFFFD) - 0x4000000;
    else
        offset = instr & 0x3FFFFFD;
    return static_cast<u32 *>(bAddr + (offset / 4));
}

SMS_NO_INLINE void BetterSMS::PPC::writeU8(u8 *ptr, u8 value) {
    *ptr = value;
    BetterSMS::Cache::flush(ptr, sizeof(u8));
}

SMS_NO_INLINE void BetterSMS::PPC::writeU16(u16 *ptr, u16 value) {
    *ptr = value;
    BetterSMS::Cache::flush(ptr, sizeof(u16));
}

SMS_NO_INLINE void BetterSMS::PPC::writeU32(u32 *ptr, u32 value) {
    *ptr = value;
    BetterSMS::Cache::flush(ptr, sizeof(u32));
}