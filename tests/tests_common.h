#pragma once

#include <stdint.h>

#define FLAG_V (1 << 28)
#define FLAG_C (1 << 29)
#define FLAG_Z (1 << 30)
#define FLAG_N (1 << 31)

#define PSR_MASK 0xF0000000

#define NO_SRC1 0x1BAD
#define NO_SRC2 0x2BAD

struct TestInfo {
    uint16_t opcode;

    uint32_t m_in, n_in;

    uint32_t d_out;

    uint32_t psr_in, psr_out;
};

typedef uint32_t (*TestFunc)(uint32_t, uint32_t, uint32_t, uint32_t);

extern uint16_t code_buf[32];

void set_cpsr(uint32_t v);
uint32_t get_cpsr_arm();

void invalidate_icache();