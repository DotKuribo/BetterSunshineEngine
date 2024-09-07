#include <math.h>
#include <types.h>


void abort() {
    while (1) {
        __asm volatile("b 0x0\r\n");
    }
}