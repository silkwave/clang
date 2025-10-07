#include "substr.h"
#include "resource_manager.h"
#include <stdlib.h>
#include <string.h>

static int char_len(const char* p, encoding_t enc) {
    unsigned char c = (unsigned char)*p;
    if (enc == ENCODING_UTF8) {
        if (c < 0x80) return 1;
        else if ((c >> 5) == 0x6) return 2;
        else if ((c >> 4) == 0xE) return 3;
        else if ((c >> 3) == 0x1E) return 4;
        return 1;
    } else { // MS949
        return (c < 0x80) ? 1 : 2;
    }
}

char* substr(const char* str, int start, int length, encoding_t enc) {
    if (!str) return NULL;

    int total_chars = 0;
    const char* p = str;
    while (*p) {
        total_chars++;
        p += char_len(p, enc);
    }

    if (start > 0) start -= 1;
    else if (start < 0) start = total_chars + start;

    if (start < 0 || start >= total_chars) return NULL;
    if (length <= 0 || start + length > total_chars)
        length = total_chars - start;

    p = str;
    int idx = 0;
    while (idx < start) {
        p += char_len(p, enc);
        idx++;
    }

    const char* q = p;
    idx = 0;
    while (idx < length && *q) {
        q += char_len(q, enc);
        idx++;
    }

    int byte_len = q - p;
    char* result = malloc(byte_len + 1);
    if (!result) return NULL;

    memcpy(result, p, byte_len);
    result[byte_len] = '\0';

    register_resource(result);

    return result;
}
