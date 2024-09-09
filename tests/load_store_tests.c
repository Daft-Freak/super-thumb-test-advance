#include "load_store_tests.h"

static const uint32_t test_data_init[4] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210};
static uint32_t test_data[4];
static const uint32_t test_data_addr = (uintptr_t)test_data;

// load byte/word, register offset
#define OP(byte, ro, rb, rd) (0x5800 | byte << 10 | ro << 6 | rb << 3 | rd)

const struct TestInfo load_reg_tests[] = {
    // ldr r0 [r2 r1]
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr    , 0x01234567, 0, 0}, // 0
    {OP(0, 1, 2, 0), 0x00000004, test_data_addr    , 0x89ABCDEF, 0, 0},
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr + 4, 0x89ABCDEF, 0, 0},
    {OP(0, 1, 2, 0), 0xFFFFFFFC, test_data_addr + 4, 0x01234567, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP(0, 1, 2, 0), 0x00000001, test_data_addr    , 0x67012345, 0, 0},
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr + 1, 0x67012345, 0, 0},
    {OP(0, 1, 2, 0), 0x00000002, test_data_addr    , 0x45670123, 0, 0},
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr + 2, 0x45670123, 0, 0},
    {OP(0, 1, 2, 0), 0x00000003, test_data_addr    , 0x23456701, 0, 0},
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr + 3, 0x23456701, 0, 0},
#endif

    // ldrb r0 [r2 r1]
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr    , 0x00000067, 0, 0}, // 10
    {OP(1, 1, 2, 0), 0x00000001, test_data_addr    , 0x00000045, 0, 0},
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr + 1, 0x00000045, 0, 0},
    {OP(1, 1, 2, 0), 0x00000002, test_data_addr    , 0x00000023, 0, 0},
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr + 2, 0x00000023, 0, 0},
    {OP(1, 1, 2, 0), 0x00000003, test_data_addr    , 0x00000001, 0, 0},
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr + 3, 0x00000001, 0, 0},
    {OP(1, 1, 2, 0), 0x00000004, test_data_addr    , 0x000000EF, 0, 0},
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr + 4, 0x000000EF, 0, 0},
    {OP(1, 1, 2, 0), 0xFFFFFFFF, test_data_addr + 1, 0x00000067, 0, 0},
};

#undef OP

static const int num_load_reg_tests = sizeof(load_reg_tests) / sizeof(load_reg_tests[0]);

// store byte/word, register offset
#define OP(byte, ro, rb, rd) (0x5000 | byte << 10 | ro << 6 | rb << 3 | rd)

static const struct TestInfo store_reg_tests[] = {
    // str r0 [r2 r1]
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr    , 0x7E57DA7A, 0, 0}, // 0
    {OP(0, 1, 2, 0), 0x00000004, test_data_addr    , 0x7E57DA7A, 0, 0},
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr + 4, 0x7E57DA7A, 0, 0},
    {OP(0, 1, 2, 0), 0xFFFFFFFC, test_data_addr + 4, 0x7E57DA7A, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP(0, 1, 2, 0), 0x00000001, test_data_addr    , 0x7E57DA7A, 0, 0},
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr + 1, 0x7E57DA7A, 0, 0},
    {OP(0, 1, 2, 0), 0x00000002, test_data_addr    , 0x7E57DA7A, 0, 0},
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr + 2, 0x7E57DA7A, 0, 0},
    {OP(0, 1, 2, 0), 0x00000003, test_data_addr    , 0x7E57DA7A, 0, 0},
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr + 3, 0x7E57DA7A, 0, 0},
#endif

    // strb r0 [r2 r1]
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr    , 0x0123457A, 0, 0}, // 10
    {OP(1, 1, 2, 0), 0x00000001, test_data_addr    , 0x01237A67, 0, 0},
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr + 1, 0x01237A67, 0, 0},
    {OP(1, 1, 2, 0), 0x00000002, test_data_addr    , 0x017A4567, 0, 0},
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr + 2, 0x017A4567, 0, 0},
    {OP(1, 1, 2, 0), 0x00000003, test_data_addr    , 0x7A234567, 0, 0},
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr + 3, 0x7A234567, 0, 0},
    {OP(1, 1, 2, 0), 0x00000004, test_data_addr    , 0x89ABCD7A, 0, 0},
    {OP(1, 1, 2, 0), 0x00000000, test_data_addr + 4, 0x89ABCD7A, 0, 0},
    {OP(1, 1, 2, 0), 0xFFFFFFFF, test_data_addr + 1, 0x0123457A, 0, 0},
};

