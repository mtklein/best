#include "hash.h"
#include <stdlib.h>

static void just_insert(struct hash *h, unsigned hash, int val) {
    unsigned i;
    for (i = hash & h->mask; h->hash[i]; i = (i+1) & h->mask);
    h->hash[i] = hash;
    h->val [i] = val;
}

void hash_insert(struct hash *h, unsigned user, int val) {
    int const vals = h->vals + 1,
         min_empty = vals / 3,                               // Freely tune memory vs performance.
             slots = vals + (min_empty ? min_empty : 1);     // Must keep at least one empty slot.
    unsigned const need = 1u<<(32 - __builtin_clz((unsigned)slots-1));  // Round up to power of 2.

    if (h->mask+1 < need) {
        struct hash grown = {
            .vals = h->vals,
            .mask = need-1,
            .hash = calloc(need, sizeof *grown.hash),
            .val  = calloc(need, sizeof *grown.val ),
        };
        if (h->vals) {
            for (unsigned i = 0; i <= h->mask; i++) {
                if (h->hash[i]) {
                    just_insert(&grown, h->hash[i], h->val[i]);
                }
            }
            free(h->hash);
            free(h->val );
        }
        *h = grown;
    }
    just_insert(h, user ? user : 1, val);
    h->vals++;
}

_Bool hash_lookup(struct hash const *h, unsigned user, _Bool(*match)(int, void*), void *ctx) {
    if (h->vals) {
        unsigned const hash = user ? user : 1;
        for (unsigned i = hash & h->mask; h->hash[i]; i = (i+1) & h->mask) {
            if (h->hash[i] == hash && match(h->val[i], ctx)) { return 1; }
        }
    }
    return 0;
}
