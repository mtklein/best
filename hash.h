#pragma once

// Create with hash_insert(NULL,0, ...); clean up with free().
int*  hash_insert(int       *h, int vals, unsigned hash, int val);
_Bool hash_lookup(int const *h, int vals, unsigned hash,
                  _Bool(*match)(int val, void *ctx), void *ctx);
