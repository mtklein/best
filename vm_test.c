#include "test.h"
#include "vm.h"
#include <math.h>
#include <stdlib.h>

static void test_simple(void) {
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
}

static void test_cse(void) {
    struct builder *b = builder();
    int x = imm(b, 42),
        y = imm(b, 42);
    expect(x == y);
    free(ret(b));
}

static void test_constant_prop(void) {
    struct builder *b = builder();
    int x =  imm(b, 42),
        y =  imm(b, 43),
        z =  imm(b, 85),
        w = iadd(b, x,y);
    expect(z == w);
    free(ret(b));
}

static void test_symmetric(void) {
    struct builder *b = builder();
    int x = imm(b, 42),
        y = idx(b);
    expect(x != y);
    int z = iadd(b, x,y),
        w = iadd(b, y,x);
    expect(z == w);
    free(ret(b));
}

int main(void) {
    test_simple();
    test_cse();
    test_constant_prop();
    test_symmetric();
    return 0;
}
