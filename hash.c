#include "hash.h"
#include <stdlib.h>

static void just_insert(struct hash *h, unsigned hash, int val) {
    unsigned   *hptr = h->data;
    int        *vptr = (int*)(hptr + h->slots);
    unsigned i, mask = (unsigned)h->slots - 1;

    for (i = hash & mask; hptr[i]; i = (i+1) & mask);
    hptr[i] = hash;
    vptr[i] = val;
}

void hash_insert(struct hash *h, unsigned user, int val) {
    int const vals = h->vals + 1,
         min_empty = vals / 3,                                 // Freely tune memory vs performance.
             slots = vals + (min_empty ? min_empty : 1),       // Must keep at least one empty slot.
              need = 1<<(32-__builtin_clz((unsigned)slots-1)); // Round up to a power of 2.

    if (h->slots < need) {
        struct hash grown = {h->vals, need, calloc((unsigned)need, sizeof(unsigned)+sizeof(int))};
        if (h->vals) {
            unsigned const *hptr = h->data;
            int      const *vptr = (int const*)(hptr + h->slots);
            for (int i = 0; i < h->slots; i++) {
                if (hptr[i]) {
                    just_insert(&grown, hptr[i], vptr[i]);
                }
            }
            free(h->data);
        }
        *h = grown;
    }
    just_insert(h, user ? user : 1, val);
    h->vals++;
}

_Bool hash_lookup(struct hash h, unsigned user, _Bool(*match)(int, void*), void *ctx) {
    if (h.vals) {
        unsigned const  hash = user ? user : 1,
                        mask = (unsigned)h.slots - 1;
        unsigned const *hptr = h.data;
        int      const *vptr = (int const*)(hptr + h.slots);

        for (unsigned i = hash & mask; hptr[i]; i = (i+1) & mask) {
            if (hptr[i] == hash && match(vptr[i], ctx)) { return 1; }
        }
    }
    return 0;
}