#undef OP

static const int num_store_reg_tests = sizeof(store_reg_tests) / sizeof(store_reg_tests[0]);

// load sign extended byte/half, register offset
#define OP(h, s, ro, rb, rd) (0x5200 | h << 11 | s << 10 | ro << 6 | rb << 3 | rd)

static const struct TestInfo load_sign_ex_tests[] = {
    // ldrh r0 [r2 r1]
    {OP(1, 0, 1, 2, 0), 0x00000000, test_data_addr    , 0x00004567, 0, 0}, // 0
    {OP(1, 0, 1, 2, 0), 0x00000002, test_data_addr    , 0x00000123, 0, 0},
    {OP(1, 0, 1, 2, 0), 0x00000000, test_data_addr + 2, 0x00000123, 0, 0},
    {OP(1, 0, 1, 2, 0), 0x00000004, test_data_addr    , 0x0000CDEF, 0, 0},
    {OP(1, 0, 1, 2, 0), 0x00000000, test_data_addr + 4, 0x0000CDEF, 0, 0},
    {OP(1, 0, 1, 2, 0), 0xFFFFFFFE, test_data_addr + 2, 0x00004567, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP(1, 0, 1, 2, 0), 0x00000001, test_data_addr    , 0x67000045, 0, 0},
    {OP(1, 0, 1, 2, 0), 0x00000000, test_data_addr + 1, 0x67000045, 0, 0},
    {OP(1, 0, 1, 2, 0), 0x00000003, test_data_addr    , 0x23000001, 0, 0},
    {OP(1, 0, 1, 2, 0), 0x00000000, test_data_addr + 3, 0x23000001, 0, 0},
#endif

    // ldsb r0 [r2 r1]
    {OP(0, 1, 1, 2, 0), 0x00000000, test_data_addr    , 0x00000067, 0, 0}, // 10
    {OP(0, 1, 1, 2, 0), 0x00000001, test_data_addr    , 0x00000045, 0, 0},
    {OP(0, 1, 1, 2, 0), 0x00000000, test_data_addr + 1, 0x00000045, 0, 0},
    {OP(0, 1, 1, 2, 0), 0x00000002, test_data_addr    , 0x00000023, 0, 0},
    {OP(0, 1, 1, 2, 0), 0x00000000, test_data_addr + 2, 0x00000023, 0, 0},
    {OP(0, 1, 1, 2, 0), 0x00000003, test_data_addr    , 0x00000001, 0, 0},
    {OP(0, 1, 1, 2, 0), 0x00000000, test_data_addr + 3, 0x00000001, 0, 0},
    {OP(0, 1, 1, 2, 0), 0x00000004, test_data_addr    , 0xFFFFFFEF, 0, 0},
    {OP(0, 1, 1, 2, 0), 0x00000000, test_data_addr + 4, 0xFFFFFFEF, 0, 0},
    {OP(0, 1, 1, 2, 0), 0xFFFFFFFF, test_data_addr + 1, 0x00000067, 0, 0},
    // a few more that get extended
    {OP(0, 1, 1, 2, 0), 0x00000006, test_data_addr    , 0xFFFFFFAB, 0, 0},
    {OP(0, 1, 1, 2, 0), 0x00000008, test_data_addr    , 0xFFFFFF98, 0, 0},

    // ldsh r0 [r2 r1]
    {OP(1, 1, 1, 2, 0), 0x00000000, test_data_addr    , 0x00004567, 0, 0}, // 22
    {OP(1, 1, 1, 2, 0), 0x00000002, test_data_addr    , 0x00000123, 0, 0},
    {OP(1, 1, 1, 2, 0), 0x00000000, test_data_addr + 2, 0x00000123, 0, 0},
    {OP(1, 1, 1, 2, 0), 0x00000004, test_data_addr    , 0xFFFFCDEF, 0, 0},
    {OP(1, 1, 1, 2, 0), 0x00000000, test_data_addr + 4, 0xFFFFCDEF, 0, 0},
    {OP(1, 1, 1, 2, 0), 0xFFFFFFFE, test_data_addr + 2, 0x00004567, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP(1, 1, 1, 2, 0), 0x00000001, test_data_addr    , 0x00000045, 0, 0},
    {OP(1, 1, 1, 2, 0), 0x00000000, test_data_addr + 1, 0x00000045, 0, 0},
    {OP(1, 1, 1, 2, 0), 0x00000003, test_data_addr    , 0x00000001, 0, 0},
    {OP(1, 1, 1, 2, 0), 0x00000000, test_data_addr + 3, 0x00000001, 0, 0},
    // extended... as bytes
    {OP(1, 1, 1, 2, 0), 0x00000000, test_data_addr + 5, 0xFFFFFFCD, 0, 0},
    {OP(1, 1, 1, 2, 0), 0x00000000, test_data_addr + 7, 0xFFFFFF89, 0, 0},
#endif
};

