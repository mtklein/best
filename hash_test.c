#include "hash.h"
#include "test.h"
#include <stdlib.h>

static _Bool match_pointee(int val, void *ctx) {
    return val == *(int const*)ctx;
}
static void test(unsigned (*hash_fn)(int)) {
    struct hash *h = NULL;
    for (int i = 0; i < 100; i += 2) {
        expect(!hash_lookup(h, hash_fn(i), match_pointee, &i));
        h = hash_insert(h, hash_fn(i), i);
        expect( hash_lookup(h, hash_fn(i), match_pointee, &i));
    }
    for (int i = 0; i < 100; i++) {
        expect( hash_lookup(h, hash_fn(i), match_pointee, &i));
        i++;
        expect(!hash_lookup(h, hash_fn(i), match_pointee, &i));
    }
    free(h);
}
static void bench(int loops, unsigned (*hash_fn)(int)) {
    struct hash *h = NULL;
    for (int i = 0; i < 100; i += 2) {
        h = hash_insert(h, hash_fn(i), i);
    }
    while (loops --> 0) {
        for (int i = 0; i < 100; i++) {
            (void)hash_lookup(h, hash_fn(i), match_pointee, &i);
        }
    }
    free(h);
}

__attribute__((no_sanitize("unsigned-integer-overflow", "unsigned-shift-base")))
static unsigned murmur3_scramble(int x) {
    unsigned bits = (unsigned)x * 0xcc9e2d51;
    bits = (bits << 15) | (bits >> 17);
    return bits * 0x1b873593;
}
static unsigned identity(int x) { return (unsigned)x; }
static unsigned     zero(int x) { (void)x; return 0; }

int main(int argc, char* argv[]) {
    test(murmur3_scramble);
    test(identity);
    test(zero);
    if (argc > 1) {
        bench(atoi(argv[1]), murmur3_scramble);
    }
    return 0;
}
