#pragma once

#include "global_allocator.hxx"
#include <JSystem/JGadget/UnorderedMap.hxx>

namespace BetterSMS {
    template <class _Key, class _T, class _Hash = JSystem::hash<_Key>,
              class _Pred = JSystem::equal_to<_Key>> 
    using TGlobalUnorderedMap = JGadget::TUnorderedMap<_Key, _T, _Hash, _Pred, TGlobalAllocator<JGadget::TPair<const _Key, _T>>>;
}