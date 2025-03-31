#pragma once

// Call fn(ctx) exactly once, guarded by *flag.
//
// once() is like call_once() with extra advantages:
//    - once() is available even when <threads.h> is not;
//    - once() takes a ctx argument, allowing one-time initialization in non-global contexts.
// Initialize *flag=0.
void once(char _Atomic *flag, void (*fn)(void *ctx), void *ctx);
