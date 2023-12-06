#include "module.hxx"

// ignores a flag test that would usually only be true on git
SMS_WRITE_32(SMS_PORT_REGION(0x80083ffc, 0, 0, 0), 0x60000000);