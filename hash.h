#pragma once

struct hash {
    int       vals;
    unsigned  mask;
    unsigned *hash;
    int      *val;
};

void  hash_insert(struct hash      *, unsigned hash, int val);
_Bool hash_lookup(struct hash const*, unsigned hash, _Bool(*match)(int val, void *ctx), void *ctx);
