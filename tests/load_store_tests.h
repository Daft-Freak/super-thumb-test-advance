#pragma once
#include "tests.h"
#include "tests_common.h"

bool run_single_load_store_tests(GroupCallback group_cb, FailCallback fail_cb);
bool run_push_pop_tests(GroupCallback group_cb, FailCallback fail_cb, const char *label);
bool run_ldm_stm_tests(GroupCallback group_cb, FailCallback fail_cb, const char *label);

bool run_ldm_stm_thumb2_tests(GroupCallback group_cb, FailCallback fail_cb, const char *label);