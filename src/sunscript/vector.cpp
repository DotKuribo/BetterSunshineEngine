#include <Dolphin/MTX.h>
#include <JSystem/JGeometry.hxx>

#include "sunscript.hxx"

void Spc::vectorTranslate(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(2, &argc);
    TVec3f *other = reinterpret_cast<TVec3f *>(Spc::Stack::popItem(interp).mValue);
    TVec3f *self  = reinterpret_cast<TVec3f *>(Spc::Stack::popItem(interp).mValue);
    self->add(*other);
}

void Spc::vectorTranslatef(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(4, &argc);
    f32 z        = static_cast<f32>(Spc::Stack::popItem(interp).mValue);
    f32 y        = static_cast<f32>(Spc::Stack::popItem(interp).mValue);
    f32 x        = static_cast<f32>(Spc::Stack::popItem(interp).mValue);
    TVec3f *self = reinterpret_cast<TVec3f *>(Spc::Stack::popItem(interp).mValue);

    self->x += x;
    self->y += y;
    self->z += z;
}

void Spc::vectorScalef(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(2, &argc);
    f32 scale    = static_cast<f32>(Spc::Stack::popItem(interp).mValue);
    TVec3f *self = reinterpret_cast<TVec3f *>(Spc::Stack::popItem(interp).mValue);
    self->scale(scale);
}

void Spc::vectorMagnitude(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    Vec *self = reinterpret_cast<Vec *>(Spc::Stack::popItem(interp).mValue);
    Spc::Stack::pushItem(interp, VECMag(self), Spc::FLOAT);
}

void Spc::vectorNormalize(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(1, &argc);
    Vec *self = reinterpret_cast<Vec *>(Spc::Stack::popItem(interp).mValue);
    VECNormalize(self, self);
}