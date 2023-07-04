#include "tests.h"

typedef uint32_t (*TestFunc)(uint32_t, uint32_t);

static uint16_t code_buf[8];

// test list
struct TestInfo {
    uint16_t opcode;

    uint32_t m_in;

    uint32_t d_out;
};

#define OP(op, imm, rm, rd) (op << 11 | imm << 6 | rm << 3 | rd)

static const struct TestInfo tests[] = {
    {OP(0,  0, 1, 0), 0x55555555, 0x55555555}, // LSL 0
    {OP(0,  0, 1, 0), 0xAAAAAAAA, 0xAAAAAAAA}, // LSL 0
    {OP(0,  1, 1, 0), 0x55555555, 0xAAAAAAAA}, // LSL 1
    {OP(0,  1, 1, 0), 0xAAAAAAAA, 0x55555554}, // LSL 1
    {OP(0, 31, 1, 0), 0x55555555, 0x80000000}, // LSL 31
    {OP(0, 31, 1, 0), 0xAAAAAAAA, 0x00000000}, // LSL 31

    {OP(1,  0, 1, 0), 0x55555555, 0x00000000}, // LSR 0 (32)
    {OP(1,  0, 1, 0), 0xAAAAAAAA, 0x00000000}, // LSR 0 (32)
    {OP(1,  1, 1, 0), 0x55555555, 0x2AAAAAAA}, // LSR 1
    {OP(1,  1, 1, 0), 0xAAAAAAAA, 0x55555555}, // LSR 1
    {OP(1, 31, 1, 0), 0x55555555, 0x00000000}, // LSR 31
    {OP(1, 31, 1, 0), 0xAAAAAAAA, 0x00000001}, // LSR 31

    {OP(2,  0, 1, 0), 0x55555555, 0x00000000}, // ASR 0 (32)
    {OP(2,  0, 1, 0), 0xAAAAAAAA, 0xFFFFFFFF}, // ASR 0 (32)
    {OP(2,  1, 1, 0), 0x55555555, 0x2AAAAAAA}, // ASR 1
    {OP(2,  1, 1, 0), 0xAAAAAAAA, 0xD5555555}, // ASR 1
    {OP(2, 31, 1, 0), 0x55555555, 0x00000000}, // ASR 31
    {OP(2, 31, 1, 0), 0xAAAAAAAA, 0xFFFFFFFF}, // ASR 31
};

#undef OP

static const int num_tests = sizeof(tests) / sizeof(tests[0]);

bool shift_imm_tests(GroupCallback group_cb, FailCallback fail_cb) {

    bool res = true;

    group_cb("sh.imm");

    for(int i = 0; i < num_tests; i++) {
        code_buf[0] = tests[i].opcode;
        code_buf[1] = 0x4770; // BX LR

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

        uint32_t out = func(0xBAD, tests[i].m_in);

        if(out != tests[i].d_out) {
            res = false;
            fail_cb(i, out, tests[i].d_out);
        }
    }

    return res;
}

bool run_tests(GroupCallback group_cb, FailCallback fail_cb) {
    
    bool ret = true;

    ret = shift_imm_tests(group_cb, fail_cb) && ret;

    return ret;
}