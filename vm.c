#include "array.h"
#include "vm.h"
#include <math.h>
#include <stdlib.h>

// TODO:
//   [ ] dead code elimination
//   [ ] constant propagation
//   [ ] common subexpression elimination
//   [ ] loop-invariant hoisting
//   [ ] misc. strength reduction

#define K 16
typedef float    __attribute__((vector_size(4*K))) F32;
typedef int32_t  __attribute__((vector_size(4*K))) I32;
typedef uint32_t __attribute__((vector_size(4*K))) U32;

union val {
    F32 f32;
    I32 i32;
    U32 u32;
};

struct inst {
    void (*fn)(struct inst const *ip, union val *r, union val const *v,
               int i, int lanes, void* ptr[]);
    int x,y,z;
    uint32_t imm;
};

struct builder {
    int          insts,unused;
    struct inst *inst;
};

struct builder* builder(void) {
    struct builder *b = calloc(1, sizeof *b);
    return b;
}

static int push_(struct builder *b, struct inst inst) {
    b->inst = push_back(b->inst, b->insts);
    b->inst[b->insts] = inst;
    return b->insts++;
}
#define push(b, ...) push_(b, (struct inst){.fn=__VA_ARGS__})

#define defn(name) void fn_##name(struct inst const *ip, union val *r, union val const *v, \
                                  int i, int lanes, void* ptr[])
#define next ip[1].fn(ip+1,r+1,v,i,lanes,ptr); return

static defn(imm) {
    r->u32 = (U32){0} + ip->imm;
    next;
}
int imm(struct builder *b, uint32_t bits) { return push(b, fn_imm, .imm=bits); }

static defn(idx) {
    _Static_assert(K == 16, "");
    r->i32 = (I32){0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15} + i;
    next;
}
int idx(struct builder *b) { return push(b, fn_idx); }

static defn(gather) {
    typedef int32_t __attribute__((aligned(1))) unaligned_i32;
    for (int l = 0; l < lanes; l++) {
        char const* p = (char const*)ptr[ v[ip->x].i32[l] ] + v[ip->y].i32[l];
        r->i32[l] = *(unaligned_i32 const*)p;
    }
    next;
}
int ld(struct builder *b, int ptr, int off) {
    return push(b, fn_gather, .x=ptr, .y=off);
}

static defn(scatter) {
    typedef int32_t __attribute__((aligned(1))) unaligned_i32;
    for (int l = 0; l < lanes; l++) {
        char* p = (char*)ptr[ v[ip->x].i32[l] ] + v[ip->y].i32[l];
        *(unaligned_i32*)p = v[ip->z].i32[l];
    }
    next;
}
void st(struct builder *b, int ptr, int off, int val) {
    (void)push(b, fn_scatter, .x=ptr, .y=off, .z=val);
}

static defn(iadd) { r->i32 = v[ip->x].i32 + v[ip->y].i32; next; }
static defn(isub) { r->i32 = v[ip->x].i32 - v[ip->y].i32; next; }
static defn(imul) { r->i32 = v[ip->x].i32 * v[ip->y].i32; next; }

static defn(fadd) { r->f32 = v[ip->x].f32 + v[ip->y].f32; next; }
static defn(fsub) { r->f32 = v[ip->x].f32 - v[ip->y].f32; next; }
static defn(fmul) { r->f32 = v[ip->x].f32 * v[ip->y].f32; next; }
static defn(fdiv) { r->f32 = v[ip->x].f32 / v[ip->y].f32; next; }

int iadd(struct builder *b, int x, int y) { return push(b, fn_iadd, .x=x, .y=y); }
int isub(struct builder *b, int x, int y) { return push(b, fn_isub, .x=x, .y=y); }
int imul(struct builder *b, int x, int y) { return push(b, fn_imul, .x=x, .y=y); }

int fadd(struct builder *b, int x, int y) { return push(b, fn_fadd, .x=x, .y=y); }
int fsub(struct builder *b, int x, int y) { return push(b, fn_fsub, .x=x, .y=y); }
int fmul(struct builder *b, int x, int y) { return push(b, fn_fmul, .x=x, .y=y); }
int fdiv(struct builder *b, int x, int y) { return push(b, fn_fdiv, .x=x, .y=y); }

static defn(fsqrt) {
    for (int l = 0; l < K; l++) {
        r->f32[l] = v[ip->x].f32[l];
    }
    next;
}
int fsqrt(struct builder *b, int x) { return push(b, fn_fsqrt, .x=x); }

