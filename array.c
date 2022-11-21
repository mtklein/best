#include "array.h"
#include <stdlib.h>

static _Bool is_pow2_or_zero(int x) {
    return (x & (x-1)) == 0;
}

void* push_back_(void *ptr, int len, int sizeofT) {
    if (is_pow2_or_zero(len)) {
        ptr = realloc(ptr, (size_t)((len ? 2*len : 1)*sizeofT));
    }
    return ptr;
}

void* drop_back_(void *ptr, int len, int sizeofT) {
    if (is_pow2_or_zero(len)) {
        ptr = len ? realloc(ptr, (size_t)(len * sizeofT))
                  : ((void)free(ptr), NULL);
    }
    return ptr;
}
