#include "hash.h"
#include <stdlib.h>

struct hash { unsigned hash; int val; };

static unsigned slots(int vals) {
    if (vals == 0) {
        return 0;
    }
    int const empty = vals / 3,                          // Freely tune memory vs performance.
              slots = vals + (empty ? empty : 1);        // Must maintain at least 1 empty slot.
    return 1u<<(32 - __builtin_clz((unsigned)slots-1));  // Round up to a power of 2.
}

static struct hash* insert(struct hash *h, unsigned mask, struct hash slot) {
    struct hash *it = h + (slot.hash & mask);
    while (it->hash) {                   // Looking for an empty slot...
        if (it++ == h+mask) { it = h; }  // Wrap around to front if we would walk off the end.
    }
    *it = slot;
    return h;
}

struct hash* hash_insert(struct hash *h, int vals, unsigned hash, int val) {
    struct hash const slot = {hash ? hash : 1, val};
    unsigned const have = slots(vals),
                   need = slots(vals+1);
    if (have == need) {
        return insert(h,have-1,slot);
    }
    struct hash *g = calloc(need, sizeof *g);
    for (struct hash *it = h; h && it != h+have; it++) {
        if (it->hash) {
            insert(g,need-1,*it);
        }
    }
    free(h);
    return insert(g,need-1,slot);
}

_Bool hash_lookup(struct hash const *h, int vals, unsigned hash,
                  _Bool(*match)(int, void*), void *ctx) {
    hash = hash ? hash : 1;
    if (h) {
        unsigned const mask = slots(vals)-1;
        for (struct hash const *it = h + (hash & mask); it->hash;) {
            if (it->hash == hash && match(it->val, ctx)) { return 1; }
            if (it++ == h+mask) { it = h; }
        }
    }
    return 0;
}