static defn(i2f) { r->f32 = __builtin_convertvector(v[ip->x].i32, F32); next; }
static defn(f2i) { r->i32 = __builtin_convertvector(v[ip->x].f32, I32); next; }
int i2f(struct builder *b, int x) { return push(b, fn_i2f, .x=x); }
int f2i(struct builder *b, int x) { return push(b, fn_f2i, .x=x); }

static defn(ieq) { r->i32 = (I32)(v[ip->x].i32 == v[ip->y].i32); next; }
static defn(ilt) { r->i32 = (I32)(v[ip->x].i32 <  v[ip->y].i32); next; }
static defn(ile) { r->i32 = (I32)(v[ip->x].i32 <= v[ip->y].i32); next; }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
static defn(feq) { r->i32 = (I32)(v[ip->x].f32 == v[ip->y].f32); next; }
static defn(flt) { r->i32 = (I32)(v[ip->x].f32 <  v[ip->y].f32); next; }
static defn(fle) { r->i32 = (I32)(v[ip->x].f32 <= v[ip->y].f32); next; }
#pragma GCC diagnostic pop

int ieq(struct builder *b, int x, int y) { return push(b, fn_ieq, .x=x, .y=y); }
int ilt(struct builder *b, int x, int y) { return push(b, fn_ilt, .x=x, .y=y); }
int ile(struct builder *b, int x, int y) { return push(b, fn_ile, .x=x, .y=y); }

int feq(struct builder *b, int x, int y) { return push(b, fn_feq, .x=x, .y=y); }
int flt(struct builder *b, int x, int y) { return push(b, fn_flt, .x=x, .y=y); }
int fle(struct builder *b, int x, int y) { return push(b, fn_fle, .x=x, .y=y); }

static defn(bshl) { r->i32 = v[ip->x].i32 << v[ip->y].i32; next; }
static defn(bshr) { r->u32 = v[ip->x].u32 >> v[ip->y].i32; next; }
static defn(bsra) { r->i32 = v[ip->x].i32 >> v[ip->y].i32; next; }
static defn(band) { r->i32 = v[ip->x].i32 &  v[ip->y].i32; next; }
static defn(bor ) { r->i32 = v[ip->x].i32 |  v[ip->y].i32; next; }
static defn(bxor) { r->i32 = v[ip->x].i32 ^  v[ip->y].i32; next; }

int bshl(struct builder *b, int x, int y) { return push(b, fn_bshl, .x=x, .y=y); }
int bshr(struct builder *b, int x, int y) { return push(b, fn_bshr, .x=x, .y=y); }
int bsra(struct builder *b, int x, int y) { return push(b, fn_bsra, .x=x, .y=y); }
int band(struct builder *b, int x, int y) { return push(b, fn_band, .x=x, .y=y); }
int bor (struct builder *b, int x, int y) { return push(b, fn_bor , .x=x, .y=y); }
int bxor(struct builder *b, int x, int y) { return push(b, fn_bxor, .x=x, .y=y); }

static defn(bsel) {
    r->i32 = ( v[ip->x].i32 & v[ip->y].i32)
           | (~v[ip->x].i32 & v[ip->z].i32);
    next;
}
int bsel(struct builder *b, int x, int y, int z) { return push(b, fn_bsel, .x=x, .y=y, .z=z); }

struct program {
    int         insts,unused;
    struct inst inst[];
};

static defn(ret) {
    (void)ip;
    (void)r;
    (void)v;
    (void)i;
    (void)lanes;
    (void)ptr;
    return;
}
struct program* ret(struct builder *b) {
    push(b, fn_ret);

    struct program *p = calloc(1, sizeof *p + (size_t)b->insts * sizeof *b->inst);
    for (int i = 0; i < b->insts; i++) {
        p->inst[p->insts++] = b->inst[i];
    }

    free(b->inst);
    free(b);
    return p;
}

void run(struct program const *p, int N, void* ptr[]) {
    union val *v = calloc((size_t)p->insts, sizeof *v);

    int i = 0;
    while (i < N/K*K) { p->inst->fn(p->inst,v,v,i,  K,ptr); i += K; }
    if    (i < N    ) { p->inst->fn(p->inst,v,v,i,N-i,ptr);         }

    free(v);
}
