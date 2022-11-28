#pragma once

// Create with hash_insert(NULL,0, ...); clean up with free().
struct hash* hash_insert(struct hash      *, int vals, unsigned hash, int val);
_Bool        hash_lookup(struct hash const*, int vals, unsigned hash,
                         _Bool(*match)(int val, void *ctx), void *ctx);
