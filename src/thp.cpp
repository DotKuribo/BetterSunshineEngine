#include "thp.hxx"
#include "Dolphin/mem.h"
#include "application.hxx"
#include "module.hxx"

static const char *sTHPList[256];

using namespace BetterSMS;

bool THP::addTHP(u8 x, const char *filename) {
    sTHPList[x] = filename;
    return true;
}

const char *THP::getTHP(u8 id) {
    return sTHPList[id];
}

void loadBaseGameTHP() { memcpy(sTHPList, (const void *)0x803dfa00, 20 * sizeof(char *)); }

static const char *_getTHP() { return THP::getTHP(gpApplication.mCutSceneID); }

SMS_WRITE_32(SMS_PORT_REGION(0x802B6858, 0, 0, 0), 0x4800000C);
SMS_PATCH_BL(SMS_PORT_REGION(0x802B6864, 0, 0, 0), _getTHP);
SMS_WRITE_32(SMS_PORT_REGION(0x802B6868, 0, 0, 0), 0x7C7A1B78);
SMS_WRITE_32(SMS_PORT_REGION(0x802B686C, 0, 0, 0), 0x4800000C);

SMS_PATCH_B(SMS_PORT_REGION(0x802b70b4, 0, 0, 0), _getTHP);