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
    int *it = h + (hash & (slots-1));
    while (*it) {                         // Looking for empty slot.
        if (++it == h+slots) { it = h; }  // Wrap if we walk off the end.
    }
    *it = (int)hash;
    it[slots] = val;
}

int* hash_insert(int *h, int vals, unsigned user, int val) {
    unsigned       have = slots_for(vals);
    unsigned const need = slots_for(vals+1);
    if (have < need) {
        int *grown = calloc(2*need, sizeof *grown);
        for (int *it = h; h && it != h+have; it++) {
            if (*it) {
                just_insert(grown,need, (unsigned)*it, it[have]);
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
        for (int const *it = h + (hash & (slots-1)); *it;) {
            if ((unsigned)*it == hash && match(it[slots], ctx)) { return 1; }
            if (++it == h+slots) { it = h; }
        }
    }
    return 0;
}
