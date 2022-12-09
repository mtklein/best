#include "array.h"
#include <stdlib.h>

static _Bool is_pow2_or_zero(int x) {
    return (x & (x-1)) == 0;
}

void* push_back_(void *ptr, int len, int sizeofT) {
    if (is_pow2_or_zero(len)) {
        ptr = realloc(ptr, (len ? (size_t)len * 2 : 1) * (size_t)sizeofT);
    }
    return ptr;
}

void* drop_back_(void *ptr, int len, int sizeofT) {
    if (is_pow2_or_zero(len)) {
        ptr = len ? realloc(ptr, (size_t)len * (size_t)sizeofT)
                  : ((void)free(ptr), NULL);
    }
    return ptr;
}
