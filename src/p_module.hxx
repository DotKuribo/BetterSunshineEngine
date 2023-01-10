#pragma once

#include "libs/global_unordered_map.hxx"
#include "libs/string.hxx"

#include "module.hxx"

extern BetterSMS::TGlobalUnorderedMap<BetterSMS::TGlobalString, const BetterSMS::ModuleInfo *>
    gModuleInfos;