#include "hash.h"
#include <assert.h>
#include <stdlib.h>

static unsigned slots_for(unsigned vals) {
    assert(vals);
    unsigned const empty = vals / 3,                    // Freely tune memory vs performance.
                   slots = vals + (empty ? empty : 1);  // Must maintain at least 1 empty slot.
    return 1u<<(32 - __builtin_clz(slots-1));           // Round up to a power of 2.
}

static void just_insert(struct hash *h, unsigned hash, int val) {
    unsigned i, mask = h->slots - 1;
    for (i = hash & mask; h->hash[i]; i = (i+1) & mask);
    h->hash[i] = hash;
    h->val [i] = val;
    h->vals++;
}

void hash_insert(struct hash *h, unsigned user, int val) {
    unsigned const need = slots_for(h->vals+1);
    if (h->slots < need) {
        struct hash grown = {
            .slots = need,
            .hash  = calloc(need, sizeof *grown.hash),
            .val   = calloc(need, sizeof *grown.val ),
        };
        for (unsigned i = 0; i < h->slots; i++) {
            if (h->hash[i]) {
                just_insert(&grown, h->hash[i], h->val[i]);
            }
        }
        assert(grown.vals == h->vals);
        free(h->hash);
        free(h->val );
        *h = grown;
    }
    just_insert(h, user ? user : 1, val);
}

_Bool hash_lookup(struct hash const *h, unsigned user, _Bool(*match)(int, void*), void *ctx) {
    if (h->slots) {
        unsigned const hash = user ? user : 1,
                       mask = h->slots - 1;
        for (unsigned i = hash & mask; h->hash[i]; i = (i+1) & mask) {
            if (h->hash[i] == hash && match(h->val[i], ctx)) { return 1; }
        }
    }
    return 0;
}
