#include <cstdint>

#include "32blit.hpp"

#include "tests/tests.h"

static bool failed = false;

static void new_test_group(const char *label) {
    blit::debugf("%s ", label);
}
static void test_failed(int i, uint32_t val, uint32_t expected) {
    failed = true;
    blit::debugf("\n%i: %x != %x\n", i, val, expected);
}

void init() {
    blit::debugf("Starting...\n");

    if(run_tests(new_test_group, test_failed))
        blit::debugf("\nSuccess!\n");
}

void render(uint32_t) {
    // TODO: improve results display
    blit::screen.pen = failed ? blit::Pen{255, 0, 0} : blit::Pen{0, 255, 0};
    blit::screen.clear();
}

void update(uint32_t) {

}