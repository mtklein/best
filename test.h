#pragma once

int dprintf(int, char const*, ...);
#define expect(x) \
    if (!(x)) dprintf(2, "%s:%d expect(%s)\n", __FILE__, __LINE__, #x), __builtin_trap()

static inline _Bool equiv(float x, float y) {
    return (x <= y && y <= x)
        || (x != x && y != y);
}
