#pragma once

#include <stdint.h>

#define FLAG_Q (1 << 27) // v6 (or 5TE), though we only care about it on v7M
#define FLAG_V (1 << 28)
#define FLAG_C (1 << 29)
#define FLAG_Z (1 << 30)
#define FLAG_N (1 << 31)

#if __ARM_ARCH >= 7
#define PSR_MASK 0xF8000000
#else
#define PSR_MASK 0xF0000000
#endif

#define PSR_VCZN 0xF0000000

#define NO_SRC1 0x1BAD
#define NO_SRC2 0x2BAD

struct TestInfo {
    uint16_t opcode;

    uint32_t m_in, n_in;

    uint32_t d_out;

    uint32_t psr_in, psr_out;
};

// Thumb2
struct TestInfo32 {
    uint32_t opcode;

    uint32_t m_in, n_in;

    uint32_t d_out;

    uint32_t psr_in, psr_out;
};

typedef uint32_t (*TestFunc)(uint32_t, uint32_t, uint32_t, uint32_t);

extern uint16_t code_buf[32];

// helpers for accessing flags (not possible from thumb on v4)
void set_cpsr(uint32_t v);
uint32_t get_cpsr_arm();

// Thumb2 opcodes to access flags directly
#define GET_PSR_OP(d) (0xF3EF8000 | d << 8)
#define SET_PSR_OP(n) (0xF3808800 | n << 16)

void invalidate_icache();

bool run_thumb2_test_list(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo32 *tests, int num_tests, const char *label, int dest, bool flags_for_val, bool test_s_bit);