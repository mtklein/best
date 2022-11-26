#include "hash.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct hash {
    int len,mask;
    struct {
        int hash,val;
    } table[];
};

static void just_insert(struct hash *h, int hash, int val) {
    int const mask = h ? h->mask : -1;
    assert(h);
    assert(hash);
    for (int n = 0, i = hash&mask; n++ <= mask; i = (i+1)&mask) {
        if (h->table[i].hash == 0) {
            h->table[i].hash = hash;
            h->table[i].val  = val;
            h->len++;
            return;
        }
    }
    assert(false);
}

struct hash* hash_insert(struct hash *h, int user, int val) {
    int const hash = user ? user : 1,
               len = h ? h->len    : 0,
               cap = h ? h->mask+1 : 0;
    if (len >= 3*cap/4) {
        int const growth = cap ? 2*cap : 1;
        struct hash *grown = calloc(1, sizeof *grown + (size_t)growth * sizeof *grown->table);
        grown->mask = growth-1;

        for (int i = 0; i < cap; i++) {
            if (h->table[i].hash) {
                just_insert(grown, h->table[i].hash, h->table[i].val);
            }
        }
        assert(len == grown->len);
        free(h);
        h = grown;
    }
    just_insert(h,hash,val);
    return h;
}

bool hash_lookup(struct hash const *h, int user, bool(*match)(int val, void *ctx), void *ctx) {
    int const hash = user ? user : 1,
              mask = h ? h->mask : -1;
    for (int n = 0, i = hash&mask; n++ <= mask; i = (i+1)&mask) {
        if (h->table[i].hash == 0) { break; }
        if (h->table[i].hash == hash && match(h->table[i].val, ctx)) { return true; }
    }
    return false;
}
