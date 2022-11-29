#pragma once

struct hash {
    unsigned  vals,slots;
    unsigned *hash;
    int      *val;
};

void  hash_insert(struct hash      *, unsigned hash, int val);
_Bool hash_lookup(struct hash const*, unsigned hash, _Bool(*)(int val, void *ctx), void *ctx);
