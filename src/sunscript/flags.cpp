#include "globals.hxx"
#include "sunscript.hxx"

void Spc::isDebugMode(TSpcInterp *interp, u32 argc) {
    interp->verifyArgNum(0, &argc);
    Spc::Stack::pushItem(interp, static_cast<u32>(BetterSMS::isDebugMode()),
                         Spc::ValueType::INT);  // Return a value
}

void Spc::getActivePlayers(TSpcInterp* interp, u32 argc)
{
}

void Spc::getMaxPlayers(TSpcInterp * interp, u32 argc)
{

}

void Spc::getPlayerByIndex(TSpcInterp* interp, u32 argc)
{
}

void Spc::getDateAsStr(TSpcInterp* interp, u32 argc)
{
}
