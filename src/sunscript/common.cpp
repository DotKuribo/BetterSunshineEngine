#include <SMS/assert.h>

#include "sunscript.hxx"

namespace Spc::Stack {

    TSpcSlice popItem(TSpcInterp *interp) {
        if (!(interp->mSlicesCount > 0)) {
            SpcTrace("TSpcStack : stack overflow\n");
        }
        interp->mSlicesCount -= 1;
        return interp->mSlices[interp->mSlicesCount];
    }

    void pushItem(TSpcInterp *interp, u32 value, Spc::ValueType type) {
        if (!(interp->mSlicesCount < interp->mSlicesMax)) {
            SpcTrace("TSpcStack : stack underflow\n");
        }
        TSpcSlice slice;
        slice.mValue                          = value;
        slice.mType                           = static_cast<u32>(type);
        interp->mSlices[interp->mSlicesCount] = slice;
        interp->mSlicesCount += 1;
    }

}  // namespace Spc::Stack