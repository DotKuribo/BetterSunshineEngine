#include <Dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif
void reverse(char *dst, const char *src, size_t len);

// Implementation of itoa()
void itoa(char *dst, int num, int base);
#ifdef __cplusplus
}
#endif