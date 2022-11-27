#include "hash.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct nonzero {
    unsigned hash;
};
static struct nonzero nonzero(unsigned hash) {
    return (struct nonzero){hash ? hash : 1};
}

struct hash {
    unsigned len,mask;
    struct {
        unsigned hash_or_0;
        int      val;
    } table[];
};

static bool empty_slots_at_least(struct hash const *h, unsigned k) {
    unsigned const cap = h->mask+1;
    return h->len+k <= cap;
}

static void insert(struct hash *h, struct nonzero nz, int val) {
    assert(empty_slots_at_least(h,2));
    for (unsigned i = nz.hash & h->mask; ; i = (i+1) & h->mask) {
        if (h->table[i].hash_or_0 == 0) {
            h->table[i].hash_or_0 = nz.hash;
            h->table[i].val       = val;
            h->len++;
            break;
        }
    }
    assert(empty_slots_at_least(h,1));
}

struct hash* hash_insert(struct hash *h, unsigned hash, int val) {
    unsigned const len = h ? h->len    : 0,
                   cap = h ? h->mask+1 : 0;
    if (len >= 7*cap/8) {
        unsigned const growth = cap ? 2*cap : 8;
        struct hash   *grown  = calloc(1, sizeof *grown + growth * sizeof *grown->table);
        grown->mask = growth-1;
        for (unsigned i = 0; i < cap; i++) {
            if (h->table[i].hash_or_0 != 0) {
                insert(grown, nonzero(h->table[i].hash_or_0), h->table[i].val);
            }
        }
        assert(grown->len == len);
        free(h);
        h = grown;
    }
    insert(h,nonzero(hash),val);
    return h;
}

bool hash_lookup(struct hash const *h, unsigned hash, bool(*match)(int, void*), void *ctx) {
    if (h) {
        assert(empty_slots_at_least(h,1));
        struct nonzero const nz = nonzero(hash);
        for (unsigned i = nz.hash & h->mask; ; i = (i+1) & h->mask) {
            if (h->table[i].hash_or_0 == 0) { break; }
            if (h->table[i].hash_or_0 == nz.hash && match(h->table[i].val, ctx)) { return true; }
        }
    }
    return false;
}
