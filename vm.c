#include "array.h"
#include "hash.h"
#include "vm.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// TODO:
//   [ ] misc. strength reduction

#define K 16
typedef float    __attribute__((vector_size(4*K))) F32;
typedef int32_t  __attribute__((vector_size(4*K))) I32;
typedef uint32_t __attribute__((vector_size(4*K))) U32;

union val {
    U32 u32;
    I32 i32;
    F32 f32;
};

struct pinst {
    void (*fn)(struct pinst const *ip, union val *r, union val const *v,
               int i, int lanes, void* ptr[]);
    int x,y,z;
    uint32_t imm;
};

#define defn(name) void fn_##name(struct pinst const *ip, union val *r, union val const *v, \
                                  int i, int lanes, void* ptr[])
#define next ip[1].fn(ip+1,r+1,v,i,lanes,ptr); return

struct binst {
    void (*fn)(struct pinst const *ip, union val *r, union val const *v,
               int i, int lanes, void* ptr[]);
    int x,y,z;
    uint32_t imm;

    enum { IMM, UNI, VAR, MEM } kind :  2;
    _Bool                       live :  1;
    int                         pad  : 29;
    int                         id;
};

struct builder {
    int           insts,padding;
    struct binst *inst;
    struct hash   cse;
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

struct cse_ctx {
    struct builder const *b;
    struct binst   const *inst;
    int                   id,padding;
};

static _Bool match_cse(int id, void *vctx) {
    struct cse_ctx *ctx = vctx;
    if (0 == memcmp(ctx->b->inst+id, ctx->inst, sizeof *ctx->inst)) {
        ctx->id = id;
        return 1;
    }
    return 0;
}

static unsigned fnv1a(void const *v, size_t len) {
    unsigned hash = 0x811c9dc5;
    for (unsigned char const *b=v, *end=b+len; b != end; b++) {
        hash ^= *b;
        __builtin_mul_overflow(hash, 0x01000193, &hash);
    }
    return hash;
}

static int push_(struct builder *b, struct binst inst) {
    if (inst.kind < b->inst[inst.x].kind) { inst.kind = b->inst[inst.x].kind; }
    if (inst.kind < b->inst[inst.y].kind) { inst.kind = b->inst[inst.y].kind; }
    if (inst.kind < b->inst[inst.z].kind) { inst.kind = b->inst[inst.z].kind; }

    if (inst.kind == IMM && (inst.x || inst.y || inst.z)) {
        union val v[4] = {
            {{b->inst[inst.x].imm}},
            {{b->inst[inst.y].imm}},
            {{b->inst[inst.z].imm}},
            {{0}},
        };
        struct pinst ip[] = {
            {.fn=inst.fn, .x=0, .y=1, .z=2, .imm=inst.imm},
            {.fn=fn_ret},
        };
        ip->fn(ip,v+3,v,0,1,NULL);
        return imm(b, v[3].u32[0]);
    }

    unsigned const hash = fnv1a(&inst, sizeof inst);
    struct cse_ctx cse_ctx = {.b=b,.inst=&inst};
    if (hash_lookup(b->cse, hash, match_cse, &cse_ctx)) {
        return cse_ctx.id;
    }
    b->inst = push_back(b->inst, b->insts);
    b->inst[b->insts] = inst;

    int const id = b->insts++;
    if (inst.kind < MEM) {
        hash_insert(&b->cse, hash, id);
    }
    return id;
}
#define push(b, ...) push_(b, (struct binst){.fn=__VA_ARGS__})


static defn(imm) {
    r->u32 = (U32){0} + ip->imm;
    next;
}
int imm(struct builder *b, uint32_t bits) { return push(b, fn_imm, .imm=bits, .kind=IMM); }

static defn(idx) {
    for (int l = 0; l < lanes; l++) {
        r->i32[l] = l + i;
    }
    next;
}
int idx(struct builder *b) { return push(b, fn_idx, .kind=VAR); }

static defn(gather) {
    typedef int32_t __attribute__((aligned(1))) unaligned_i32;
    for (int l = 0; l < lanes; l++) {
        char const* p = (char const*)ptr[ v[ip->x].i32[l] ] + v[ip->y].i32[l];
        r->i32[l] = *(unaligned_i32 const*)p;
    }
    next;
}
int ld(struct builder *b, int ptr, int off) {
    return push(b, fn_gather, .x=ptr, .y=off, .kind=MEM);
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
    (void)push(b, fn_scatter, .x=ptr, .y=off, .z=val, .kind=MEM, .live=1);
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
    for (int l = 0; l < lanes; l++) {
        r->f32[l] = sqrtf(v[ip->x].f32[l]);
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

struct builder* builder(void) {
    struct builder *b = calloc(1, sizeof *b);
    b->insts = 1;
    b->inst  = calloc(1, sizeof *b->inst);
    return b;
}

struct program {
    int          insts,loop;
    struct pinst inst[];
};

struct program* ret(struct builder *b) {
    push(b, fn_ret, .kind=MEM, .live=1);

    int live = 0;
    for (struct binst *inst = b->inst+b->insts; inst --> b->inst;) {
        if (inst->live) {
            b->inst[inst->x].live = 1;
            b->inst[inst->y].live = 1;
            b->inst[inst->z].live = 1;
        } else {
            inst->fn = NULL;
        }
        live += (inst->fn != NULL);
    }

    struct program *p = calloc(1, sizeof *p + (size_t)live * sizeof *p->inst);

    for (int varying = 0; varying < 2; varying++) {
        if (varying) {
            p->loop = p->insts;
        }
        for (struct binst *inst = b->inst; inst < b->inst+b->insts; inst++) {
            if (inst->fn && varying == (inst->kind >= VAR)) {
                struct pinst *pinst = p->inst + p->insts;
                pinst->fn  = inst->fn;
                pinst->x   = b->inst[inst->x].id;
                pinst->y   = b->inst[inst->y].id;
                pinst->z   = b->inst[inst->z].id;
                pinst->imm = inst->imm;

                inst->id = p->insts++;
            }
        }
    }

    free(b->inst);
    free(b->cse.data);
    free(b);
    return p;
}

void run(struct program const *p, int N, void* ptr[]) {
    union val *v = calloc((size_t)p->insts, sizeof *v);

    struct pinst const *start = p->inst, *loop  = start + p->loop;
    union  val         *r     = v,       *loopr =     r + p->loop;

    int i = 0;
    while (i < N/K*K) { start->fn(start,r,v,i,  K,ptr); i += K; start=loop; r=loopr; }
    if    (i < N    ) { start->fn(start,r,v,i,N-i,ptr);                              }

    free(v);
}
