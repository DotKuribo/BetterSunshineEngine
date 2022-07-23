#pragma once

#include <Dolphin/OS.h>
#include <Dolphin/mem.h>
#include <Dolphin/types.h>
#include <JSystem/JKernel/JKRHeap.hxx>

namespace BetterSMS::Cache {
    void flush(void *addr, size_t size);
    void flash();
    void store(void *addr, size_t size);
    void zero(void *addr, size_t size);
    void enable();
    void disable();
}  // namespace BetterSMS::Cache

namespace BetterSMS::Memory {
    void *malloc(const size_t size, const size_t alignment);
    void *hmalloc(JKRHeap *heap, const size_t size, const size_t alignment);
    void *calloc(const size_t size, const size_t alignment);
    void *hcalloc(JKRHeap *heap, const size_t size, const size_t alignment);
    void free(const void *ptr) { delete (u8 *)ptr; }

    namespace Protection {
        enum ACCESS { DENIED, READ, WRITE, READWRITE };

        class MemoryMap {
        public:
            MemoryMap() {}
            MemoryMap(u8 index);
            ~MemoryMap();

            void *end() const { return (void *)((this->_mEnd << 10) | 0x80000000); }
            u8 index() const { return this->mIndex; }
            ACCESS permission() const { return this->mPermission; }
            s32 size() const { return (this->_mEnd - this->_mStart) << 10; }
            void *start() const { return (void *)((this->_mStart << 10) | 0x80000000); }
            void setStart(const u32 addr) { this->_mStart = (addr & 0x1FFFFFF) >> 10; }
            void setEnd(const u32 addr) { this->_mEnd = (addr & 0x1FFFFFF) >> 10; }
            void setPermission(ACCESS perm) { this->mPermission = perm; }
            void setIndex(u8 index) { this->mIndex = index; }

            void activate();
            void deactivate();

            static const u32 MARR0    = 0xCC004000;
            static const u32 MARR1    = 0xCC004004;
            static const u32 MARR2    = 0xCC004008;
            static const u32 MARR3    = 0xCC00400C;
            static const u32 MARRCTRL = 0xCC004010;
            static const u32 MARRENBL = 0xCC00401C;

        private:
            u16 _mStart;
            u16 _mEnd;
            ACCESS mPermission;
            u8 mIndex;
        };

    }  // namespace Protection

}  // namespace BetterSMS::Memory

namespace BetterSMS::PPC {
    u32 *getBranchDest(u32 *bAddr);
    void writeU8(u8 *ptr, u8 value);
    void writeU16(u16 *ptr, u16 value);
    void writeU32(u32 *ptr, u32 value);
}  // namespace BetterSMS::PPC