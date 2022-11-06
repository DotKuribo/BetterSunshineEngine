#include <SMS/macros.h>


#include "module.hxx"

#if BETTER_SMS_NO_DOWNWARP

// Make illegal data not downwarp anymore
SMS_WRITE_32(SMS_PORT_REGION(0x8018D080, 0x80185908, 0, 0), 0x38000600);
SMS_WRITE_32(SMS_PORT_REGION(0x8018D088, 0x80185910, 0, 0), 0xB01E0000);
SMS_WRITE_32(SMS_PORT_REGION(0x8018D08C, 0x80185914, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x8018D090, 0x80185918, 0, 0), 0x60000000);

#endif