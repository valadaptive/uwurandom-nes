#ifndef _UWURANDOM_PLATFORM_H
#define _UWURANDOM_PLATFORM_H
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "uwurandom_types.h"

static int uwu_init_rng(uwu_state * state) {
    (void)state;
    return 0;
}

static void uwu_destroy_rng(uwu_state* state) {
    (void)state;
}

static uint16_t rand_state = 0;

void get_random_bytes(void* buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        rand_state = rand_state * 1105245 + 12345;
        ((char*)buf)[i] = (char)(rand_state >> 8);
    }
}

static uwu_random_number uwu_random_int(uwu_state* state) {
    (void)state;
    uwu_random_number rand_value = 0;
    get_random_bytes((char*)&rand_value, sizeof(uwu_random_number));
    return rand_value;
}

#define COPY_STR(dst, src, len) do {\
    memcpy((dst), (src), (len));\
} while (0)

#define COPY_CHAR(value, dst) do {\
    *(dst) = (value);\
} while (0)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif // _UWURANDOM_PLATFORM_H
