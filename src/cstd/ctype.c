#include <Dolphin/types.h>
#include <SMS/macros.h>

SMS_NO_INLINE int isxdigit(int c) {
    if (c >= '0' && c <= '9')
        return true;
    else if (c >= 'a' && c <= 'f')
        return true;
    else if (c >= 'A' && c <= 'F')
        return true;

    return false;
}

SMS_NO_INLINE int isupper(int c) {
    if (c >= 'A' && c <= 'Z')
        return true;

    return false;
}

SMS_NO_INLINE int isspace(int c) {
    if (c >= '\t' && c <= '\r')
        return true;
    else if (c == ' ')
        return true;

    return false;
}

SMS_NO_INLINE int ispunct(int c) {
    if (c >= '!' && c <= '/')
        return true;
    else if (c >= ':' && c <= '@')
        return true;
    else if (c >= '[' && c <= '`')
        return true;
    else if (c >= '{' && c <= '~')
        return true;
    else if (c > 0x7F)
        return true;

    return false;
}

SMS_NO_INLINE int isprint(int c) {
    if (c >= ' ' && c <= '~')
        return true;
    else if (c > 0x7F)
        return true;

    return false;
}

SMS_NO_INLINE int islower(int c) {
    if (c >= 'a' && c <= 'z')
        return true;

    return false;
}

SMS_NO_INLINE int isgraph(int c) {
    if (c >= '!' && c <= '~')
        return true;
    else if (c > 0x7F)
        return true;

    return false;
}

SMS_NO_INLINE int isdigit(int c) {
    if (c >= '0' && c <= '9')
        return true;

    return false;
}

SMS_NO_INLINE int iscntrl(int c) {
    if (c >= 0 && c <= 0x1F)
        return true;
    else if (c == 0x7F)
        return true;

    return false;
}

SMS_NO_INLINE int isalpha(int c) {
    if (c >= 'a' && c <= 'z')
        return true;
    else if (c >= 'A' && c <= 'Z')
        return true;

    return false;
}

SMS_NO_INLINE int isalnum(int c) {
    if (c >= '0' && c <= '9')
        return true;
    else if (c >= 'a' && c <= 'z')
        return true;
    else if (c >= 'A' && c <= 'Z')
        return true;

    return false;
}