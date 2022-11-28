#include "once.h"
#include "test.h"
#include <pthread.h>

#define len(arr) (int)(sizeof arr / sizeof *arr)

static void inc_char(void *ctx) {
    *(char*)ctx += 1;
}

struct foo {
    char _Atomic flag;
    char         data;
};

static void* go(void *ctx) {
    struct foo *f = ctx;
    once(&f->flag, inc_char, &f->data);
    return NULL;
}

int main(void) {
    struct foo f = {.data=10};

    pthread_t thread[42];
    for (int i = 0; i < len(thread); i++) {
        pthread_create(thread+i, NULL, go, &f);
    }
    for (int i = 0; i < len(thread); i++) {
        pthread_join(thread[i], NULL);
    }

    expect(f.data == 11);
    return 0;
}
