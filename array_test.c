#include "array.h"
#include "test.h"
#include <stddef.h>

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
    ptr = drop_back(ptr,0);
    expect(ptr == NULL);

    return 0;
}