#undef OP

static const int num_load_sign_ex_tests = sizeof(load_sign_ex_tests) / sizeof(load_sign_ex_tests[0]);

// store half, register offset
#define OP(ro, rb, rd) (0x5200 | ro << 6 | rb << 3 | rd)

static const struct TestInfo store_half_reg_tests[] = {
    // strh r0 [r2 r1]
    {OP(1, 2, 0), 0x00000000, test_data_addr    , 0x0123DA7A, 0, 0}, // 0
    {OP(1, 2, 0), 0x00000002, test_data_addr    , 0xDA7A4567, 0, 0},
    {OP(1, 2, 0), 0x00000000, test_data_addr + 2, 0xDA7A4567, 0, 0},
    {OP(1, 2, 0), 0x00000004, test_data_addr    , 0x89ABDA7A, 0, 0},
    {OP(1, 2, 0), 0x00000000, test_data_addr + 4, 0x89ABDA7A, 0, 0},
    {OP(1, 2, 0), 0xFFFFFFFE, test_data_addr + 2, 0x0123DA7A, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP(1, 2, 0), 0x00000001, test_data_addr    , 0x0123DA7A, 0, 0},
    {OP(1, 2, 0), 0x00000000, test_data_addr + 1, 0x0123DA7A, 0, 0},
    {OP(1, 2, 0), 0x00000003, test_data_addr    , 0xDA7A4567, 0, 0},
    {OP(1, 2, 0), 0x00000000, test_data_addr + 3, 0xDA7A4567, 0, 0},
#endif
};

#undef OP

static const int num_store_half_reg_tests = sizeof(store_half_reg_tests) / sizeof(store_half_reg_tests[0]);

// load, immediate offset
#define OP_W(off, rb, rd) (0x6800 | off << 6 | rb << 3 | rd)
#define OP_B(off, rb, rd) (0x7800 | off << 6 | rb << 3 | rd)
#define OP_H(off, rb, rd) (0x8800 | off << 6 | rb << 3 | rd)

static const struct TestInfo load_imm_off_tests[] = {
    // ldr r0 [r2 #imm]
    {OP_W(0, 2, 0), NO_SRC1, test_data_addr    , 0x01234567, 0, 0}, // 0
    {OP_W(1, 2, 0), NO_SRC1, test_data_addr    , 0x89ABCDEF, 0, 0},
    {OP_W(0, 2, 0), NO_SRC1, test_data_addr + 4, 0x89ABCDEF, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP_W(0, 2, 0), NO_SRC1, test_data_addr + 1, 0x67012345, 0, 0},
    {OP_W(0, 2, 0), NO_SRC1, test_data_addr + 2, 0x45670123, 0, 0},
    {OP_W(0, 2, 0), NO_SRC1, test_data_addr + 3, 0x23456701, 0, 0},
#endif

    // ldrb r0 [r2 #imm]
    {OP_B(0, 2, 0), NO_SRC1, test_data_addr    , 0x00000067, 0, 0}, // 6
    {OP_B(1, 2, 0), NO_SRC1, test_data_addr    , 0x00000045, 0, 0},
    {OP_B(0, 2, 0), NO_SRC1, test_data_addr + 1, 0x00000045, 0, 0},
    {OP_B(2, 2, 0), NO_SRC1, test_data_addr    , 0x00000023, 0, 0},
    {OP_B(0, 2, 0), NO_SRC1, test_data_addr + 2, 0x00000023, 0, 0},
    {OP_B(3, 2, 0), NO_SRC1, test_data_addr    , 0x00000001, 0, 0},
    {OP_B(0, 2, 0), NO_SRC1, test_data_addr + 3, 0x00000001, 0, 0},
    {OP_B(4, 2, 0), NO_SRC1, test_data_addr    , 0x000000EF, 0, 0},
    {OP_B(0, 2, 0), NO_SRC1, test_data_addr + 4, 0x000000EF, 0, 0},

    // ldrh r0 [r2 #imm]
    {OP_H(0, 2, 0), NO_SRC1, test_data_addr    , 0x00004567, 0, 0}, // 15
    {OP_H(1, 2, 0), NO_SRC1, test_data_addr    , 0x00000123, 0, 0},
    {OP_H(0, 2, 0), NO_SRC1, test_data_addr + 2, 0x00000123, 0, 0},
    {OP_H(2, 2, 0), NO_SRC1, test_data_addr    , 0x0000CDEF, 0, 0},
    {OP_H(0, 2, 0), NO_SRC1, test_data_addr + 4, 0x0000CDEF, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP_H(0, 2, 0), NO_SRC1, test_data_addr + 1, 0x67000045, 0, 0},
    {OP_H(0, 2, 0), NO_SRC1, test_data_addr + 3, 0x23000001, 0, 0},
#endif
};

