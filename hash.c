#include "hash.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct hash {
    unsigned len,mask;
    struct { unsigned hash; int val; } table[];
};

static void just_insert(struct hash *h, unsigned hash, int val) {
    assert(h && h->len < h->mask && hash);
    for (unsigned i = hash & h->mask; ; i = (i+1) & h->mask) {
        if (h->table[i].hash == 0) {
            h->table[i].hash = hash;
            h->table[i].val  = val;
            h->len++;
            break;
        }
    }
}

struct hash* hash_insert(struct hash *h, unsigned user, int val) {
    unsigned const hash = user ? user : 1,
                    len = h ? h->len    : 0,
                    cap = h ? h->mask+1 : 0;
    if (len >= 7*cap/8) {
        unsigned const growth = cap ? 2*cap : 8;
        struct hash   *grown  = calloc(1, sizeof *grown + growth * sizeof *grown->table);
        grown->mask = growth-1;
        for (unsigned i = 0; i < cap; i++) {
            if (h->table[i].hash) {
                just_insert(grown, h->table[i].hash, h->table[i].val);
            }
        }
        assert(grown->len == len);
        free(h);
        h = grown;
    }
    just_insert(h,hash,val);
    return h;
}

bool hash_lookup(struct hash const *h, unsigned user, bool(*match)(int, void*), void *ctx) {
    if (h) {
        unsigned const hash = user ? user : 1;
        for (unsigned i = hash & h->mask; ; i = (i+1) & h->mask) {
            if (h->table[i].hash == 0) { break; }
            if (h->table[i].hash == hash && match(h->table[i].val, ctx)) { return true; }
        }
    }
    return false;
}
