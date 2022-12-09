#include "once.h"
#include <stdatomic.h>

// A thread can take five paths through once(), depending on what happens at (A) and (B).
//
// Lucky at (A) and (B)
//   Exactly one thread sees state=ZERO at (A) and successfully swaps to state=BUSY at (B).
//   This lucky thread calls fn(ctx) and finishes by releasing state=DONE at (C), marking a
//   sync point for other threads to acquire up to.
//
// DONE at (A)
//   Threads that see state=DONE at (A) are on the steady-state fast path and can simply exit.
//   The lucky thread called fn(ctx) long ago, and the acquire at (A) already synced up to (C).
//
// BUSY at (A)
//   Threads that see state=BUSY at (A) raced to become the lucky thread but lost that race.
//   They wait at (D) for the lucky thread to finish.
//
// Threads that see state=ZERO at (A) but fail to swap to state=BUSY at (B) will now see
// state=DONE or state=BUSY.  Each state works exactly like it did above,
//    DONE at (B):  just like "DONE at (A)", simply exit;
//    BUSY at (B):  just like "BUSY at (A)", wait at (D) for the lucky thread to finish.
//
// The memory orders for loads and stores at (A), (C), and (D) should now seem straightforward.
// Acquiring on failure at (B) syncs "DONE at (B)" threads to the state released at (C).
// Relaxed order would be logically fine on success at (B), but the API demands at least acquire.
//
// TODO: sleep at (D) instead of spinning?

void once(char _Atomic *flag, void (*fn)(void *ctx), void *ctx) {
    enum { ZERO, DONE, BUSY };

    char state = atomic_load_explicit(flag, memory_order_acquire);                        // (A)
    if (state == ZERO && atomic_compare_exchange_strong_explicit(flag, &state, BUSY,      // (B)
                                                                 memory_order_acquire,
                                                                 memory_order_acquire)) {
        fn(ctx);
        atomic_store_explicit(flag, state=DONE, memory_order_release);                    // (C)
    }
    while (state != DONE) { state = atomic_load_explicit(flag, memory_order_acquire); }   // (D)
}
