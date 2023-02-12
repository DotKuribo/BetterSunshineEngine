#include <Dolphin/mem.h>

#include "p_sunscript.hxx"

void Spc::read8(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    u8 *dst = reinterpret_cast<u8 *>(Spc::Stack::popItem(interp).mValue);
    Spc::Stack::pushItem(interp, *dst, Spc::ValueType::INT);
}

void Spc::read16(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    u16 *dst = reinterpret_cast<u16 *>(Spc::Stack::popItem(interp).mValue);
    Spc::Stack::pushItem(interp, *dst, Spc::ValueType::INT);
}

void Spc::read32(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    u32 *dst = reinterpret_cast<u32 *>(Spc::Stack::popItem(interp).mValue);
    Spc::Stack::pushItem(interp, *dst, Spc::ValueType::INT);
}

void Spc::write8(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(2, &argc);
    u8 val  = Spc::Stack::popItem(interp).mValue;
    u8 *dst = reinterpret_cast<u8 *>(Spc::Stack::popItem(interp).mValue);
    *dst    = val;
}

void Spc::write16(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(2, &argc);
    u16 val  = Spc::Stack::popItem(interp).mValue;
    u16 *dst = reinterpret_cast<u16 *>(Spc::Stack::popItem(interp).mValue);
    *dst     = val;
}

void Spc::write32(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(2, &argc);
    u32 val  = Spc::Stack::popItem(interp).mValue;
    u32 *dst = reinterpret_cast<u32 *>(Spc::Stack::popItem(interp).mValue);
    *dst     = val;
}

void Spc::memcpy_(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(3, &argc);
    size_t size = Spc::Stack::popItem(interp).mValue;
    void *src   = reinterpret_cast<void *>(Spc::Stack::popItem(interp).mValue);
    void *dst   = reinterpret_cast<void *>(Spc::Stack::popItem(interp).mValue);
    memcpy(dst, src, size);
}

void Spc::memmove_(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(3, &argc);
    size_t size = Spc::Stack::popItem(interp).mValue;
    void *src   = reinterpret_cast<void *>(Spc::Stack::popItem(interp).mValue);
    void *dst   = reinterpret_cast<void *>(Spc::Stack::popItem(interp).mValue);
    memmove(dst, src, size);
}

void Spc::memcmp_(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(3, &argc);
    size_t size = Spc::Stack::popItem(interp).mValue;
    void *src   = reinterpret_cast<void *>(Spc::Stack::popItem(interp).mValue);
    void *dst   = reinterpret_cast<void *>(Spc::Stack::popItem(interp).mValue);
    memcmp(dst, src, size);
}

void Spc::memset_(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(3, &argc);
    size_t size = Spc::Stack::popItem(interp).mValue;
    u32 fill    = Spc::Stack::popItem(interp).mValue;
    void *dst   = reinterpret_cast<void *>(Spc::Stack::popItem(interp).mValue);
    memset(dst, fill, size);
}