#undef OP_W
#undef OP_B
#undef OP_H

static const int num_load_imm_off_tests = sizeof(load_imm_off_tests) / sizeof(load_imm_off_tests[0]);

// store, immediate offset
#define OP_W(off, rb, rd) (0x6000 | off << 6 | rb << 3 | rd)
#define OP_B(off, rb, rd) (0x7000 | off << 6 | rb << 3 | rd)
#define OP_H(off, rb, rd) (0x8000 | off << 6 | rb << 3 | rd)

static const struct TestInfo store_imm_off_tests[] = {
    // doesn't need r1, but test code uses it to work out the addr
    // str r0 [r2 #imm]
    {OP_W(0, 2, 0), 0, test_data_addr    , 0x7E57DA7A, 0, 0}, // 0
    {OP_W(1, 2, 0), 4, test_data_addr    , 0x7E57DA7A, 0, 0},
    {OP_W(0, 2, 0), 0, test_data_addr + 4, 0x7E57DA7A, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP_W(0, 2, 0), 0, test_data_addr + 1, 0x7E57DA7A, 0, 0},
    {OP_W(0, 2, 0), 0, test_data_addr + 2, 0x7E57DA7A, 0, 0},
    {OP_W(0, 2, 0), 0, test_data_addr + 3, 0x7E57DA7A, 0, 0},
#endif

    // strb r0 [r2 #imm]
    {OP_B(0, 2, 0), 0, test_data_addr    , 0x0123457A, 0, 0}, // 6
    {OP_B(1, 2, 0), 1, test_data_addr    , 0x01237A67, 0, 0},
    {OP_B(0, 2, 0), 0, test_data_addr + 1, 0x01237A67, 0, 0},
    {OP_B(2, 2, 0), 2, test_data_addr    , 0x017A4567, 0, 0},
    {OP_B(0, 2, 0), 0, test_data_addr + 2, 0x017A4567, 0, 0},
    {OP_B(3, 2, 0), 3, test_data_addr    , 0x7A234567, 0, 0},
    {OP_B(0, 2, 0), 0, test_data_addr + 3, 0x7A234567, 0, 0},
    {OP_B(4, 2, 0), 4, test_data_addr    , 0x89ABCD7A, 0, 0},
    {OP_B(0, 2, 0), 0, test_data_addr + 4, 0x89ABCD7A, 0, 0},

    // strh r0 [r2 #imm]
    {OP_H(0, 2, 0), 0, test_data_addr    , 0x0123DA7A, 0, 0}, // 15
    {OP_H(1, 2, 0), 2, test_data_addr    , 0xDA7A4567, 0, 0},
    {OP_H(0, 2, 0), 0, test_data_addr + 2, 0xDA7A4567, 0, 0},
    {OP_H(2, 2, 0), 4, test_data_addr    , 0x89ABDA7A, 0, 0},
    {OP_H(0, 2, 0), 0, test_data_addr + 4, 0x89ABDA7A, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP_H(0, 2, 0), 0, test_data_addr + 1, 0x0123DA7A, 0, 0},
    {OP_H(0, 2, 0), 0, test_data_addr + 3, 0xDA7A4567, 0, 0},
#endif
};

#undef OP_W
#undef OP_B
#undef OP_H

static const int num_store_imm_off_tests = sizeof(store_imm_off_tests) / sizeof(store_imm_off_tests[0]);

// load, sp-relative
// test helper moves r2 to sp
#define OP(off, rd) (0x9800 | rd << 8 | off)

static const struct TestInfo load_sp_rel_tests[] = {
    // ldr r0 [r2 #imm]
    {OP(0, 0), NO_SRC1, test_data_addr    , 0x01234567, 0, 0}, // 0
    {OP(1, 0), NO_SRC1, test_data_addr    , 0x89ABCDEF, 0, 0},
    {OP(0, 0), NO_SRC1, test_data_addr + 4, 0x89ABCDEF, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP(0, 0), NO_SRC1, test_data_addr + 1, 0x67012345, 0, 0},
    {OP(0, 0), NO_SRC1, test_data_addr + 2, 0x45670123, 0, 0},
    {OP(0, 0), NO_SRC1, test_data_addr + 3, 0x23456701, 0, 0},
#endif
};

