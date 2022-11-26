#include "hash.h"
#include "test.h"
#include <stdlib.h>

static _Bool match_expected(int val, void *ctx) {
    return val == *(int const*)ctx;
}

static void test(int (*hash_fn)(int)) {
    struct hash *h = NULL;
    for (int i = 0; i < 100; i += 2) {
        h = hash_insert(h, hash_fn(i), i);
    }
    for (int i = 0; i < 100; i += 2) {
        expect( hash_lookup(h, hash_fn(i), match_expected, &i));
        int odd = i+1;
        expect(!hash_lookup(h, hash_fn(i), match_expected, &odd));
    }
    free(h);
}

static int zero(int x) {
    (void)x;
    return 0;
}

static int identity(int x) {
    return x;
}

__attribute__((no_sanitize("unsigned-shift-base")))
static unsigned rotate(unsigned x, int bits) {
    return (x << bits) | (x >> (32-bits));
}

static int murmur3_scramble(int x) {
    unsigned bits = (unsigned)x;
    (void)__builtin_mul_overflow(bits, 0xcc9e2d51, &bits);
    bits = rotate(bits, 15);
    (void)__builtin_mul_overflow(bits, 0x1b873593, &bits);
    return (int)bits;
}

int main(void) {
    test(zero);
    test(identity);
    test(murmur3_scramble);
    return 0;
}
