#include <SMS/macros.h>
#include <SMS/manager/FlagManager.hxx>
#include <SMS/spc/SpcBinary.hxx>

#include "common_sdk.h"
#include "logging.hxx"
#include "module.hxx"
#include "p_settings.hxx"

using namespace BetterSMS;

#if defined(BETTER_SMS_BUGFIXES) || defined(BETTER_SMS_CRASHFIXES)

static void initBinaryNullptrPatch(TSpcBinary *binary) {
    if (binary || !BetterSMS::areBugsPatched()) {
        binary->init();
        return;
    }
    Console::debugLog("Warning: SPC binary is nullptr! \n");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80289098, 0x80280E24, 0, 0), initBinaryNullptrPatch);

#endif