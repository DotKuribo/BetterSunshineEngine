#pragma once

#include <Dolphin/types.h>
#include <SMS/SPC/SpcInterp.hxx>
#include <SMS/SPC/SpcSlice.hxx>
#include <SMS/assert.h>

#include "sunscript.hxx"

namespace BetterSMS {

    namespace Spc {

        typedef void (*SpcFunction)(TSpcInterp *interp, u32 argc);

        enum ValueType { INT, FLOAT, STRING };

        namespace Stack {

            inline TSpcSlice popItem(TSpcInterp *interp) {
                if (!(interp->mSlicesCount > 0)) {
                    SpcTrace("TSpcStack : stack underflow\n");
                    return interp->mSlices[interp->mSlicesCount];
                }
                return interp->mSlices[--interp->mSlicesCount];
            }

            inline void pushItem(TSpcInterp *interp, u32 value, Spc::ValueType type) {
                TSpcSlice slice;
                slice.mValue = value;
                slice.mType  = static_cast<u32>(type);

                if (!(interp->mSlicesCount < interp->mSlicesMax)) {
                    SpcTrace("TSpcStack : stack overflow\n");
                    interp->mSlices[interp->mSlicesCount] = slice;
                    return;
                }

                interp->mSlices[interp->mSlicesCount++] = slice;
            }

        }  // namespace Stack

        bool registerBuiltinFunction(const char *key, SpcFunction function);

    }  // namespace Spc

}  // namespace BetterSMS