#undef OP

static const int num_load_sp_rel_tests = sizeof(load_sp_rel_tests) / sizeof(load_sp_rel_tests[0]);

// store, sp-relative
// test helper moves r2 to sp
#define OP(off, rd) (0x9000  | rd << 8 | off)

static const struct TestInfo store_sp_rel_tests[] = {
    // doesn't need r1, but test code uses it to work out the addr
    // str r0 [r2 #imm]
    {OP(0, 0), 0, test_data_addr    , 0x7E57DA7A, 0, 0}, // 0
    {OP(1, 0), 4, test_data_addr    , 0x7E57DA7A, 0, 0},
    {OP(0, 0), 0, test_data_addr + 4, 0x7E57DA7A, 0, 0},

#if __ARM_ARCH == 4
    // misaligned
    {OP(0, 0), 0, test_data_addr + 1, 0x7E57DA7A, 0, 0},
    {OP(0, 0), 0, test_data_addr + 2, 0x7E57DA7A, 0, 0},
    {OP(0, 0), 0, test_data_addr + 3, 0x7E57DA7A, 0, 0},
#endif
};

#undef OP

static const int num_store_sp_rel_tests = sizeof(store_sp_rel_tests) / sizeof(store_sp_rel_tests[0]);

static bool run_pc_rel_load_tests(GroupCallback group_cb, FailCallback fail_cb, const char *label) {

    bool res = true;

    group_cb(label);
    int i = 0;

    #define OP(rd, word) (0x4800 | rd << 8 | word)

    // + 0
    uint16_t *ptr = code_buf;

    *ptr++ = OP(0, 0); // r0 0

    *ptr++ = 0x4770; // BX LR
    *ptr++ = 0x0123;
    *ptr++ = 0x4567;
    *ptr++ = 0x89AB;
    *ptr++ = 0xCDEF;
    *ptr++ = 0xFEDC;
    *ptr++ = 0xBA98;
    *ptr++ = 0x7654;
    *ptr++ = 0x3210;
    uint16_t *end_ptr = ptr;

    TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);
    invalidate_icache();

    uint32_t out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);

    uint32_t expected = 0x45670123;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // now with an offset
    i++;
    code_buf[0] = OP(0, 1); // r0 4
    invalidate_icache();

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = 0xCDEF89AB;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // unalign it
    i++;
    for(uint16_t *p = end_ptr; p != code_buf + 1; p--)
        *p = *(p - 1);

    code_buf[0] = 0;
    code_buf[1] = OP(0, 0); // r0 0

    invalidate_icache();

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = 0x01234770;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // ... and with an offset again
    i++;
    code_buf[1] = OP(0, 1); // r0 4

    invalidate_icache();

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = 0x89AB4567;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    #undef OP

    return res;
}

static bool run_load_store_tests(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo *tests, int num_tests, const char *label, bool is_store) {

    bool res = true;

    group_cb(label);

    uint32_t psr_save = get_cpsr_arm() & ~PSR_MASK;

    int set_cpsr_off = (uintptr_t)set_cpsr - ((uintptr_t)code_buf + 6);

    for(int i = 0; i < num_tests; i++) {
        const struct TestInfo *test = &tests[i];

        // init data
        for(int i = 0; i < 4; i++)
            test_data[i] = test_data_init[i];
    
        // value test
        uint16_t *ptr = code_buf;

        *ptr++ = test->opcode;
        *ptr++ = 0x4770; // BX LR

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

        invalidate_icache();

        uint32_t out = func(is_store ? 0x7E57DA7A : 0xBAD, test->m_in, test->n_in, 0x3BAD);

        if(is_store) {
            // read back nearest word
            uint32_t addr = (test->m_in + test->n_in) & ~3;
            out = *(uint32_t *)addr; 
        }

        if(out != test->d_out) {
            res = false;
            fail_cb(i, out, test->d_out);
        }
    }

    return res;
}

