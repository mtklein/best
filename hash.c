#include "hash.h"
#include <assert.h>
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
    if (len >= 3*cap/4) {
        _Static_assert(sizeof *h == sizeof *h->slot, "The first ''slot'' holds len and cap.");
        unsigned const growth = cap ? 2*(cap+1) : 8;
        struct hash   *grown  = calloc(growth, sizeof *grown->slot);
        grown->cap = growth - 1;
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
        assert(h->len < h->cap && "This loop needs an empty slot (it->hash == 0) to terminate.");
        for (struct slot const *it = h->slot + (hash % h->cap), *end = h->slot + h->cap;;) {
            if (it->hash == 0) { break; }
            if (it->hash == hash && match(it->val, ctx)) { return (_Bool)1; }
            if (++it == end) { it = h->slot; }
        }
    }
    return (_Bool)0;
}
