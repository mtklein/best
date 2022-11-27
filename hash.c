#include "hash.h"
#include <stdlib.h>

struct hash {
    unsigned len,cap;
    struct slot { unsigned hash; int val; } slot[];
};

static void insert(struct hash *h, struct slot const s) {
    for (struct slot *it = h->slot + (s.hash % h->cap), *end = h->slot + h->cap;;) {
        if (it->hash == 0) {
            *it = s;
            h->len++;
            break;
        }
        if (++it == end) { it = h->slot; }
    }
}

struct hash* hash_insert(struct hash *h, unsigned hash, int val) {
    unsigned const len = h ? h->len : 0,
                   cap = h ? h->cap : 0;
    if (len >= 7*cap/8) {
        unsigned const growth = cap ? 2*cap : 8;
        struct hash   *grown  = calloc(1, sizeof *grown + growth * sizeof *grown->slot);
        grown->cap = growth;
        if (h) {
            for (struct slot *it = h->slot, *end = h->slot + h->cap; it != end; it++) {
                if (it->hash) {
                    insert(grown, *it);
                }
            }
            free(h);
        }
        h = grown;
    }
    insert(h, (struct slot){hash ? hash : 1, val});
    return h;
}

_Bool hash_lookup(struct hash const *h, unsigned hash, _Bool(*match)(int, void*), void *ctx) {
    hash = hash ? hash : 1;
    if (h) {
        for (struct slot const *it = h->slot + (hash % h->cap), *end = h->slot + h->cap;;) {
            if (it->hash == 0) { break; }
            if (it->hash == hash && match(it->val, ctx)) { return (_Bool)1; }
            if (++it == end) { it = h->slot; }
        }
    }
    return (_Bool)0;
}
