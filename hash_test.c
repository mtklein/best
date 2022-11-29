#include "hash.h"
#include "test.h"
#include <stdlib.h>
//#include <malloc/malloc.h>

static _Bool match_pointee(int val, void *ctx) {
    return val == *(int const*)ctx;
}
static void test(int const n, unsigned (*hash_fn)(int)) {
    int *h = NULL;
    for (int i = 0; i < n; i += 2) {
        expect(!hash_lookup(h,i/2, hash_fn(i), match_pointee, &i));
        h = hash_insert(h,i/2, hash_fn(i), i);
        expect( hash_lookup(h,i/2+1, hash_fn(i), match_pointee, &i));
    }
    for (int i = 0; i < n; i++) {
        expect( hash_lookup(h,n/2, hash_fn(i), match_pointee, &i));
        i++;
        expect(!hash_lookup(h,n/2, hash_fn(i), match_pointee, &i));
    }

#if defined(_MALLOC_MALLOC_H_)
    size_t const slots = (malloc_size(h) / sizeof *h) / 2;
    dprintf(1, "%zu slots\n", slots);
    for (size_t i = 0; i < slots; i++) {
        if (h[i]) {
            dprintf(1, "%zu\t0x%08x %d\n", i, h[i], h[i+slots]);
        } else {
            dprintf(1, "%zu\t~~~~~~~~~~\n", i);
        }
    }
#endif

    free(h);
}
static void bench(int const n, unsigned (*hash_fn)(int), int loops) {
    int *h = NULL;
    for (int i = 0; i < n; i += 2) {
        h = hash_insert(h,i/2, hash_fn(i), i);
    }
    while (loops --> 0) {
        for (int i = 0; i < n; i++) {
            (void)hash_lookup(h,n/2, hash_fn(i), match_pointee, &i);
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
static unsigned     zero(int x) { (void)x; return 0; }
static unsigned identity(int x) { return (unsigned)x; }

int main(int argc, char* argv[]) {
    test(    100, zero);
    test(  10000, identity);
    test(1000000, murmur3_scramble);
    if (argc > 1) {
        bench(100, murmur3_scramble, atoi(argv[1]));
    }
    return 0;
}
