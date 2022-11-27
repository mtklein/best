#pragma once

// Create with hash_insert(NULL, ...); clean up with free().
struct hash* hash_insert(struct hash      *, unsigned, int val);
_Bool        hash_lookup(struct hash const*, unsigned, _Bool(*)(int val, void *ctx), void *ctx);
