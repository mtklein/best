#pragma once

// Call fn(ctx) once, guarded by flag.
void once(char _Atomic *flag, void (*fn)(void *ctx), void *ctx);
