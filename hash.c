#include "hash.h"
#include <assert.h>
#include <stdlib.h>

static unsigned slots(int vals) {
    assert(vals);
    int const empty = vals / 3,                          // Freely tune memory vs performance.
              slots = vals + (empty ? empty : 1);        // Must maintain at least 1 empty slot.
    return 1u<<(32 - __builtin_clz((unsigned)slots-1));  // Round up to a power of 2.
}

static void just_insert(struct hash *h, unsigned hash, int val) {
    unsigned i;
    for (i = hash & h->mask; h->hash[i]; i = (i+1) & h->mask);
    h->hash[i] = hash;
    h->val [i] = val;
    h->vals++;
}

void hash_insert(struct hash *h, unsigned user, int val) {
    unsigned const need = slots(h->vals+1);
    if (h->mask+1 < need) {
        struct hash grown = {
            .mask = need-1,
            .hash = calloc(need, sizeof *grown.hash),
            .val  = calloc(need, sizeof *grown.val ),
        };
        for (unsigned i = 0; h->vals && i <= h->mask; i++) {
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
    if (h->vals) {
        unsigned const hash = user ? user : 1;
        for (unsigned i = hash & h->mask; h->hash[i]; i = (i+1) & h->mask) {
            if (h->hash[i] == hash && match(h->val[i], ctx)) { return 1; }
        }
    }
    return 0;
}
