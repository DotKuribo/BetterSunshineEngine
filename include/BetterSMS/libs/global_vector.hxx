#pragma once

#include <JSystem/JGadget/Vector.hxx>
#include "libs/global_allocator.hxx"

namespace BetterSMS {
    template <class _T> 
    using TGlobalVector = JGadget::TVector<_T, TGlobalAllocator<_T>>;
}