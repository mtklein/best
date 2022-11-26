#pragma once

// Create with hash_insert(NULL, ...); clean up with free().
struct hash* hash_insert(struct hash      *, int hash, int val);
_Bool        hash_lookup(struct hash const*, int hash, _Bool(*)(int val, void *ctx), void *ctx);