static bool run_sp_rel_load_store_tests(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo *tests, int num_tests, const char *label, bool is_store) {

    bool res = true;

    group_cb(label);

    uint32_t psr_save = get_cpsr_arm() & ~PSR_MASK;

    int set_cpsr_off = (uintptr_t)set_cpsr - ((uintptr_t)code_buf + 6);

    for(int i = 0; i < num_tests; i++) {
        const struct TestInfo *test = &tests[i];

        // init data
        for(int i = 0; i < 4; i++)
            test_data[i] = test_data_init[i];
    
        // value test
        uint16_t *ptr = code_buf;

        *ptr++ = 0x466B; // mov r3 sp
        *ptr++ = 0x4695; // mov sp r2

        *ptr++ = test->opcode;
        *ptr++ = 0x469D; // mov sp r3
        *ptr++ = 0x4770; // BX LR

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

        invalidate_icache();

        uint32_t out = func(is_store ? 0x7E57DA7A : 0xBAD, test->m_in, test->n_in, 0x3BAD);

        if(is_store) {
            // read back nearest word
            uint32_t addr = (test->m_in + test->n_in) & ~3;
            out = *(uint32_t *)addr; 
        }

        if(out != test->d_out) {
            res = false;
            fail_cb(i, out, test->d_out);
        }
    }

    return res;
}

bool run_single_load_store_tests(GroupCallback group_cb, FailCallback fail_cb) {
    bool ret = true;

    ret = run_pc_rel_load_tests(group_cb, fail_cb, "pcrell") && ret;
    ret = run_load_store_tests(group_cb, fail_cb, load_reg_tests, num_load_reg_tests, "ldr.reg", false) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, store_reg_tests, num_store_reg_tests, "str.reg", true) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, load_sign_ex_tests, num_load_sign_ex_tests, "ldr.sx", false) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, store_half_reg_tests, num_store_half_reg_tests, "strh.reg", true) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, load_imm_off_tests, num_load_imm_off_tests, "ldr.imm", false) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, store_imm_off_tests, num_store_imm_off_tests, "str.imm", true) && ret;
    ret = run_sp_rel_load_store_tests(group_cb, fail_cb, load_sp_rel_tests, num_load_sp_rel_tests, "ldr.sp", false) && ret;
    ret = run_sp_rel_load_store_tests(group_cb, fail_cb, store_sp_rel_tests, num_store_sp_rel_tests, "str.sp", true) && ret;

    return ret;
}

bool run_push_pop_tests(GroupCallback group_cb, FailCallback fail_cb, const char *label) {
    static const uint32_t values[] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210};
    bool res = true;

    group_cb(label);
    int i = 0;

    #define OP(load, pclr, regs) (0xB400 | load << 11 | pclr << 8 | regs)

    TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

    // push r0-3, pop single (0-3)
    for(int j = 0; j < 4; j++, i++) {
        static const uint32_t expected[] = {0x01234567, 0x76543210, 0xFEDCBA98, 0x89ABCDEF};

        uint16_t *ptr = code_buf;

        *ptr++ = OP(0, 0, 0xF); // push r0-3
        
        // rotate through regs
        // pop r0; pop r1; pop r2; pop r3
        // pop r1; pop r2; pop r3; pop r0
        // ...
        for(int r = 0; r < 4; r++) {
            int index = (r + j) % 4;
            *ptr++ = OP(1, 0, 1 << index); // pop rN
        }

        *ptr++ = 0x4770; // BX LR

        invalidate_icache();

        uint32_t out = func(values[0], values[1], values[2], values[3]);

        if(out != expected[j]) {
            res = false;
            fail_cb(i, out, expected[j]);
        }
    }

    // push single, pop r0-3 (4-7)
    for(int j = 0; j < 4; j++, i++) {
        static const uint32_t expected[] = {0x76543210, 0x01234567, 0x89ABCDEF, 0xFEDCBA98};

        uint16_t *ptr = code_buf;

        // rotate through regs
        // push r0; push r1; push r2; push r3
        // push r1; push r2; push r3; push r0
        // ...
        for(int r = 0; r < 4; r++) {
            int index = (r + j) % 4;
            *ptr++ = OP(0, 0, 1 << index); // push rN
        }

        *ptr++ = OP(1, 0, 0xF); // pop r0-3

        *ptr++ = 0x4770; // BX LR

        invalidate_icache();

        uint32_t out = func(values[0], values[1], values[2], values[3]);

        if(out != expected[j]) {
            res = false;
            fail_cb(i, out, expected[j]);
        }
    }

    // push r0, lr pop single (8-9)
    for(int j = 0; j < 2; j++, i++) {
        static const uint32_t expected[] = {0x01234567, 0x89ABCDEF};

        uint16_t *ptr = code_buf;

        *ptr++ = 0x4672; // mov r2 lr
        *ptr++ = 0x468E; // mov lr r1

        *ptr++ = OP(0, 1, 0x1); // push r0, lr

        *ptr++ = 0x4696; // mov lr r2
        
        // rotate through regs
        // pop r0; pop r1;
        // pop r1; pop r0;
        // ...
        for(int r = 0; r < 2; r++) {
            int index = (r + j) % 2;
            *ptr++ = OP(1, 0, 1 << index); // pop rN
        }

        *ptr++ = 0x4770; // BX LR

        invalidate_icache();

        uint32_t out = func(values[0], values[1], values[2], values[3]);

        if(out != expected[j]) {
            res = false;
            fail_cb(i, out, expected[j]);
        }
    }

    // unaligned sp (10)
    // has no effect
    uint16_t *ptr = code_buf;

    *ptr++ = 0x466B; // mov r3 sp
    *ptr++ = 0x1C5A; // add r2 r3 1
    *ptr++ = 0x4695; // mov sp r2

    *ptr++ = OP(0, 0, 1); // push r0
    *ptr++ = OP(1, 0, 1); // pop r0

    *ptr++ = 0x469D; // mov sp r3
    *ptr++ = 0x4770; // BX LR

    invalidate_icache();

    uint32_t out = func(values[0], values[1], values[2], values[3]);

    uint32_t expected = 0x01234567;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    #undef OP

    return res;
}

