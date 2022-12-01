#include "hash.h"
#include <stdlib.h>

static void just_insert(struct hash *h, unsigned hash, int val) {
    unsigned   *hptr = h->data;
    int        *vptr = (int*)(hptr + h->slots);
    unsigned i, mask = h->slots - 1;

    for (i = hash & mask; hptr[i]; i = (i+1) & mask);
    hptr[i] = hash;
    vptr[i] = val;
}

void hash_insert(struct hash *h, unsigned user, int val) {
    if (h->vals/3 >= h->slots/4) {
        struct hash grown = {.vals=h->vals, .slots=h->slots ? 2*h->slots : 2};
        grown.data = calloc(grown.slots, sizeof(unsigned) + sizeof(int));

        if (h->vals) {
            unsigned const *hptr = h->data;
            int      const *vptr = (int const*)(hptr + h->slots);
            for (unsigned i = 0; i < h->slots; i++) {
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

#define walk unsigned const *hptr = h.data, hash = user ? user : 1, mask = h.slots - 1; \
             int      const *vptr = (int const*)(hptr + h.slots);                       \
             for (unsigned i = hash & mask; hptr[i]; i = (i+1) & mask)

__attribute__((cold))
static _Bool hash_lookup_slow(struct hash h, unsigned user, _Bool(*match)(int, void*), void *ctx) {
    walk { if (hptr[i] == hash && match(vptr[i], ctx)) { return 1; } }
    return 0;
}

_Bool hash_lookup(struct hash h, unsigned user, _Bool(*match)(int, void*), void *ctx) {
    if (h.vals) {
        if (match) { return hash_lookup_slow(h,user,match,ctx); }
        walk { if (hptr[i] == hash && vptr[i] == (int)(intptr_t)ctx) { return 1; } }
    }
    return 0;
}
