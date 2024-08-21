#include "thp.hxx"
#include "Dolphin/mem.h"
#include "application.hxx"
#include "module.hxx"
static char *thpList[256];

using namespace BetterSMS;

bool THP::addTHP(u8 x, char *i) {

    thpList[x] = i;
    return true;
}

static const char *_getTHP() { return THP::getTHP(); }

const char *THP::getTHP() {
    if (gpApplication.mCutSceneID < 256) {

        return thpList[gpApplication.mCutSceneID];
    } else {

        return nullptr;
    }
}
void loadBaseGameTHP() {
    memcpy(thpList, (const void *)0x803dfa00, 20 * sizeof(char *));
}

SMS_WRITE_32(SMS_PORT_REGION(0x802B6858, 0, 0, 0), 0x4800000C);

SMS_PATCH_BL(SMS_PORT_REGION(0x802B6864, 0, 0, 0), _getTHP);

SMS_WRITE_32(SMS_PORT_REGION(0x802B6868, 0, 0, 0), 0x7C7A1B78);
SMS_WRITE_32(SMS_PORT_REGION(0x802B686C, 0, 0, 0), 0x4800000C);

SMS_PATCH_B(SMS_PORT_REGION(0x802b70b4, 0, 0, 0), _getTHP);