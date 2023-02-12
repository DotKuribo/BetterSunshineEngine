#pragma once

#include <Dolphin/OS.h>
#include <Dolphin/mem.h>
#include <Dolphin/types.h>
#include <JSystem/JKernel/JKRHeap.hxx>

namespace BetterSMS {
    namespace Cache {
        void flush(void *addr, size_t size);
        void flash();
        void store(void *addr, size_t size);
        void zero(void *addr, size_t size);
        void enable();
        void disable();
    }  // namespace Cache

    namespace Memory {
        void *malloc(const size_t size, const size_t alignment);
        void *hmalloc(JKRHeap *heap, const size_t size, const size_t alignment);
        void *calloc(const size_t size, const size_t alignment);
        void *hcalloc(JKRHeap *heap, const size_t size, const size_t alignment);
        void free(const void *ptr);
    }  // namespace Memory

    namespace PowerPC {
        u32 *getBranchDest(u32 *bAddr);
        void writeU8(u8 *ptr, u8 value);
        void writeU16(u16 *ptr, u16 value);
        void writeU32(u32 *ptr, u32 value);
    }  // namespace PowerPC
}  // namespace BetterSMS