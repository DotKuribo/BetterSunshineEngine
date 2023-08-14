#include <SMS/Strategic/HitActor.hxx>
#include <SMS/Strategic/LiveActor.hxx>
#include <SMS/System/CardManager.hxx>
#include <SMS/macros.h>

#include "module.hxx"

#if BETTER_SMS_SLOT_B_SUPPORT

/*** Memory File Buffer ***/
#define MAXFILEBUFFER (1024 * 2048) /*** 2MB Buffer ***/

/*
class CardHandler
{
    void *mSysArea;
    CARDInfo *mCardInfo;

public:
    CARDInfo *getInfo() const { return this->mCardInfo; }
    void *getSysArea() const { return this->mSysArea; }

};
*/
// static u8 sSysArea[CARD_WORKAREA] __attribute__((aligned(32)));
// static CARDInfo sCardInfo;
// static int cardcount = 0;

static s32 mountCard(TCardManager *cardManager, bool r4) {
    bool tryMount = true;
    s32 ret       = 0;

    if (cardManager->mMounted)
        tryMount = cardManager->probe() < -1;

    if (tryMount) {
        cardManager->mChannel = CARD_SLOTA;

        ret = cardManager->mount_(r4);
        if (ret == 0)
            return ret;

        cardManager->mChannel = CARD_SLOTB;

        ret = cardManager->mount_(r4);
        if (ret == 0)
            return ret;

        cardManager->mChannel = CARD_SLOTA;
        return ret;
    }

    return ret;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802B20F8, 0x802AA008, 0, 0), mountCard);

static s32 probeCard(TCardManager *cardManager) {
    s32 ret = cardManager->probe();
    if (ret == CARD_ERROR_NOCARD) {
        cardManager->unmount();
        cardManager->mLastStatus = mountCard(cardManager, true);
    }
    return ret;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80163C40, 0x80158BF0, 0, 0), probeCard);

#endif