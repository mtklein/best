#include "hash.h"
#include <assert.h>
#include <stdlib.h>

struct hash {
    unsigned len,cap;
    struct slot { unsigned hash; int val; } slot[];
};

static void insert(struct hash *h, unsigned hash, int val) {
    struct slot *s = h->slot + (hash % h->cap);
    for (struct slot const *e = h->slot + h->cap; s->hash;) {
        if (++s == e) { s = h->slot; }
    }
    *s = (struct slot){hash, val};
    h->len++;
}

struct hash* hash_insert(struct hash *h, unsigned hash, int val) {
    unsigned const len = h ? h->len : 0,
                   cap = h ? h->cap : 0;
    if (len >= 3*cap/4) {
        _Static_assert(sizeof *h == sizeof *h->slot, "The first ''slot'' holds len and cap.");
        unsigned const growth = cap ? 2*(cap+1) : 8;
        struct hash   *grown  = calloc(growth, sizeof *grown->slot);
        grown->cap = growth - 1;
        if (h) {
            for (struct slot *s = h->slot, *e = h->slot + h->cap; s != e; s++) {
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
        assert(h->len < h->cap && "This loop needs an empty slot to terminate.");
        for (struct slot const *s = h->slot + (hash % h->cap), *e = h->slot + h->cap; s->hash;) {
            if (s->hash == hash && match(s->val, ctx)) { return (_Bool)1; }
            if (++s == e) { s = h->slot; }
        }
    }
    return (_Bool)0;
}
