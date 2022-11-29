#include "once.h"
#include <stdatomic.h>

void once(char _Atomic *flag, void (*fn)(void *ctx), void *ctx) {
    char state = atomic_load_explicit(flag, memory_order_acquire);
    if (state == 0 && atomic_compare_exchange_strong_explicit(flag, &state, 1,
                                                              memory_order_acquire,
                                                              memory_order_acquire)) {
        fn(ctx);
        atomic_store_explicit(flag, state=2, memory_order_release);
    }
    while (state != 2) { state = atomic_load_explicit(flag, memory_order_acquire); }
}
