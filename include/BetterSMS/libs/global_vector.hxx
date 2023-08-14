#pragma once

#include "global_allocator.hxx"
#include <JSystem/JGadget/Vector.hxx>

namespace BetterSMS {
    template <class _T> using TGlobalVector = JGadget::TVector<_T, TGlobalAllocator<_T>>;
}