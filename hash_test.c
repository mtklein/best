#include "hash.h"
#include "test.h"
#include <stdlib.h>

static _Bool match_expected(int val, void *ctx) {
    return val == *(int const*)ctx;
}

static void test(unsigned (*hash_fn)(int)) {
    struct hash *h = NULL;
    for (int i = 0; i < 100; i += 2) {
        expect(!hash_lookup(h, hash_fn(i), match_expected, &i));
        h = hash_insert(h, hash_fn(i), i);
        expect( hash_lookup(h, hash_fn(i), match_expected, &i));
    }
    for (int i = 0; i < 100; i += 2) {
        expect( hash_lookup(h, hash_fn(i), match_expected, &i));
        int odd = i+1;
        expect(!hash_lookup(h, hash_fn(i), match_expected, &odd));
    }
    free(h);
}

static void bench(int loops, unsigned (*hash_fn)(int)) {
    struct hash *h = NULL;
    for (int i = 0; i < 100; i += 2) {
        h = hash_insert(h, hash_fn(i), i);
    }
    while (loops --> 0) {
        for (int i = 0; i < 100; i += 2) {
            (void)hash_lookup(h, hash_fn(i), match_expected, &i);
            int odd = i+1;
            (void)hash_lookup(h, hash_fn(i), match_expected, &odd);
        }
    }
    free(h);
}

static unsigned zero(int x) {
    (void)x;
    return 0;
}

static unsigned identity(int x) {
    return (unsigned)x;
}

__attribute__((no_sanitize("unsigned-shift-base")))
static unsigned rotate(unsigned x, int bits) {
    return (x << bits) | (x >> (32-bits));
}

static unsigned murmur3_scramble(int x) {
    unsigned bits = (unsigned)x;
    (void)__builtin_mul_overflow(bits, 0xcc9e2d51, &bits);
    bits = rotate(bits, 15);
    (void)__builtin_mul_overflow(bits, 0x1b873593, &bits);
    return bits;
}

int main(int argc, char* argv[]) {
    test(zero);
    test(identity);
    test(murmur3_scramble);
    if (argc > 1) {
        bench(atoi(argv[1]), murmur3_scramble);
    }
    return 0;
}
