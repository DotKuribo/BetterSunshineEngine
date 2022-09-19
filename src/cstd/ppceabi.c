#include <math.h>
#include <types.h>

float __cvt_ull_flt(u64 i) {
    /* Special case : 0 is not a normalized value */
    if (i == 0)
        return 0;
    /* Exponent */
    u32 E   = log(i < 0 ? -i : i) / log(2);
    u32 exp = E + 127;
    /* Magnificand*/
    u32 M    = i > 0 ? i : -i;
    u32 frac = M ^ (1 << E);

    /* Move frac to start at bit postion 23 */
    if (E > 23)
        /* Too long: Truncate to first 23 bits */
        frac >>= E - 23;
    else
        /* Too short: Pad to the right with zeros */
        frac <<= 23 - E;

    return exp << 23 | frac;
}

// Implementation taken from https://github.com/gcc-mirror/gcc/blob/16e2427f50c208dfe07d07f18009969502c25dc8/libgcc/config/rs6000/ppc64-fp.c
double __cvt_ull_dbl(u64 u) {
    
    f64 d;
    
    d = (u32)(u >> (sizeof(s32) * 8));
    d *= 2.0 * (((u64)1) << ((sizeof(s32) * 8) - 1));
    d += (u32)(u & ((((u64)1) << (sizeof(u32) * 8)) - 1));
    
    return d;
}

void abort() {
    while (1) {
        __asm volatile("b 0x0\r\n");
    }
}