#include "array.h"
#include "vm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint32_t bits(float f) {
    union {
        float    f;
        uint32_t bits;
    } x = {f};
    return x.bits;
}

int main(int argc, char* argv[]) {
    FILE *in = fopen(argc > 1 ? argv[1] : "prospero.vm", "r");
    int const img_wh = argc > 2 ? atoi(argv[2]) : 1024;

    float const step = 2.0f / (float)(img_wh-1);

    struct builder *b    = builder();
    int            *val  = NULL;
    int             vals = 0;

    char line[1024] = {0};
    while (fgets(line, sizeof line, in)) {
        unsigned id;
        int      skip;
        if (1 != sscanf(line, "_%x %n", &id, &skip)) {
            continue;
        }
        assert((int)id == vals);
        val = push_back(val, vals++);
        char const *c = line + skip;

        float f;
        if (1 == sscanf(c, "const %f\n", &f)) { val[id] = imm(b, bits(f)); continue; }

        if (0 == strcmp(c, "var-x\n")) {
            val[id] = fadd(b, fmul(b, i2f(b, idx(b))
                                    , imm(b, bits(step)))
                            , imm(b, bits(-1.0f)));
            continue;
        }
        if (0 == strcmp(c, "var-y\n")) {
            val[id] = ld(b, imm(b,1), imm(b,0));
            continue;
        }

        unsigned x,y;
        if (2 == sscanf(c, "add _%x _%x\n", &x,&y)) {
            val[id] = fadd(b, val[x], val[y]);
            continue;
        }
        if (2 == sscanf(c, "sub _%x _%x\n", &x,&y)) {
            val[id] = fsub(b, val[x], val[y]);
            continue;
        }
        if (2 == sscanf(c, "mul _%x _%x\n", &x,&y)) {
            val[id] = fmul(b, val[x], val[y]);
            continue;
        }
        if (2 == sscanf(c, "min _%x _%x\n", &x,&y)) {
            val[id] = bsel(b, flt(b, val[x], val[y]), val[x], val[y]);
            continue;
        }
        if (2 == sscanf(c, "max _%x _%x\n", &x,&y)) {
            val[id] = bsel(b, flt(b, val[x], val[y]), val[y], val[x]);
            continue;
        }
        if (1 == sscanf(c, "square _%x\n", &x)) {
            val[id] = fmul(b, val[x], val[x]);
            continue;
        }
        if (1 == sscanf(c, "neg _%x\n", &x)) {
            val[id] = fsub(b, imm(b, bits(0.0f)), val[x]);
            continue;
        }
        if (1 == sscanf(c, "sqrt _%x\n", &x)) {
            val[id] = fsqrt(b, val[x]);
            continue;
        }
    }
    fclose(in);

    st(b, imm(b,0), imul(b, imm(b,4), idx(b)), val[vals-1]);
    free(val);
    struct program *p = ret(b);
    void *scratch = NULL;

    float *img = calloc((size_t)(img_wh*img_wh), sizeof *img);
    for (int j = 0; j < img_wh; j++) {
        float y = +1.0f - (float)j*step;
        scratch = run(p, img_wh, (void*[]){img + j*img_wh, &y}, scratch);
    }
    free(p);
    free(scratch);

    dprintf(1, "P5\n%d %d\n255\n", img_wh, img_wh);
    for (int i = 0; i < img_wh*img_wh; i++) {
        ((char*)img)[i] = img[i] < 0 ? 0xff : 0x00;
    }
    write(1, img, (size_t)(img_wh*img_wh));
    free(img);

    return 0;
}
