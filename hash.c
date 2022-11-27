#include "hash.h"
#include <stdlib.h>

struct hash {
    unsigned len,cap;
    struct slot { unsigned hash; int val; } slot[];
};

static void insert(struct hash *h, unsigned hash, int val) {
    for (struct slot *s = h->slot + (hash % h->cap), *end = h->slot + h->cap;;) {
        if (s->hash == 0) {
            s->hash = hash;
            s->val  = val;
            h->len++;
            break;
        }
        if (++s == end) { s = h->slot; }
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
            for (struct slot *s = h->slot; s != h->slot + h->cap; s++) {
                if (s->hash) {
                    insert(grown, s->hash, s->val);
                }
            }
            free(h);
        }
        h = grown;
    }
    insert(h, hash ? hash : 1, val);
    return h;
}

_Bool hash_lookup(struct hash const *h, unsigned hash, _Bool(*match)(int, void*), void *ctx) {
    hash = hash ? hash : 1;
    if (h) {
        for (struct slot const *s = h->slot + (hash % h->cap), *end = h->slot + h->cap;;) {
            if (s->hash == 0) { break; }
            if (s->hash == hash && match(s->val, ctx)) { return 1; }
            if (++s == end) { s = h->slot; }
        }
    }
    return 0;
}
