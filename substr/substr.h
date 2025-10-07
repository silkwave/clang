#ifndef SUBSTR_H
#define SUBSTR_H

typedef enum {
    ENCODING_UTF8,
    ENCODING_MS949
} encoding_t;

char* substr(const char* str, int start, int length, encoding_t enc);

#endif // SUBSTR_H