bool run_ldm_stm_tests(GroupCallback group_cb, FailCallback fail_cb, const char *label) {
    static const uint32_t values[] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210};
    bool res = true;

    group_cb(label);
    int i = 0;

    #define OP(load, rb, regs) (0xC000 | load << 11 | rb << 8 | regs)

    TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

    // stm r0-3, ldm single (0-3)
    // stm is not the inverse of ldm
    for(int j = 0; j < 4; j++, i++) {
        static const uint32_t expected[] = {0x01234567, 0x76543210, 0xFEDCBA98, 0x89ABCDEF};

        uint16_t *ptr = code_buf;
        *ptr++ = 0xB480; // push r7

        *ptr++ = 0x466F; // mov r7 sp
        *ptr++ = 0x3F10; // sub r7 16

        *ptr++ = OP(0, 7, 0xF); // stmia r7 r0-3

        *ptr++ = 0x3F10; // sub r7 16

        // rotate through regs
        for(int r = 0; r < 4; r++) {
            int index = (r + j) % 4;
            *ptr++ = OP(1, 7, 1 << index); // ldmia r7 rN
        }

        *ptr++ = 0xBC80; // pop r7
        *ptr++ = 0x4770; // BX LR

        invalidate_icache();

        uint32_t out = func(values[0], values[1], values[2], values[3]);

        if(out != expected[j]) {
            res = false;
            fail_cb(i, out, expected[j]);
        }
    }

    // stm single, ldm r0-3 (4-7)
    for(int j = 0; j < 4; j++, i++) {
        static const uint32_t expected[] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210};

        uint16_t *ptr = code_buf;
        *ptr++ = 0xB480; // push r7
        *ptr++ = 0x466F; // mov r7 sp
        *ptr++ = 0x3F10; // sub r7 16

        // rotate through regs
        for(int r = 0; r < 4; r++) {
            int index = (r + j) % 4;
            *ptr++ = OP(0, 7, 1 << index); // stmia r7 rN
        }

        *ptr++ = 0x3F10; // sub r7 16

        *ptr++ = OP(1, 7, 0xF); // ldmia r7 r0-3

        *ptr++ = 0xBC80; // pop r7
        *ptr++ = 0x4770; // BX LR

        invalidate_icache();

        uint32_t out = func(values[0], values[1], values[2], values[3]);

        if(out != expected[j]) {
            res = false;
            fail_cb(i, out, expected[j]);
        }
    }

    // stm base in list (8-11)
    for(int j = 0; j < 4; j++, i++) {
        uint32_t spIn;
        asm volatile(
            "mov %0, sp"
            : "=r"(spIn)
        );

        uint16_t *ptr = code_buf;
        *ptr++ = 0xB480; // push r7

        *ptr++ = 0x466F; // mov r7 sp
        *ptr++ = 0x3F10; // sub r7 16

        *ptr++ = 0x0038 | j; // mov rN r7

        *ptr++ = OP(0, j, 0xF); // stmia rN r0-3

        *ptr++ = OP(1, 7, 0xF); // ldmia r7 r0-3

        *ptr++ = j << 3; // mov r0 rN

        *ptr++ = 0xBC80; // pop r7
        *ptr++ = 0x4770; // BX LR

        invalidate_icache();

        uint32_t out = func(values[0], values[1], values[2], values[3]);

#if __ARM_ARCH == 6
        uint32_t expected = spIn - 20; // always overwrites
#else
        uint32_t expected = j == 0 ? spIn - 20 : spIn - 4; // overwrites if not first reg
#endif

        if(out != expected) {
            res = false;
            fail_cb(i, out, expected);
        }
    }

    // ldm base in list (12-15)
    // doesn't write back
    for(int j = 0; j < 4; j++, i++) {
        uint32_t spIn;
        asm volatile(
            "mov %0, sp"
            : "=r"(spIn)
        );

        static const uint32_t expected[] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210};

        uint16_t *ptr = code_buf;
        *ptr++ = 0xB480; // push r7

        *ptr++ = 0x466F; // mov r7 sp
        *ptr++ = 0x3F10; // sub r7 16

        *ptr++ = OP(0, 7, 0xF); // stmia r7 r0-3

        *ptr++ = 0x3F10; // sub r7 16
        *ptr++ = 0x0038 | j; // mov rN r7

        *ptr++ = OP(1, j, 0xF); // ldmia rN r0-3

        *ptr++ = j << 3; // mov r0 rN

        *ptr++ = 0xBC80; // pop r7
        *ptr++ = 0x4770; // BX LR

        invalidate_icache();

        uint32_t out = func(values[0], values[1], values[2], values[3]);
        
        if(out != expected[j]) {
            res = false;
            fail_cb(i, out, expected[j]);
        }
    }

