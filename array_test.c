#include "array.h"
#include "test.h"
#include <stddef.h>
#include <stdlib.h>

int main(void) {
    float *ptr = NULL;
    int    len = 0;

    // Push values one at a time.
    for (int i = 0; i < 42; i++) {
        ptr = push_back(ptr, len);
        ptr[len++] = (float)i;
    }
    for (int i = 0; i < 42; i++) {
        expect(equiv(ptr[i], (float)i));
    }

    // Drop values one at a time.
    for (int i = 42; i --> 0;) {
        expect(equiv(ptr[--len], (float)i));
        ptr = drop_back(ptr, len);
    }
    expect(ptr == NULL);
    expect(len == 0);

    // Drop everything at once.
    for (int i = 0; i < 42; i++) {
        ptr = push_back(ptr, len);
        ptr[len++] = (float)i;
    }
    ptr = drop_back(ptr, len=0);
    expect(ptr == NULL);

    // Let's see how big we can go.
    len = 0x40000000;
    ptr = malloc((size_t)len * sizeof *ptr);
    ptr = push_back(ptr, len);
    ptr[len++] = 42.0f;

    drop_back(ptr, len=0);
    return 0;
}
