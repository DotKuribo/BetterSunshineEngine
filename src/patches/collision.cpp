#include <SMS/macros.h>

#include "common_sdk.h"
#include "module.hxx"

#if BETTER_SMS_NO_DOWNWARP

// Make illegal data not downwarp anymore
SMS_WRITE_32(SMS_PORT_REGION(0x8018D08C, 0x80185914, 0, 0), 0x60000000);

#endif