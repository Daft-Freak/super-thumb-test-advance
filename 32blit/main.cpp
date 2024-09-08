#include <cstdint>

#include "32blit.hpp"

#include "tests/tests.h"

static bool failed = false;

static std::vector<std::tuple<std::string_view, bool>> test_groups; // name, failed

static void new_test_group(const char *label) {
    blit::debugf("%s ", label);

    test_groups.push_back({label, false});
}
static void test_failed(int i, uint32_t val, uint32_t expected) {
    failed = true;
    std::get<1>(test_groups.back()) = true;

    blit::debugf("\n%i: %x != %x\n", i, val, expected);
}

void init() {
    blit::debugf("Starting...\n");

    if(run_tests(new_test_group, test_failed))
        blit::debugf("\nSuccess!\n");
}

void render(uint32_t) {
    blit::screen.pen = {0, 0, 0};
    blit::screen.clear();

    blit::screen.pen = {255, 255, 255};
    blit::Point p{4, 4};
    for(auto &group : test_groups) {
        auto &group_name = std::get<0>(group);

        blit::screen.pen = std::get<1>(group) ? blit::Pen{255, 0, 0} : blit::Pen{0, 255, 0};

        auto text_size = blit::screen.measure_text(group_name, blit::minimal_font);

        if(p.x + text_size.w > blit::screen.bounds.w)
        {
            p.x = 4;
            p.y += 8;
        }

        blit::screen.text(group_name, blit::minimal_font, p);

        p.x += text_size.w + 4;
    }
}

void update(uint32_t) {

}