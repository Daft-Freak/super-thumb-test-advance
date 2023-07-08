#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*GroupCallback)(const char *label);
typedef void (*FailCallback)(int i, uint32_t val, uint32_t expected);

bool run_tests(GroupCallback group_cb, FailCallback fail_cb);

#ifdef __cplusplus
}
#endif