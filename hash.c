#include "hash.h"
#include <stdbool.h>
#include <stdlib.h>

struct hash {
    unsigned len,cap;
    struct { unsigned hash; int val; } table[];
};

static void insert(struct hash *h, unsigned hash, int val) {
    for (unsigned i = hash % h->cap; ; i = (i+1) % h->cap) {
        if (h->table[i].hash == 0) {
            h->table[i].hash = hash;
            h->table[i].val  = val;
            h->len++;
            break;
        }
    }
}

struct hash* hash_insert(struct hash *h, unsigned hash, int val) {
    hash = hash ? hash : 1;
    unsigned const len = h ? h->len : 0,
                   cap = h ? h->cap : 0;
    if (len >= 7*cap/8) {
        unsigned const growth = cap ? 2*cap : 8;
        struct hash   *grown  = calloc(1, sizeof *grown + growth * sizeof *grown->table);
        grown->cap = growth;
        for (unsigned i = 0; i < cap; i++) {
            if (h->table[i].hash) {
                insert(grown, h->table[i].hash, h->table[i].val);
            }
        }
        free(h);
        h = grown;
    }
    insert(h,hash,val);
    return h;
}

bool hash_lookup(struct hash const *h, unsigned hash, bool(*match)(int, void*), void *ctx) {
    hash = hash ? hash : 1;
    if (h) {
        for (unsigned i = hash % h->cap; ; i = (i+1) % h->cap) {
            if (h->table[i].hash == 0) { break; }
            if (h->table[i].hash == hash && match(h->table[i].val, ctx)) { return true; }
        }
    }
    return false;
}
