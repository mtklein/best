#pragma once
#include <stdint.h>

struct builder* builder(void);

int imm(struct builder*, uint32_t bits);
int idx(struct builder*);
int  ld(struct builder*, int ptr, int off);
void st(struct builder*, int ptr, int off, int val);

int iadd(struct builder*, int,int);
int isub(struct builder*, int,int);
int imul(struct builder*, int,int);

int fadd(struct builder*, int,int);
int fsub(struct builder*, int,int);
int fmul(struct builder*, int,int);
int fdiv(struct builder*, int,int);
int fsqrt(struct builder*, int);

int i2f(struct builder*, int);
int f2i(struct builder*, int);

int ieq(struct builder*, int,int);
int ilt(struct builder*, int,int);
int ile(struct builder*, int,int);

int feq(struct builder*, int,int);
int flt(struct builder*, int,int);
int fle(struct builder*, int,int);

int bshl(struct builder*, int,int);
int bshr(struct builder*, int,int);
int bsra(struct builder*, int,int);

int band(struct builder*, int,int);
int bor (struct builder*, int,int);
int bxor(struct builder*, int,int);
int bsel(struct builder*, int,int,int);

struct program* ret(struct builder*);
void* run(struct program const*, int N, void* ptr[], void *scratch);
