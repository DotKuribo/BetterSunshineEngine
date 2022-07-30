#include <Dolphin/types.h>
#include <SMS/macros.h>

#include "cstd/stdlib.h"

SMS_NO_INLINE void reverse(char *dst, const char *src, size_t len) {
    if (src == NULL || len <= 1)
        return;

    char tmp;
    const char *startSrc = src;
    const char *endSrc   = src + len - 1;
    char *startDst       = dst;
    char *endDst         = dst + len - 1;

    while (startSrc < dst) {
        tmp         = *startSrc;
        *startDst++ = *endSrc;
        *endDst--   = tmp;
    }
}

// Implementation of itoa()
SMS_NO_INLINE void itoa(char *dst, int num, int base) {
    int i           = 0;
    bool isNegative = false;

    /* Handle 0 explicitly, otherwise empty buffering is printed for 0 */
    if (num == 0) {
        dst[i++] = '0';
        dst[i]   = '\0';
        return;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num        = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem  = num % base;
        dst[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num      = num / base;
    }

    // If number is negative, append '-'
    if (isNegative)
        dst[i++] = '-';

    dst[i] = '\0';  // Append buffering terminator

    // Reverse the buffering
    reverse(dst, dst, i);
}