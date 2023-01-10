#pragma once

#include "global_allocator.hxx"
#include <JSystem/JGadget/List.hxx>

namespace BetterSMS {
    template <class _T> using TGlobalList = JGadget::TList<_T, TGlobalAllocator<_T>>;
}