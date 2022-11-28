#include "once.h"
#include <stdatomic.h>

void once(char _Atomic *flag, void (*fn)(void *ctx), void *ctx) {
    char state = atomic_load_explicit(flag, memory_order_acquire);
    if (state == 0 && atomic_compare_exchange_strong_explicit(flag, &state, 1,
                                                              memory_order_relaxed,
                                                              memory_order_relaxed)) {
        fn(ctx);
        atomic_store_explicit(flag, 2, memory_order_release);
    }
    while (atomic_load_explicit(flag, memory_order_acquire) != 2) /*spin*/;
}
