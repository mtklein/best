#include "hash.h"
#include <stdlib.h>

static unsigned slots_for(int vals) {
    if (vals == 0) {
        return 0;
    }
    int const empty = vals / 3,                          // Freely tune memory vs performance.
              slots = vals + (empty ? empty : 1);        // Must maintain at least 1 empty slot.
    return 1u<<(32 - __builtin_clz((unsigned)slots-1));  // Round up to a power of 2.
}

static void just_insert(int *h, unsigned slots, unsigned hash, int val) {
    unsigned i;
    for (i = hash & (slots-1); h[i]; i = (i+1) & (slots-1));
    h[i] = (int)hash;
    h[i+slots] = val;
}

int* hash_insert(int *h, int vals, unsigned user, int val) {
    unsigned       have = slots_for(vals);
    unsigned const need = slots_for(vals+1);
    if (have < need) {
        int *grown = calloc(2*need, sizeof *grown);
        for (unsigned i = 0; h && i != have; i++) {
            if (h[i]) {
                just_insert(grown,need, (unsigned)h[i], h[i+have]);
            }
        }
        free(h);
        h    = grown;
        have = need;
    }
    just_insert(h,have, user ? user : 1, val);
    return h;
}

_Bool hash_lookup(int const *h, int vals, unsigned user, _Bool(*match)(int, void*), void *ctx) {
    unsigned const hash = user ? user : 1,
                  slots = slots_for(vals);
    if (h) {
        for (unsigned i = hash & (slots-1); h[i]; i = (i+1) & (slots-1)) {
            if ((unsigned)h[i] == hash && match(h[i+slots], ctx)) { return 1; }
        }
    }
    return 0;
}
