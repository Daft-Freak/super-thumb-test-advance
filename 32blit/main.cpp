#include <cstdint>

#include "32blit.hpp"

#include "tests/tests.h"

static void new_test_group(const char *label) {
    blit::debugf("%s ", label);
}
static void test_failed(int i, uint32_t val, uint32_t expected) {
    blit::debugf("\n%i: %x != %x\n", i, val, expected);
}


void init() {
    blit::debugf("Starting...\n");

    if(run_tests(new_test_group, test_failed))
        blit::debugf("\nSuccess!\n");
}

void render(uint32_t) {

}

void update(uint32_t) {

}