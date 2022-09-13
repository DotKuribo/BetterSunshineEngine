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

double __cvt_ull_dbl(u64 i) {
    /* Special case : 0 is not a normalized value */
    if (i == 0)
        return 0;
    /* Exponent */
    u64 E   = log(i < 0 ? -i : i) / log(2);
    u64 exp = E + 1023;
    /* Magnificand*/
    u64 M    = i > 0 ? i : -i;
    u64 frac = M ^ (1 << E);

    /* Move frac to start at bit postion 52 */
    if (E > 52)
        /* Too long: Truncate to first 52 bits */
        frac >>= E - 52;
    else
        /* Too short: Pad to the right with zeros */
        frac <<= 52 - E;

    return exp << 52 | frac;
}

void abort() {
    while (1) {
        __asm volatile("b 0x0\r\n");
    }
}