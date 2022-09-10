#include "debug.hxx"
#include "libs/container.hxx"
#include "object.hxx"
#include "player.hxx"
#include "stage.hxx"

using namespace BetterSMS;

template <> TDictS<void *>::~TDictS() {}

 template <> TDictS<Debug::DebugModeInitCallback>::~TDictS() {}
// template <> TDictS<Debug::DebugModeUpdateCallback>::~TDictS() {}
// template <> TDictS<Debug::DebugModeDrawCallback>::~TDictS() {}

 template <> TDictS<Objects::NameRefInitializer>::~TDictS() {}
// template <> TDictI<Objects::ObjectInteractor>::~TDictI() {}

 template <> TDictS<Player::InitProcess>::~TDictS() {}
// template <> TDictS<Player::UpdateProcess>::~TDictS() {}

 //template <> TDictS<Stage::StageInitCallback>::~TDictS() {}
 //template <> TDictS<Stage::StageUpdateCallback>::~TDictS() {}
 template <> TDictS<Stage::Draw2DCallback>::~TDictS() {}