#if __ARM_ARCH < 7
    // stm empty list (16)
    // stores pc or lr
    uint16_t *ptr = code_buf;
    *ptr++ = 0xB580; // push r7,lr

    *ptr++ = 0x468E; // mov lr r1

    *ptr++ = 0x466F; // mov r7 sp
    *ptr++ = 0x3F40; // sub r7 64

    *ptr++ = OP(0, 7, 0); // stmia r7 nothing

#if __ARM_ARCH >= 6
    *ptr++ = 0x3F04; // sub r7 4
#else
    *ptr++ = 0x3F40; // sub r7 64
#endif

    *ptr++ = OP(1, 7, 1); // ldmia r7 r0

    *ptr++ = 0xBD80; // pop r7, lr

    uint32_t out = func(values[0], values[1], values[2], values[3]);
#if __ARM_ARCH >= 6
    uint32_t expected = values[1]; // stores lr
#else
    uint32_t expected = (uintptr_t)code_buf + 14; // stores pc
#endif

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }
    i++;

    // ldm empty list (17)
    // loads pc
    uint32_t spIn;
    asm volatile(
        "mov %0, sp"
        : "=r"(spIn)
    );

    ptr = code_buf;
    *ptr++ = 0xB480; // push r7

#if __ARM_ARCH >= 6// pop interworks so need to make sure the bit is set
    *ptr++ = 0xA003; // add r0 pc 12
    *ptr++ = 0x3801; // sub r0 1 (need to -2 then +1)
#else
    *ptr++ = 0xA002; // add r0 pc 8
#endif

    *ptr++ = 0xB401; // push r0

    *ptr++ = 0x466F; // mov r7 sp
    *ptr++ = OP(1, 7, 0); // ldmia r7 nothing

    *ptr++ = 0x2700; // mov r7 0 (shouldn't get here)

    *ptr++ = 0xBC01; // pop r0

    *ptr++ = 0x0038; // mov r0 r7

    *ptr++ = 0xBC80; // pop r7
    *ptr++ = 0x4770; // BX LR

    out = func(values[0], values[1], values[2], values[3]);

#if __ARM_ARCH >= 6
    expected = spIn - 8/*2x push*/; // does not adjust base
#else
    expected = spIn - 8/*2x push*/ + 64; // adds 64 to base
#endif

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }
    i++;
#endif

    // unaligned base (18)
#if __ARM_ARCH == 4
    // has no effect
    ptr = code_buf;
    *ptr++ = 0xB480; // push r7

    *ptr++ = 0x466F; // mov r7 sp
    *ptr++ = 0x3F05; // sub r7 5

    *ptr++ = OP(0, 7, 1); // stmia r7 r0

    *ptr++ = 0x3F04; // sub r7 4

    *ptr++ = OP(1, 7, 1); // ldmia r7 r0

    *ptr++ = 0xBC80; // pop r7
    *ptr++ = 0x4770; // BX LR

    out = func(values[0], values[1], values[2], values[3]);

    expected = 0x01234567;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }
#endif

    #undef OP

    return res;
}