#include "test.h"
#include <math.h>

struct iv {
    float lo,hi;
};

static struct iv iv_add(struct iv x, struct iv y) {
    return (struct iv){x.lo + y.lo, x.hi + y.hi};
}
static struct iv iv_sub(struct iv x, struct iv y) {
    return (struct iv){x.lo - y.hi, x.hi - y.lo};
}
static struct iv iv_mul(struct iv x, struct iv y) {
    float const a = x.lo * y.lo,
                b = x.lo * y.hi,
                c = x.hi * y.lo,
                d = x.hi * y.hi;

    return (struct iv){ fminf(fminf(a,b), fminf(c,d)),
                        fmaxf(fmaxf(a,b), fmaxf(c,d)) };
}

static void test_add(void) {
    struct iv z = iv_add((struct iv){3,4},
                         (struct iv){1,2});
    expect(equiv(4, z.lo));
    expect(equiv(6, z.hi));
}

static void test_sub(void) {
    struct iv z = iv_sub((struct iv){3,4},
                         (struct iv){1,2});
    expect(equiv(1, z.lo));
    expect(equiv(3, z.hi));
}

static void test_mul(void) {
    {
        struct iv z = iv_mul((struct iv){+3,+4},
                             (struct iv){+1,+2});
        expect(equiv(+3, z.lo));
        expect(equiv(+8, z.hi));
    }
    {
        struct iv z = iv_mul((struct iv){-3,+4},
                             (struct iv){+1,+2});
        expect(equiv(-6, z.lo));
        expect(equiv(+8, z.hi));
    }
    {
        struct iv z = iv_mul((struct iv){+3,-4},
                             (struct iv){+1,+2});
        expect(equiv(-8, z.lo));
        expect(equiv(+6, z.hi));
    }
    {
        struct iv z = iv_mul((struct iv){-3,-4},
                             (struct iv){+1,+2});
        expect(equiv(-8, z.lo));
        expect(equiv(-3, z.hi));
    }
    {
        struct iv z = iv_mul((struct iv){-3,-4},
                             (struct iv){-1,+2});
        expect(equiv(-8, z.lo));
        expect(equiv(+4, z.hi));
    }
}

int main(void) {
    test_add();
    test_sub();
    test_mul();
    return 0;
}
