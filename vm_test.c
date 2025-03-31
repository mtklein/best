#include "test.h"
#include "vm.h"
#include <math.h>
#include <stdlib.h>

int main(void) {
    struct program *p = NULL;
    {
        struct builder *b = builder();

        int const off = imul(b, idx(b), imm(b,4)),
                    x = ld(b, imm(b,0), off),
                    y = ld(b, imm(b,1), off),
                    z = fsqrt(b, fmul(b, x,y));
        st(b, imm(b,0), off, z);
        p = ret(b);
    }

    #define N 42
    float x[N],y[N];
    for (int i = 0; i < N; i++) {
        x[i] = (float)i;
        y[i] = (float)i + N;
    }

    run(p, N, (void*[]){x,y});

    for (int i = 0; i < N; i++) {
        expect(equiv(x[i], sqrtf((float)i * (float)(i + N)) ));
    }

    free(p);
    return 0;
}
