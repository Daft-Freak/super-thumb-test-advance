#include "tests.h"

typedef uint32_t (*TestFunc)(uint32_t, uint32_t, uint32_t, uint32_t);

static _Alignas(4) uint16_t code_buf[32];

static const uint32_t test_data_init[4] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210};
static uint32_t test_data[4];
static const uint32_t test_data_addr = (uintptr_t)test_data;

#define FLAG_V (1 << 28)
#define FLAG_C (1 << 29)
#define FLAG_Z (1 << 30)
#define FLAG_N (1 << 31)

#define PSR_MASK 0xF0000000

#define NO_SRC1 0x1BAD
#define NO_SRC2 0x2BAD

// test list
struct TestInfo {
    uint16_t opcode;

    uint32_t m_in, n_in;

    uint32_t d_out;

    uint32_t psr_in, psr_out;
};

#define OP(op, imm, rm, rd) (op << 11 | imm << 6 | rm << 3 | rd)

static const struct TestInfo shift_imm_tests[] = {
    // setting flag bits to make sure they get cleared
    {OP(0,  0, 1, 0), 0x55555555, NO_SRC2, 0x55555555, FLAG_Z | FLAG_N, 0                       }, // LSL 0
    {OP(0,  0, 1, 0), 0xAAAAAAAA, NO_SRC2, 0xAAAAAAAA, PSR_MASK       , FLAG_V | FLAG_C | FLAG_N}, // LSL 0
    {OP(0,  1, 1, 0), 0x55555555, NO_SRC2, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_N                  }, // LSL 1
    {OP(0,  1, 1, 0), 0xAAAAAAAA, NO_SRC2, 0x55555554, FLAG_Z | FLAG_N, FLAG_C                  }, // LSL 1
    {OP(0, 31, 1, 0), 0x55555555, NO_SRC2, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  }, // LSL 31
    {OP(0, 31, 1, 0), 0xAAAAAAAA, NO_SRC2, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         }, // LSL 31

    {OP(1,  0, 1, 0), 0x55555555, NO_SRC2, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  }, // LSR 0 (32)
    {OP(1,  0, 1, 0), 0xAAAAAAAA, NO_SRC2, 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z}, // LSR 0 (32)
    {OP(1,  1, 1, 0), 0x55555555, NO_SRC2, 0x2AAAAAAA, FLAG_Z | FLAG_N, FLAG_C                  }, // LSR 1
    {OP(1,  1, 1, 0), 0xAAAAAAAA, NO_SRC2, 0x55555555, FLAG_C | FLAG_N, 0                       }, // LSR 1
    {OP(1, 31, 1, 0), 0x55555555, NO_SRC2, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         }, // LSR 31
    {OP(1, 31, 1, 0), 0xAAAAAAAA, NO_SRC2, 0x00000001, FLAG_Z | FLAG_N, 0                       }, // LSR 31

    {OP(2,  0, 1, 0), 0x55555555, NO_SRC2, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  }, // ASR 0 (32)
    {OP(2,  0, 1, 0), 0xAAAAAAAA, NO_SRC2, 0xFFFFFFFF, PSR_MASK       , FLAG_V | FLAG_C | FLAG_N}, // ASR 0 (32)
    {OP(2,  1, 1, 0), 0x55555555, NO_SRC2, 0x2AAAAAAA, FLAG_Z | FLAG_N, FLAG_C                  }, // ASR 1
    {OP(2,  1, 1, 0), 0xAAAAAAAA, NO_SRC2, 0xD5555555, FLAG_C | FLAG_Z, FLAG_N                  }, // ASR 1
    {OP(2, 31, 1, 0), 0x55555555, NO_SRC2, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         }, // ASR 31
    {OP(2, 31, 1, 0), 0xAAAAAAAA, NO_SRC2, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  }, // ASR 31
};

#undef OP

static const int num_shift_imm_tests = sizeof(shift_imm_tests) / sizeof(shift_imm_tests[0]);

// add/sub reg/imm3
#define OP(op, rm, rn, rd) (0x1800 | op << 9 | rm << 6 | rn << 3 | rd)

static const struct TestInfo add_sub_tests[] = {
    // ADD r0 r1 r2
    {OP(0, 2, 1, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 0
    {OP(0, 2, 1, 0), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_Z                  },
    {OP(0, 2, 1, 0), 0x00000000, 0x00000001, 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(0, 2, 1, 0), 0x00000001, 0x00000000, 0x00000001, FLAG_C | FLAG_N, 0                       },
    {OP(0, 2, 1, 0), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, 0                       },
    {OP(0, 2, 1, 0), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_Z, 0                       },
    {OP(0, 2, 1, 0), 0x7FFFFFFF, 0x00000001, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0, 2, 1, 0), 0x00000001, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0, 2, 1, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0, 2, 1, 0), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0, 2, 1, 0), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0, 2, 1, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0, 2, 1, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0, 2, 1, 0), 0x80000000, 0x00000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0, 2, 1, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0, 2, 1, 0), 0x80000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0, 2, 1, 0), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0, 2, 1, 0), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0, 2, 1, 0), 0xFFFFFFFF, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0, 2, 1, 0), 0x00000001, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0, 2, 1, 0), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0, 2, 1, 0), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0, 2, 1, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },

    // SUB r0 r1 r2
    {OP(1, 2, 1, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 23
    {OP(1, 2, 1, 0), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_C | FLAG_Z         },
    {OP(1, 2, 1, 0), 0x00000000, 0x00000001, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 2, 1, 0), 0x00000001, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 2, 1, 0), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 2, 1, 0), 0x00000000, 0x7FFFFFFF, 0x80000001, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(1, 2, 1, 0), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 2, 1, 0), 0x00000001, 0x7FFFFFFF, 0x80000002, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 2, 1, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(1, 2, 1, 0), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 2, 1, 0), 0x80000000, 0x7FFFFFFF, 0x00000001, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(1, 2, 1, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 2, 1, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 2, 1, 0), 0x80000000, 0x00000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 2, 1, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 2, 1, 0), 0x80000000, 0x80000000, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(1, 2, 1, 0), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 2, 1, 0), 0x00000000, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_C, 0                       },
    {OP(1, 2, 1, 0), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 2, 1, 0), 0x00000001, 0xFFFFFFFF, 0x00000002, FLAG_V | FLAG_N, 0                       },
    {OP(1, 2, 1, 0), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 2, 1, 0), 0x80000000, 0xFFFFFFFF, 0x80000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 2, 1, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // ADD r0 r1 #imm
    {OP(2, 0, 1, 0), 0x00000000, NO_SRC2   , 0x00000000, 0              , FLAG_Z                  }, // 46
    {OP(2, 0, 1, 0), 0x00000000, NO_SRC2   , 0x00000000, PSR_MASK       , FLAG_Z                  },
    {OP(2, 1, 1, 0), 0x00000000, NO_SRC2   , 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(2, 0, 1, 0), 0x00000001, NO_SRC2   , 0x00000001, FLAG_C | FLAG_N, 0                       },
    {OP(2, 0, 1, 0), 0x7FFFFFFF, NO_SRC2   , 0x7FFFFFFF, FLAG_C | FLAG_Z, 0                       },
    {OP(2, 3, 1, 0), 0x00000000, NO_SRC2   , 0x00000003, FLAG_V | FLAG_Z, 0                       },
    {OP(2, 1, 1, 0), 0x7FFFFFFF, NO_SRC2   , 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(2, 3, 1, 0), 0x7FFFFFFD, NO_SRC2   , 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(2, 3, 1, 0), 0xFFFFFFFC, NO_SRC2   , 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(2, 3, 1, 0), 0xFFFFFFFF, NO_SRC2   , 0x00000002, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(2, 0, 1, 0), 0x80000000, NO_SRC2   , 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(2, 0, 1, 0), 0xFFFFFFFF, NO_SRC2   , 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(2, 7, 1, 0), 0x00000000, NO_SRC2   , 0x00000007, FLAG_V | FLAG_C, 0                       },
    {OP(2, 1, 1, 0), 0xFFFFFFFF, NO_SRC2   , 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(2, 7, 1, 0), 0xFFFFFFF9, NO_SRC2   , 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // SUB r0 r1 #imm
    {OP(3, 0, 1, 0), 0x00000000, NO_SRC2   , 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 61
    {OP(3, 0, 1, 0), 0x00000000, NO_SRC2   , 0x00000000, PSR_MASK       , FLAG_C | FLAG_Z         },
    {OP(3, 1, 1, 0), 0x00000000, NO_SRC2   , 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(3, 0, 1, 0), 0x00000001, NO_SRC2   , 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(3, 0, 1, 0), 0x7FFFFFFF, NO_SRC2   , 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(3, 3, 1, 0), 0x00000000, NO_SRC2   , 0xFFFFFFFD, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(3, 1, 1, 0), 0x7FFFFFFF, NO_SRC2   , 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(3, 3, 1, 0), 0x00000001, NO_SRC2   , 0xFFFFFFFE, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(3, 3, 1, 0), 0x00000003, NO_SRC2   , 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(3, 3, 1, 0), 0x80000000, NO_SRC2   , 0x7FFFFFFD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(3, 3, 1, 0), 0xFFFFFFFF, NO_SRC2   , 0xFFFFFFFC, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(3, 0, 1, 0), 0x80000000, NO_SRC2   , 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(3, 0, 1, 0), 0xFFFFFFFF, NO_SRC2   , 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(3, 1, 1, 0), 0xFFFFFFFF, NO_SRC2   , 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(3, 7, 1, 0), 0x00000007, NO_SRC2   , 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
};

#undef OP

static const int num_add_sub_tests = sizeof(add_sub_tests) / sizeof(add_sub_tests[0]);

// mov/cmp/add/sub imm8
#define OP(op, rdn, imm) (0x2000 | op << 11 | rdn << 8 | imm)

static const struct TestInfo mov_cmp_add_sub_imm_tests[] = {
    // MOV r1 #imm
    // only sets Z/N and N=1 is impossible
    {OP(0, 1, 0x00), 0x00000000, NO_SRC2   , 0x00000000, 0              , FLAG_Z                  }, // 0
    {OP(0, 1, 0x00), 0x00000000, NO_SRC2   , 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0, 1, 0xFF), 0x00000000, NO_SRC2   , 0x000000FF, FLAG_Z | FLAG_N, 0                       },

    // CMP r1 #imm
    {OP(1, 1, 0x00), 0x00000000, NO_SRC2   , 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 3
    {OP(1, 1, 0x00), 0x00000000, NO_SRC2   , 0x00000000, PSR_MASK       , FLAG_C | FLAG_Z         },
    {OP(1, 1, 0x01), 0x00000000, NO_SRC2   , 0x00000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 1, 0x00), 0x00000001, NO_SRC2   , 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 1, 0xFF), 0x80000000, NO_SRC2   , 0x80000000, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(1, 1, 0xFF), 0xFFFFFFFF, NO_SRC2   , 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },

    // ADD r1 #imm
    {OP(2, 1, 0x00), 0x00000000, NO_SRC2   , 0x00000000, 0              , FLAG_Z                  }, // 9
    {OP(2, 1, 0x00), 0x00000000, NO_SRC2   , 0x00000000, PSR_MASK       , FLAG_Z                  },
    {OP(2, 1, 0x01), 0x00000000, NO_SRC2   , 0x00000001, FLAG_C | FLAG_Z, 0                       },
    {OP(2, 1, 0x00), 0x00000001, NO_SRC2   , 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(2, 1, 0x00), 0x7FFFFFFF, NO_SRC2   , 0x7FFFFFFF, FLAG_C | FLAG_Z, 0                       },
    {OP(2, 1, 0xFF), 0x00000000, NO_SRC2   , 0x000000FF, FLAG_V | FLAG_N, 0                       },
    {OP(2, 1, 0x01), 0x7FFFFFFF, NO_SRC2   , 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(2, 1, 0x7F), 0xFFFFFFFF, NO_SRC2   , 0x0000007E, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(2, 1, 0x00), 0x80000000, NO_SRC2   , 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(2, 1, 0x01), 0xFFFFFFFF, NO_SRC2   , 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // SUB r1 #imm
    {OP(3, 1, 0x00), 0x00000000, NO_SRC2   , 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 19
    {OP(3, 1, 0x00), 0x00000000, NO_SRC2   , 0x00000000, PSR_MASK       , FLAG_C | FLAG_Z         },
    {OP(3, 1, 0x01), 0x00000000, NO_SRC2   , 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(3, 1, 0x00), 0x00000001, NO_SRC2   , 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(3, 1, 0x00), 0x7FFFFFFF, NO_SRC2   , 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_C                  },
    {OP(3, 1, 0xFF), 0x80000000, NO_SRC2   , 0x7FFFFF01, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(3, 1, 0xFF), 0xFFFFFFFF, NO_SRC2   , 0xFFFFFF00, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(3, 1, 0xFF), 0x000000FF, NO_SRC2   , 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(3, 1, 0xFF), 0x00000000, NO_SRC2   , 0xFFFFFF01, FLAG_V | FLAG_Z, FLAG_N                  },
};

#undef OP

static const int num_mov_cmp_add_sub_imm_tests = sizeof(mov_cmp_add_sub_imm_tests) / sizeof(mov_cmp_add_sub_imm_tests[0]);

// data processing
#define OP(op, rm, rdn) (0x4000 | op << 6  | rm << 3 | rdn)

static const struct TestInfo dp_tests[] = {
    // AND r1 r2
    {OP(0x0, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 0
    {OP(0x0, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x0, 2, 1), 0x00000000, 0x7FFFFFFF, 0x00000000, 0              , FLAG_Z                  },
    {OP(0x0, 2, 1), 0x7FFFFFFF, 0x00000000, 0x00000000, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0x0, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x0, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x0, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x0, 2, 1), 0x00000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 2, 1), 0x80000000, 0x00000000, 0x00000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 2, 1), 0x80000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 2, 1), 0xFFFFFFFF, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 2, 1), 0xAAAAAAAA, 0x55555555, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 2, 1), 0x55555555, 0xAAAAAAAA, 0x00000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 2, 1), 0xAAAAAAAA, 0xFFFFFFFF, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 2, 1), 0xFFFFFFFF, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 2, 1), 0x55555555, 0xFFFFFFFF, 0x55555555, 0              , 0                       },
    {OP(0x0, 2, 1), 0xFFFFFFFF, 0x55555555, 0x55555555, FLAG_C         , FLAG_C                  },

    // EOR r1 r2
    {OP(0x1, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 18
    {OP(0x1, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x1, 2, 1), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x1, 2, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x1, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x1, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x1, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x1, 2, 1), 0x00000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x1, 2, 1), 0x80000000, 0x00000000, 0x80000000, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x1, 2, 1), 0x80000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x1, 2, 1), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0x1, 2, 1), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_C                  },
    {OP(0x1, 2, 1), 0xAAAAAAAA, 0x55555555, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x1, 2, 1), 0x55555555, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x1, 2, 1), 0xAAAAAAAA, 0xFFFFFFFF, 0x55555555, 0              , 0                       },
    {OP(0x1, 2, 1), 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, FLAG_C         , FLAG_C                  },
    {OP(0x1, 2, 1), 0x55555555, 0xFFFFFFFF, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x1, 2, 1), 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },

    // LSL r1 r2
    {OP(0x2, 2, 1), 0x55555555, 0x00000000, 0x55555555, FLAG_Z | FLAG_N, 0                       }, // 36
    {OP(0x2, 2, 1), 0xAAAAAAAA, 0x00000000, 0xAAAAAAAA, PSR_MASK       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x2, 2, 1), 0x55555555, 0x00000001, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x2, 2, 1), 0xAAAAAAAA, 0x00000001, 0x55555554, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x2, 2, 1), 0x55555555, 0x0000001F, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x2, 2, 1), 0xAAAAAAAA, 0x0000001F, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x2, 2, 1), 0x55555555, 0x00000020, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x2, 2, 1), 0xAAAAAAAA, 0x00000020, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(0x2, 2, 1), 0x55555555, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  }, // too big
    {OP(0x2, 2, 1), 0xAAAAAAAA, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(0x2, 2, 1), 0x55555555, 0x00000100, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  }, // only low byte is used
    {OP(0x2, 2, 1), 0xAAAAAAAA, 0x00000100, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x2, 2, 1), 0x55555555, 0x80000000, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x2, 2, 1), 0xAAAAAAAA, 0x80000000, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },

    // LSR r1 r2
    {OP(0x3, 2, 1), 0x55555555, 0x00000000, 0x55555555, FLAG_Z | FLAG_N, 0                       }, // 50
    {OP(0x3, 2, 1), 0xAAAAAAAA, 0x00000000, 0xAAAAAAAA, PSR_MASK       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x3, 2, 1), 0x55555555, 0x00000001, 0x2AAAAAAA, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x3, 2, 1), 0xAAAAAAAA, 0x00000001, 0x55555555, FLAG_C | FLAG_N, 0                       },
    {OP(0x3, 2, 1), 0x55555555, 0x0000001F, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x3, 2, 1), 0xAAAAAAAA, 0x0000001F, 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(0x3, 2, 1), 0x55555555, 0x00000020, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(0x3, 2, 1), 0xAAAAAAAA, 0x00000020, 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x3, 2, 1), 0x55555555, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(0x3, 2, 1), 0xAAAAAAAA, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(0x3, 2, 1), 0x55555555, 0x00000100, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x3, 2, 1), 0xAAAAAAAA, 0x00000100, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x3, 2, 1), 0x55555555, 0x80000000, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x3, 2, 1), 0xAAAAAAAA, 0x80000000, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },

    // ASR r1 r2
    {OP(0x4, 2, 1), 0x55555555, 0x00000000, 0x55555555, FLAG_Z | FLAG_N, 0                       }, // 64
    {OP(0x4, 2, 1), 0xAAAAAAAA, 0x00000000, 0xAAAAAAAA, PSR_MASK       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x4, 2, 1), 0x55555555, 0x00000001, 0x2AAAAAAA, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x4, 2, 1), 0xAAAAAAAA, 0x00000001, 0xD5555555, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x4, 2, 1), 0x55555555, 0x0000001F, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x4, 2, 1), 0xAAAAAAAA, 0x0000001F, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x4, 2, 1), 0x55555555, 0x00000020, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(0x4, 2, 1), 0xAAAAAAAA, 0x00000020, 0xFFFFFFFF, PSR_MASK       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x4, 2, 1), 0x55555555, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(0x4, 2, 1), 0xAAAAAAAA, 0x00000080, 0xFFFFFFFF, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x4, 2, 1), 0x55555555, 0x00000100, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x4, 2, 1), 0xAAAAAAAA, 0x00000100, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 2, 1), 0x55555555, 0x80000000, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x4, 2, 1), 0xAAAAAAAA, 0x80000000, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },

    // ADC r1 r2
    {OP(0x5, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 78
    {OP(0x5, 2, 1), 0x00000000, 0x00000000, 0x00000001, PSR_MASK       , 0                       },
    {OP(0x5, 2, 1), 0x00000000, 0x00000001, 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(0x5, 2, 1), 0x00000001, 0x00000000, 0x00000001, FLAG_N         , 0                       },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0x5, 2, 1), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_Z, 0                       },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0x00000001, 0x80000000, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x5, 2, 1), 0x00000001, 0x7FFFFFFF, 0x80000000, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x5, 2, 1), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0x5, 2, 1), 0x80000000, 0x00000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x5, 2, 1), 0x00000000, 0x80000000, 0x80000000, FLAG_V         , FLAG_N                  },
    {OP(0x5, 2, 1), 0x80000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x5, 2, 1), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V         , FLAG_N                  },
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x5, 2, 1), 0x00000001, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x5, 2, 1), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },

    // now with C set
    {OP(0x5, 2, 1), 0x00000000, 0x00000001, 0x00000002, FLAG_C | FLAG_Z | FLAG_N, 0                       }, // 101
    {OP(0x5, 2, 1), 0x00000001, 0x00000000, 0x00000002, FLAG_C | FLAG_N         , 0                       },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0x00000000, 0x80000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x5, 2, 1), 0x00000000, 0x7FFFFFFF, 0x80000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0x00000001, 0x80000001, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x5, 2, 1), 0x00000001, 0x7FFFFFFF, 0x80000001, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0x80000000, 0x00000000, FLAG_C | FLAG_Z         , FLAG_C | FLAG_Z         },
    {OP(0x5, 2, 1), 0x80000000, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_Z         },
    {OP(0x5, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x5, 2, 1), 0x80000000, 0x00000000, 0x80000001, FLAG_C | FLAG_Z         , FLAG_N                  },
    {OP(0x5, 2, 1), 0x00000000, 0x80000000, 0x80000001, FLAG_V | FLAG_C         , FLAG_N                  },
    {OP(0x5, 2, 1), 0x80000000, 0x80000000, 0x00000001, FLAG_C | FLAG_N         , FLAG_V | FLAG_C         },
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0x00000000, 0x00000000, FLAG_C | FLAG_Z         , FLAG_C | FLAG_Z         },
    {OP(0x5, 2, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0x00000001, 0x00000001, FLAG_V | FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x5, 2, 1), 0x00000001, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0x80000000, 0x80000000, FLAG_C | FLAG_Z | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x5, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x5, 2, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },

    // SBC r0 r1 r2
    // start with C set
    {OP(0x6, 2, 1), 0x00000000, 0x00000000, 0xFFFFFFFF, 0                       , FLAG_N                  }, // 122
    {OP(0x6, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK                , FLAG_C | FLAG_Z         },
    {OP(0x6, 2, 1), 0x00000000, 0x00000001, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_N                  },
    {OP(0x6, 2, 1), 0x00000001, 0x00000000, 0x00000001, FLAG_C | FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_C | FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x6, 2, 1), 0x00000000, 0x7FFFFFFF, 0x80000001, FLAG_V | FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFE, FLAG_C | FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x6, 2, 1), 0x00000001, 0x7FFFFFFF, 0x80000002, FLAG_C | FLAG_Z         , FLAG_N                  },
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x6, 2, 1), 0x80000000, 0x7FFFFFFF, 0x00000001, FLAG_C | FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x6, 2, 1), 0x80000000, 0x00000000, 0x80000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x6, 2, 1), 0x00000000, 0x80000000, 0x80000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x6, 2, 1), 0x80000000, 0x80000000, 0x00000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x6, 2, 1), 0x00000000, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_C | FLAG_C, 0                       },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFE, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x6, 2, 1), 0x00000001, 0xFFFFFFFF, 0x00000002, FLAG_V | FLAG_C | FLAG_N, 0                       },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x6, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000001, FLAG_C | FLAG_Z         , FLAG_N                  },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },

    // now with C clear
    {OP(0x6, 2, 1), 0x00000000, 0x00000001, 0xFFFFFFFE,          FLAG_Z, FLAG_N                  }, // 145
    {OP(0x6, 2, 1), 0x00000001, 0x00000000, 0x00000000, FLAG_Z | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x6, 2, 1), 0x00000000, 0x7FFFFFFF, 0x80000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x6, 2, 1), 0x00000001, 0x7FFFFFFF, 0x80000001, FLAG_Z         , FLAG_N                  },
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_N                  },
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFE, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0x6, 2, 1), 0x80000000, 0x7FFFFFFF, 0x00000000, FLAG_Z | FLAG_N, FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x6, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_Z, FLAG_V | FLAG_C         },
    {OP(0x6, 2, 1), 0x80000000, 0x00000000, 0x7FFFFFFF, FLAG_V | FLAG_Z, FLAG_V | FLAG_C         },
    {OP(0x6, 2, 1), 0x00000000, 0x80000000, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0x6, 2, 1), 0x80000000, 0x80000000, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_N                  },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x6, 2, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_V         , FLAG_Z                  },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x6, 2, 1), 0x00000001, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_N, 0                       },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x6, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x6, 2, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_N                  },

    // ROR r1 r2
    {OP(0x7, 2, 1), 0x55555555, 0x00000000, 0x55555555, FLAG_Z | FLAG_N, 0                       }, // 166
    {OP(0x7, 2, 1), 0xAAAAAAAA, 0x00000000, 0xAAAAAAAA, PSR_MASK       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x7, 2, 1), 0x55555555, 0x00000001, 0xAAAAAAAA, FLAG_Z | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x7, 2, 1), 0xAAAAAAAA, 0x00000001, 0x55555555, FLAG_C | FLAG_N, 0                       },
    {OP(0x7, 2, 1), 0x55555555, 0x0000001F, 0xAAAAAAAA, FLAG_Z | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x7, 2, 1), 0xAAAAAAAA, 0x0000001F, 0x55555555, FLAG_C | FLAG_N, 0                       },
    {OP(0x7, 2, 1), 0x55555555, 0x00000020, 0x55555555, FLAG_Z | FLAG_N, 0                       },
    {OP(0x7, 2, 1), 0xAAAAAAAA, 0x00000020, 0xAAAAAAAA, PSR_MASK       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x7, 2, 1), 0x55555555, 0x00000080, 0x55555555, FLAG_Z | FLAG_N, 0                       },
    {OP(0x7, 2, 1), 0xAAAAAAAA, 0x00000080, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_N | FLAG_C         },
    {OP(0x7, 2, 1), 0x55555555, 0x00000100, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x7, 2, 1), 0xAAAAAAAA, 0x00000100, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x7, 2, 1), 0x55555555, 0x80000000, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x7, 2, 1), 0xAAAAAAAA, 0x80000000, 0xAAAAAAAA,          FLAG_Z, FLAG_N                  },
    {OP(0x7, 2, 1), 0x12345678, 0x00000001, 0x091A2B3C, FLAG_Z | FLAG_N, 0                       },
    {OP(0x7, 2, 1), 0x12345678, 0x00000004, 0x81234567, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x7, 2, 1), 0x12345678, 0x00000010, 0x56781234, FLAG_Z | FLAG_N, 0                       },
    {OP(0x7, 2, 1), 0x12345678, 0x00000020, 0x12345678, FLAG_C | FLAG_Z, 0                       },

    // TST r1 r2
    {OP(0x8, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 184
    {OP(0x8, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x8, 2, 1), 0x00000000, 0x7FFFFFFF, 0x00000000, 0              , FLAG_Z                  },
    {OP(0x8, 2, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0x8, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x8, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x8, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x8, 2, 1), 0x00000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x8, 2, 1), 0x80000000, 0x00000000, 0x80000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 2, 1), 0x80000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x8, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x8, 2, 1), 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x8, 2, 1), 0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, FLAG_N         , FLAG_Z                  },
    {OP(0x8, 2, 1), 0x55555555, 0xAAAAAAAA, 0x55555555, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 2, 1), 0xAAAAAAAA, 0xFFFFFFFF, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x8, 2, 1), 0xFFFFFFFF, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x8, 2, 1), 0x55555555, 0xFFFFFFFF, 0x55555555, 0              , 0                       },
    {OP(0x8, 2, 1), 0xFFFFFFFF, 0x55555555, 0xFFFFFFFF, FLAG_C         , FLAG_C                  },

    // NEG r1 r2 (RSB r1 r2 0)
    {OP(0x9, 2, 1), NO_SRC1   , 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 202
    {OP(0x9, 2, 1), NO_SRC1   , 0x00000000, 0x00000000, PSR_MASK       , FLAG_C | FLAG_Z         },
    {OP(0x9, 2, 1), NO_SRC1   , 0x00000001, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x9, 2, 1), NO_SRC1   , 0x7FFFFFFF, 0x80000001, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0x9, 2, 1), NO_SRC1   , 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x9, 2, 1), NO_SRC1   , 0x80000001, 0x7FFFFFFF, FLAG_C | FLAG_Z, 0                       },
    {OP(0x9, 2, 1), NO_SRC1   , 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_C, 0                       },

    // CMP r1 r2
    {OP(0xA, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 209
    {OP(0xA, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_C | FLAG_Z         },
    {OP(0xA, 2, 1), 0x00000000, 0x00000001, 0x00000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xA, 2, 1), 0x00000001, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xA, 2, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xA, 2, 1), 0x00000000, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xA, 2, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xA, 2, 1), 0x00000001, 0x7FFFFFFF, 0x00000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xA, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xA, 2, 1), 0x7FFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xA, 2, 1), 0x80000000, 0x7FFFFFFF, 0x80000000, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xA, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xA, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xA, 2, 1), 0x80000000, 0x00000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xA, 2, 1), 0x00000000, 0x80000000, 0x00000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xA, 2, 1), 0x80000000, 0x80000000, 0x80000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xA, 2, 1), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xA, 2, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_C, 0                       },
    {OP(0xA, 2, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xA, 2, 1), 0x00000001, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_N, 0                       },
    {OP(0xA, 2, 1), 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xA, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xA, 2, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // CMN r1 r2
    {OP(0xB, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 232
    {OP(0xB, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_Z                  },
    {OP(0xB, 2, 1), 0x00000000, 0x00000001, 0x00000000, FLAG_Z | FLAG_N, 0                       },
    {OP(0xB, 2, 1), 0x00000001, 0x00000000, 0x00000001, FLAG_C | FLAG_N, 0                       },
    {OP(0xB, 2, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, 0                       },
    {OP(0xB, 2, 1), 0x00000000, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_Z, 0                       },
    {OP(0xB, 2, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xB, 2, 1), 0x00000001, 0x7FFFFFFF, 0x00000001, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xB, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xB, 2, 1), 0x7FFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xB, 2, 1), 0x80000000, 0x7FFFFFFF, 0x80000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xB, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xB, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0xB, 2, 1), 0x80000000, 0x00000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xB, 2, 1), 0x00000000, 0x80000000, 0x00000000, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0xB, 2, 1), 0x80000000, 0x80000000, 0x80000000, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0xB, 2, 1), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xB, 2, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0xB, 2, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xB, 2, 1), 0x00000001, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xB, 2, 1), 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xB, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xB, 2, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },

    // ORR r1 r2
    {OP(0xC, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 255
    {OP(0xC, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0xC, 2, 1), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0xC, 2, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0xC, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0xC, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0xC, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C         , FLAG_C | FLAG_N         },
    {OP(0xC, 2, 1), 0x00000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xC, 2, 1), 0x80000000, 0x00000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xC, 2, 1), 0x80000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xC, 2, 1), 0x80000000, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xC, 2, 1), 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xC, 2, 1), 0xAAAAAAAA, 0x55555555, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xC, 2, 1), 0x55555555, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0xC, 2, 1), 0xAAAAAAAA, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_N         , FLAG_N                  },
    {OP(0xC, 2, 1), 0xFFFFFFFF, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xC, 2, 1), 0x55555555, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0xC, 2, 1), 0xFFFFFFFF, 0x55555555, 0xFFFFFFFF, FLAG_C         , FLAG_C | FLAG_N         },

    // MUL r1 r2
    // carry is a "meaningless value"
    {OP(0xD, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 273
    {OP(0xD, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0xD, 2, 1), 0x00000000, 0x00000001, 0x00000000, FLAG_N         , FLAG_Z                  }, // 0 * n
    {OP(0xD, 2, 1), 0x00000001, 0x00000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xD, 2, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x00000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xD, 2, 1), 0x00000001, 0x00000001, 0x00000001, FLAG_N         , 0                       }, // 1 * n
    {OP(0xD, 2, 1), 0x00000001, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_N         , 0                       },
    {OP(0xD, 2, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0xD, 2, 1), 0x00000001, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x80000000, 0x00000001, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x00000001, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x00000002, 0x30000000, 0x60000000, FLAG_Z         , 0                       }, // 2 * n
    {OP(0xD, 2, 1), 0x30000000, 0x00000002, 0x60000000, FLAG_N         , 0                       },
    {OP(0xD, 2, 1), 0x00000002, 0x40000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x40000000, 0x00000002, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x00000002, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x7FFFFFFF, 0x00000002, 0xFFFFFFFE, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x00000002, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xD, 2, 1), 0x80000000, 0x00000002, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xD, 2, 1), 0x00000002, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x00000002, 0xFFFFFFFE, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x00000003, 0x30000000, 0x90000000, FLAG_Z         , FLAG_N                  }, // 3 * n
    {OP(0xD, 2, 1), 0x30000000, 0x00000003, 0x90000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000001, FLAG_Z         , FLAG_N                  }, // -1 * n
    {OP(0xD, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000001, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, FLAG_Z         , 0                       },
    {OP(0xD, 2, 1), 0x01234567, 0x89ABCDEF, 0xC94E4629, FLAG_Z         , FLAG_N                  }, // extras
    {OP(0xD, 2, 1), 0x89ABCDEF, 0x01234567, 0xC94E4629, FLAG_Z         , FLAG_N                  },

    // BIC r1 r2
    {OP(0xE, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 305
    {OP(0xE, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0xE, 2, 1), 0x00000000, 0x7FFFFFFF, 0x00000000, 0              , FLAG_Z                  },
    {OP(0xE, 2, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, 0              , 0                       },
    {OP(0xE, 2, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xE, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x00000000, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0xE, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 2, 1), 0x00000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xE, 2, 1), 0x80000000, 0x00000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xE, 2, 1), 0x80000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xE, 2, 1), 0x80000000, 0xFFFFFFFF, 0x00000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xE, 2, 1), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0xE, 2, 1), 0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0xE, 2, 1), 0x55555555, 0xAAAAAAAA, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0xE, 2, 1), 0xAAAAAAAA, 0xFFFFFFFF, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xE, 2, 1), 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, FLAG_C         , FLAG_C                  },
    {OP(0xE, 2, 1), 0x55555555, 0xFFFFFFFF, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xE, 2, 1), 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },

    // MVN r1 r2
    {OP(0xF, 2, 1), NO_SRC1   , 0x00000000, 0xFFFFFFFF, 0              , FLAG_N                  }, // 323
    {OP(0xF, 2, 1), NO_SRC1   , 0x00000000, 0xFFFFFFFF, PSR_MASK       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0xF, 2, 1), NO_SRC1   , 0x7FFFFFFF, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xF, 2, 1), NO_SRC1   , 0xFFFFFFFF, 0x00000000, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0xF, 2, 1), NO_SRC1   , 0x80000000, 0x7FFFFFFF, FLAG_N         , 0                       },
    {OP(0xF, 2, 1), NO_SRC1   , 0x55555555, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0xF, 2, 1), NO_SRC1   , 0xAAAAAAAA, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
};

#undef OP

static const int num_dp_tests = sizeof(dp_tests) / sizeof(dp_tests[0]);

// high reg
#define OP(op, rm, rdn) (0x4400 | op << 8 | (rdn & 8) << 4 | rm << 3 | (rdn & 7))

static const struct TestInfo hi_reg_tests[] = {
    // ADD r1 r9
    // does not affect flags
    {OP(0, 9, 1), 0x00000000, 0x00000000, 0x00000000, 0              , 0                       }, // 0
    {OP(0, 9, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0x00000000, 0x00000001, 0x00000001, 0              , 0                       },
    {OP(0, 9, 1), 0x00000001, 0x00000000, 0x00000001, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, 0              , 0                       },
    {OP(0, 9, 1), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0x7FFFFFFF, 0x00000001, 0x80000000, 0              , 0                       },
    {OP(0, 9, 1), 0x00000001, 0x7FFFFFFF, 0x80000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFE, 0              , 0                       },
    {OP(0, 9, 1), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0              , 0                       },
    {OP(0, 9, 1), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFE, 0              , 0                       },
    {OP(0, 9, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFE, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0x80000000, 0x00000000, 0x80000000, 0              , 0                       },
    {OP(0, 9, 1), 0x00000000, 0x80000000, 0x80000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0x80000000, 0x80000000, 0x00000000, 0              , 0                       },
    {OP(0, 9, 1), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0              , 0                       },
    {OP(0, 9, 1), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0xFFFFFFFF, 0x00000001, 0x00000000, 0              , 0                       },
    {OP(0, 9, 1), 0x00000001, 0xFFFFFFFF, 0x00000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, 0              , 0                       },
    {OP(0, 9, 1), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0              , 0                       },

    // ADD r8 r2
    // does not affect flags
    {OP(0, 2, 8), 0x00000000, 0x00000000, 0x00000000, 0              , 0                       }, // 23
    {OP(0, 2, 8), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0x00000000, 0x00000001, 0x00000001, 0              , 0                       },
    {OP(0, 2, 8), 0x00000001, 0x00000000, 0x00000001, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, 0              , 0                       },
    {OP(0, 2, 8), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0x7FFFFFFF, 0x00000001, 0x80000000, 0              , 0                       },
    {OP(0, 2, 8), 0x00000001, 0x7FFFFFFF, 0x80000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFE, 0              , 0                       },
    {OP(0, 2, 8), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0              , 0                       },
    {OP(0, 2, 8), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFE, 0              , 0                       },
    {OP(0, 2, 8), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFE, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0x80000000, 0x00000000, 0x80000000, 0              , 0                       },
    {OP(0, 2, 8), 0x00000000, 0x80000000, 0x80000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0x80000000, 0x80000000, 0x00000000, 0              , 0                       },
    {OP(0, 2, 8), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0              , 0                       },
    {OP(0, 2, 8), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0xFFFFFFFF, 0x00000001, 0x00000000, 0              , 0                       },
    {OP(0, 2, 8), 0x00000001, 0xFFFFFFFF, 0x00000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, 0              , 0                       },
    {OP(0, 2, 8), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 2, 8), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0              , 0                       },

    // ADD r8 r9
    // does not affect flags
    {OP(0, 9, 8), 0x00000000, 0x00000000, 0x00000000, 0              , 0                       }, // 46
    {OP(0, 9, 8), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0x00000000, 0x00000001, 0x00000001, 0              , 0                       },
    {OP(0, 9, 8), 0x00000001, 0x00000000, 0x00000001, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, 0              , 0                       },
    {OP(0, 9, 8), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0x7FFFFFFF, 0x00000001, 0x80000000, 0              , 0                       },
    {OP(0, 9, 8), 0x00000001, 0x7FFFFFFF, 0x80000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFE, 0              , 0                       },
    {OP(0, 9, 8), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0              , 0                       },
    {OP(0, 9, 8), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFE, 0              , 0                       },
    {OP(0, 9, 8), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFE, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0x80000000, 0x00000000, 0x80000000, 0              , 0                       },
    {OP(0, 9, 8), 0x00000000, 0x80000000, 0x80000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0x80000000, 0x80000000, 0x00000000, 0              , 0                       },
    {OP(0, 9, 8), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0              , 0                       },
    {OP(0, 9, 8), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0xFFFFFFFF, 0x00000001, 0x00000000, 0              , 0                       },
    {OP(0, 9, 8), 0x00000001, 0xFFFFFFFF, 0x00000000, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, 0              , 0                       },
    {OP(0, 9, 8), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, PSR_MASK       , PSR_MASK                },
    {OP(0, 9, 8), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0              , 0                       },

    // CMP r1 r9
    {OP(1, 9, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 69
    {OP(1, 9, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_C | FLAG_Z         },
    {OP(1, 9, 1), 0x00000000, 0x00000001, 0x00000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 9, 1), 0x00000001, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 9, 1), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 9, 1), 0x00000000, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(1, 9, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 9, 1), 0x00000001, 0x7FFFFFFF, 0x00000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 9, 1), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(1, 9, 1), 0x7FFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 9, 1), 0x80000000, 0x7FFFFFFF, 0x80000000, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(1, 9, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 9, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 9, 1), 0x80000000, 0x00000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 9, 1), 0x00000000, 0x80000000, 0x00000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 9, 1), 0x80000000, 0x80000000, 0x80000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(1, 9, 1), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 9, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_C, 0                       },
    {OP(1, 9, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 9, 1), 0x00000001, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_N, 0                       },
    {OP(1, 9, 1), 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 9, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 9, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // CMP r8 r2
    {OP(1, 2, 8), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 92
    {OP(1, 2, 8), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_C | FLAG_Z         },
    {OP(1, 2, 8), 0x00000000, 0x00000001, 0x00000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 2, 8), 0x00000001, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 2, 8), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 2, 8), 0x00000000, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(1, 2, 8), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 2, 8), 0x00000001, 0x7FFFFFFF, 0x00000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 2, 8), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(1, 2, 8), 0x7FFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 2, 8), 0x80000000, 0x7FFFFFFF, 0x80000000, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(1, 2, 8), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 2, 8), 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 2, 8), 0x80000000, 0x00000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 2, 8), 0x00000000, 0x80000000, 0x00000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 2, 8), 0x80000000, 0x80000000, 0x80000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(1, 2, 8), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 2, 8), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_C, 0                       },
    {OP(1, 2, 8), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 2, 8), 0x00000001, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_N, 0                       },
    {OP(1, 2, 8), 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 2, 8), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 2, 8), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // CMP r8 r9
    {OP(1, 9, 8), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 115
    {OP(1, 9, 8), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_C | FLAG_Z         },
    {OP(1, 9, 8), 0x00000000, 0x00000001, 0x00000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 9, 8), 0x00000001, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 9, 8), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 9, 8), 0x00000000, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(1, 9, 8), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 9, 8), 0x00000001, 0x7FFFFFFF, 0x00000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 9, 8), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(1, 9, 8), 0x7FFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 9, 8), 0x80000000, 0x7FFFFFFF, 0x80000000, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(1, 9, 8), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 9, 8), 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 9, 8), 0x80000000, 0x00000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 9, 8), 0x00000000, 0x80000000, 0x00000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(1, 9, 8), 0x80000000, 0x80000000, 0x80000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(1, 9, 8), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 9, 8), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_C, 0                       },
    {OP(1, 9, 8), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(1, 9, 8), 0x00000001, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_N, 0                       },
    {OP(1, 9, 8), 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 9, 8), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(1, 9, 8), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // MOV
    // does not affect flags
    // also, the other tests rely on this working
    {OP(2, 2, 8), NO_SRC1   , 0x00000000, 0x00000000, 0              , 0                       }, // 138,  r8 r2
    {OP(2, 2, 8), NO_SRC1   , 0x00000000, 0x00000000, PSR_MASK       , PSR_MASK                },
    {OP(2, 2, 8), NO_SRC1   , 0x80000000, 0x80000000, 0              , 0                       },
    {OP(2, 2, 8), NO_SRC1   , 0x80000000, 0x80000000, PSR_MASK       , PSR_MASK                },
    {OP(2, 9, 1), NO_SRC1   , 0x00000000, 0x00000000, 0              , 0                       }, // r1 r9
    {OP(2, 9, 1), NO_SRC1   , 0x00000000, 0x00000000, PSR_MASK       , PSR_MASK                },
    {OP(2, 9, 1), NO_SRC1   , 0x80000000, 0x80000000, 0              , 0                       },
    {OP(2, 9, 1), NO_SRC1   , 0x80000000, 0x80000000, PSR_MASK       , PSR_MASK                },
    {OP(2, 9, 8), NO_SRC1   , 0x00000000, 0x00000000, 0              , 0                       }, // r8 r9
    {OP(2, 9, 8), NO_SRC1   , 0x00000000, 0x00000000, PSR_MASK       , PSR_MASK                },
    {OP(2, 9, 8), NO_SRC1   , 0x80000000, 0x80000000, 0              , 0                       },
    {OP(2, 9, 8), NO_SRC1   , 0x80000000, 0x80000000, PSR_MASK       , PSR_MASK                },

    // BX
    // branches
};

#undef OP

static const int num_hi_reg_tests = sizeof(hi_reg_tests) / sizeof(hi_reg_tests[0]);

// load byte/word, register offset
#define OP(byte, ro, rb, rd) (0x5800 | byte << 10 | ro << 6 | rb << 3 | rd)

static const struct TestInfo load_reg_tests[] = {
    // ldr r0 [r2 r1]
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr    , 0x01234567, 0, 0}, // 0
    {OP(0, 1, 2, 0), 0x00000004, test_data_addr    , 0x89ABCDEF, 0, 0},
    {OP(0, 1, 2, 0), 0x00000000, test_data_addr + 4, 0x89ABCDEF, 0, 0},
    {OP(0, 1, 2, 0), 0xFFFFFFFC, test_data_addr + 4, 0x01234567, 0, 0},

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
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

#ifndef __ARM_ARCH_6M__ // fault (maybe test that they do?)
    // misaligned
    {OP(0, 0), 0, test_data_addr + 1, 0x7E57DA7A, 0, 0},
    {OP(0, 0), 0, test_data_addr + 2, 0x7E57DA7A, 0, 0},
    {OP(0, 0), 0, test_data_addr + 3, 0x7E57DA7A, 0, 0},
#endif
};

#undef OP

static const int num_store_sp_rel_tests = sizeof(store_sp_rel_tests) / sizeof(store_sp_rel_tests[0]);

// add/sub sp
// test code moves to/from sp
#define OP(s, word) (0xB000 | s <<  7 | word)

static const struct TestInfo sp_offset_tests[] = {
    // ADD sp #imm
    {OP(0, 0x00), 0x00000000, NO_SRC2, 0x00000000, 0, 0}, // 0
    {OP(0, 0x01), 0x00000000, NO_SRC2, 0x00000004, 0, 0},
    {OP(0, 0x7F), 0x00000000, NO_SRC2, 0x000001FC, 0, 0},
    {OP(0, 0x00), 0x00000004, NO_SRC2, 0x00000004, 0, 0},
    {OP(0, 0x01), 0x00000004, NO_SRC2, 0x00000008, 0, 0},
    {OP(0, 0x7F), 0x00000004, NO_SRC2, 0x00000200, 0, 0},

#ifdef __ARM_ARCH_6M__ // can't have unaligned SP
    {OP(0, 0x00), 0x00000001, NO_SRC2, 0x00000000, 0, 0},
    {OP(0, 0x01), 0x00000001, NO_SRC2, 0x00000004, 0, 0},
    {OP(0, 0x7F), 0x00000001, NO_SRC2, 0x000001FC, 0, 0},
    {OP(0, 0x00), 0x00000002, NO_SRC2, 0x00000000, 0, 0},
    {OP(0, 0x01), 0x00000002, NO_SRC2, 0x00000004, 0, 0},
    {OP(0, 0x7F), 0x00000002, NO_SRC2, 0x000001FC, 0, 0},
#else
    {OP(0, 0x00), 0x00000001, NO_SRC2, 0x00000001, 0, 0},
    {OP(0, 0x01), 0x00000001, NO_SRC2, 0x00000005, 0, 0},
    {OP(0, 0x7F), 0x00000001, NO_SRC2, 0x000001FD, 0, 0},
    {OP(0, 0x00), 0x00000002, NO_SRC2, 0x00000002, 0, 0},
    {OP(0, 0x01), 0x00000002, NO_SRC2, 0x00000006, 0, 0},
    {OP(0, 0x7F), 0x00000002, NO_SRC2, 0x000001FE, 0, 0},
#endif

    // SUB sp #imm
    {OP(1, 0x00), 0x00000000, NO_SRC2, 0x00000000, 0, 0}, // 12
    {OP(1, 0x01), 0x00000000, NO_SRC2, 0xFFFFFFFC, 0, 0},
    {OP(1, 0x7F), 0x00000000, NO_SRC2, 0xFFFFFE04, 0, 0},
    {OP(1, 0x00), 0x00000004, NO_SRC2, 0x00000004, 0, 0},
    {OP(1, 0x01), 0x00000004, NO_SRC2, 0x00000000, 0, 0},
    {OP(1, 0x7F), 0x00000004, NO_SRC2, 0xFFFFFE08, 0, 0},

#ifdef __ARM_ARCH_6M__ // can't have unaligned SP
    {OP(1, 0x00), 0x00000001, NO_SRC2, 0x00000000, 0, 0},
    {OP(1, 0x01), 0x00000001, NO_SRC2, 0xFFFFFFFC, 0, 0},
    {OP(1, 0x7F), 0x00000001, NO_SRC2, 0xFFFFFE04, 0, 0},
    {OP(1, 0x00), 0x00000002, NO_SRC2, 0x00000000, 0, 0},
    {OP(1, 0x01), 0x00000002, NO_SRC2, 0xFFFFFFFC, 0, 0},
    {OP(1, 0x7F), 0x00000002, NO_SRC2, 0xFFFFFE04, 0, 0},
#else
    {OP(1, 0x00), 0x00000001, NO_SRC2, 0x00000001, 0, 0},
    {OP(1, 0x01), 0x00000001, NO_SRC2, 0xFFFFFFFD, 0, 0},
    {OP(1, 0x7F), 0x00000001, NO_SRC2, 0xFFFFFE05, 0, 0},
    {OP(1, 0x00), 0x00000002, NO_SRC2, 0x00000002, 0, 0},
    {OP(1, 0x01), 0x00000002, NO_SRC2, 0xFFFFFFFE, 0, 0},
    {OP(1, 0x7F), 0x00000002, NO_SRC2, 0xFFFFFE06, 0, 0},
#endif
};

#undef OP

static const int num_sp_offset_tests = sizeof(sp_offset_tests) / sizeof(sp_offset_tests[0]);

#ifdef __ARM_ARCH_6M__

#if defined(PICO_BUILD)
#include <pico/platform.h>
#define in_ram(fn) __not_in_flash_func(fn)
#else
#error need to place helper in ram
#endif

void in_ram(set_cpsr)(uint32_t v) {
    asm volatile(
        "msr apsr, %0"
        :
        : "r"(v)
    );
}

static uint32_t get_cpsr_arm() {
    uint32_t ret;
    asm volatile(
        "mrs %0, apsr"
        : "=r"(ret)
    );

    return ret;
}
#else
// PSR helpers as we can't access from thumb
__attribute__((target("arm")))
static void set_cpsr_arm(uint32_t v) {
    asm volatile(
        "msr cpsr, %0"
        :
        : "r"(v)
    );
}

// needs to be in the same section as the code buffer
__attribute__((section(".iwram.set_cpsr")))
void set_cpsr(uint32_t v) {
    set_cpsr_arm(v);
}

__attribute__((target("arm")))
static uint32_t get_cpsr_arm() {
    uint32_t ret;
    asm volatile(
        "mrs %0, cpsr"
        : "=r"(ret)
    );

    return ret;
}

#endif

bool run_test_list(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo *tests, int num_tests, const char *label, int dest, bool flags_for_val) {

    bool res = true;

    group_cb(label);

    uint32_t psr_save = get_cpsr_arm() & ~PSR_MASK;

    int set_cpsr_off = (uintptr_t)set_cpsr - ((uintptr_t)code_buf + 6);

    for(int i = 0; i < num_tests; i++) {
        const struct TestInfo *test = &tests[i];
    
        // value test
        uint16_t *ptr = code_buf;

        if(flags_for_val) {
            *ptr++ = 0xB500; // push lr
            *ptr++ = 0xF000 | ((set_cpsr_off >> 12) & 0x7FF); // bl set_cpsr
            *ptr++ = 0xF800 | ((set_cpsr_off >> 1) & 0x7FF); // bl set_cpsr
        }

        if(dest == 13) {
            // setup sp to known value for sp offset tests
            *ptr++ = 0x466B; // mov r3 sp
            *ptr++ = 0x468D; // mov sp r1
        }

        *ptr++ = test->opcode;

        if(dest == 13) {
            *ptr++ = 0x4668; // mov r0 sp
            *ptr++ = 0x469D; // mov sp r3
        }
        else if(dest != 0) // assume < 8
            *ptr++ = dest << 3; // mov r0 rN
        
        if(flags_for_val)
            *ptr++ = 0xBD00; // pop pc
        else
            *ptr++ = 0x4770; // BX LR

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

        uint32_t in0 = flags_for_val ? (test->psr_in | psr_save) : 0xBAD;

        uint32_t out = func(in0, test->m_in, test->n_in, 0x3BAD);

        if(out != test->d_out) {
            res = false;
            fail_cb(i, out, test->d_out);
        }

        if(dest == 13)
            continue; // don't run the flags tests for sp offset tests

        // flags test
        // this relies on the PSR helpers not affecting anything other than R0
        code_buf[0] = 0xB500; // push lr
        code_buf[1] = 0xF000 | ((set_cpsr_off >> 12) & 0x7FF); // bl set_cpsr
        code_buf[2] = 0xF800 | ((set_cpsr_off >> 1) & 0x7FF); // bl set_cpsr

        code_buf[3] = test->opcode;

        code_buf[4] = 0xBC01; // pop r0
        code_buf[5] = 0x4686; // mov lr r0
        code_buf[6] = 0x4718; // bx r3

        out = func(test->psr_in | psr_save, test->m_in, test->n_in, (intptr_t)get_cpsr_arm);
        out &= PSR_MASK;

        if(out != test->psr_out) {
            res = false;
            fail_cb(i, out, test->psr_out);
        }

        // single flag tests
        // (to try and break my recompiler)
        for(int j = 0; j < 4; j++) {
            // Z C N V
            static const uint32_t flags[] = {FLAG_Z, FLAG_C, FLAG_N, FLAG_V};

            code_buf[4] = 0xD001 | (j << 9); // Bcc +4
            // flag not set
            code_buf[5] = 0x1A00; // sub r0 r0 r0
            code_buf[6] = 0xE001; // B + 4
            // flag set
            code_buf[7] = 0x1A00; // sub r0 r0 r0
            code_buf[8] = 0x3001; // add r0 1

            code_buf[9] = 0xBD00; // pop pc

            out = func(test->psr_in | psr_save, test->m_in, test->n_in, 0x3BAD);

            if(out != !!(test->psr_out & flags[j])) {
                res = false;
                fail_cb(i, out ? flags[j] : 0, test->psr_out & flags[j]);
            }
        }

    }

    return res;
}

bool run_hi_reg_tests(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo *tests, int num_tests, const char *label) {

    bool res = true;

    group_cb(label);

    uint32_t psr_save = get_cpsr_arm() & ~PSR_MASK;

    int set_cpsr_off = (uintptr_t)set_cpsr - ((uintptr_t)code_buf + 6);

    for(int i = 0; i < num_tests; i++) {
        const struct TestInfo *test = &tests[i];

        bool hiDest = test->opcode & (1 << 7);
        bool hiSrc = test->opcode & (1 << 6);

        // value test
        uint16_t *ptr = code_buf;

        // save r8
        *ptr++ = 0x4640; // mov r0 r8
        *ptr++ = 0xB401; // push r0
        // save r9
        *ptr++ = 0x4648; // mov r0 r9
        *ptr++ = 0xB401; // push r0

        // move src/dst to high reg
        if(hiSrc)
            *ptr++ = 0x4691; // mov r9 r2
        
        if(hiDest)
            *ptr++ = 0x4688; // mov r8 r1

        *ptr++ = test->opcode;

        // move dst to ret
        if(hiDest)
            *ptr++ = 0x4640; // mov r0 r8
        else
            *ptr++ = 0x0008; // mov r0 r1

        // restore r9
        *ptr++ = 0xBC02; // pop r1
        *ptr++ = 0x4689; // mov r9 r1
        // restore r8
        *ptr++ = 0xBC02; // pop r1
        *ptr++ = 0x4688; // mov r8 r1
        
        *ptr++ = 0x4770; // BX LR

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

        uint32_t out = func(0xBAD, test->m_in, test->n_in, 0x3BAD);

        if(out != test->d_out) {
            res = false;
            fail_cb(i, out, test->d_out);
        }

        // flags test
        // this relies on the PSR helpers not affecting anything other than R0
        ptr = code_buf;
        *ptr++ = 0xB500; // push lr
        *ptr++ = 0xF000 | ((set_cpsr_off >> 12) & 0x7FF); // bl set_cpsr
        *ptr++ = 0xF800 | ((set_cpsr_off >> 1) & 0x7FF); // bl set_cpsr

        *ptr++ = 0x4640; // mov r0 r8
        *ptr++ = 0xB401; // push r0
        *ptr++ = 0x4648; // mov r0 r9
        *ptr++ = 0xB401; // push r0

        if(hiSrc)
            *ptr++ = 0x4691; // mov r9 r2
        
        if(hiDest)
            *ptr++ = 0x4688; // mov r8 r1

        *ptr++ =test->opcode;

        *ptr++ = 0xBC02; // pop r1
        *ptr++ = 0x4689; // mov r9 r1
        *ptr++ = 0xBC02; // pop r1
        *ptr++ = 0x4688; // mov r8 r1

        *ptr++ = 0xBC01; // pop r0
        *ptr++ = 0x4686; // mov lr r0
        *ptr++ = 0x4718; // bx r3

        out = func(test->psr_in | psr_save, test->m_in, test->n_in, (intptr_t)get_cpsr_arm);
        out &= PSR_MASK;

        if(out != test->psr_out) {
            res = false;
            fail_cb(i, out, test->psr_out);
        }
    }

    return res;
}

bool run_pc_rel_load_tests(GroupCallback group_cb, FailCallback fail_cb, const char *label) {

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

    uint32_t out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);

    uint32_t expected = 0x45670123;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // now with an offset
    i++;
    code_buf[0] = OP(0, 1); // r0 4

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

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = 0x01234770;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // ... and with an offset again
    i++;
    code_buf[1] = OP(0, 1); // r0 4

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = 0x89AB4567;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    #undef OP

    return res;
}

bool run_load_store_tests(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo *tests, int num_tests, const char *label, bool is_store) {

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

bool run_sp_rel_load_store_tests(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo *tests, int num_tests, const char *label, bool is_store) {

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

bool run_load_addr_tests(GroupCallback group_cb, FailCallback fail_cb, const char *label) {

    bool res = true;

    group_cb(label);
    int i = 0;

    #define OP(sp, rd, word) (0xA000 | sp << 11 | rd << 8 | word)

    // + 0
    uint16_t *ptr = code_buf;

    *ptr++ = OP(0, 0, 0); // r0 pc 0

    *ptr++ = 0x4770; // BX LR
    uint16_t *end_ptr = ptr;

    TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

    uint32_t out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);

    uint32_t expected = (uintptr_t)code_buf + 4;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // now with an offset
    i++;
    code_buf[0] = OP(0, 0, 1); // r0 pc 4

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = (uintptr_t)code_buf + 8;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // big offset
    i++;
    code_buf[0] = OP(0, 0, 0xFF); // r0 pc 1020

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = (uintptr_t)code_buf + 1024;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // now repeat for SP
    uint32_t spIn;
    asm volatile(
        "mov %0, sp"
        : "=r"(spIn)
    );

    // + 0
    i++;
    code_buf[0] = OP(1, 0, 0); // r0 sp 0

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = spIn;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // now with an offset
    i++;
    code_buf[0] = OP(1, 0, 1); // r0 sp 4

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = spIn + 4;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // big offset
    i++;
    code_buf[0] = OP(1, 0, 0xFF); // r0 sp 1020

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = spIn + 1020;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    #undef OP

    return res;
}

bool run_push_pop_tests(GroupCallback group_cb, FailCallback fail_cb, const char *label) {
    static const uint32_t values[] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210};
    bool res = true;

    group_cb(label);
    int i = 0;

    #define OP(load, pclr, regs) (0xB400 | load << 11 | pclr << 8 | regs)

    // push r0-4, pop single (0-3)
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
        uint16_t *end_ptr = ptr;

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

        uint32_t out = func(values[0], values[1], values[2], values[3]);

        if(out != expected[j]) {
            res = false;
            fail_cb(i, out, expected[j]);
        }
    }

    // push single, pop r0-4 (4-7)
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
        uint16_t *end_ptr = ptr;

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

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
        uint16_t *end_ptr = ptr;

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

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
    uint16_t *end_ptr = ptr;

    TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

    uint32_t out = func(values[0], values[1], values[2], values[3]);

    uint32_t expected = 0x01234567;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    #undef OP

    return res;
}

bool run_tests(GroupCallback group_cb, FailCallback fail_cb) {
    
    bool ret = true;

    ret = run_test_list(group_cb, fail_cb, shift_imm_tests, num_shift_imm_tests, "sh.imm", 0, false) && ret;
    ret = run_test_list(group_cb, fail_cb, add_sub_tests, num_add_sub_tests, "addsub", 0, false) && ret;
    ret = run_test_list(group_cb, fail_cb, mov_cmp_add_sub_imm_tests, num_mov_cmp_add_sub_imm_tests, "dp.imm", 1, false) && ret;
    ret = run_test_list(group_cb, fail_cb, dp_tests, num_dp_tests, "dp", 1, true) && ret;
    ret = run_hi_reg_tests(group_cb, fail_cb, hi_reg_tests, num_hi_reg_tests, "hireg") && ret;
    ret = run_pc_rel_load_tests(group_cb, fail_cb, "pcrell") && ret;
    ret = run_load_store_tests(group_cb, fail_cb, load_reg_tests, num_load_reg_tests, "ldr.reg", false) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, store_reg_tests, num_store_reg_tests, "str.reg", true) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, load_sign_ex_tests, num_load_sign_ex_tests, "ldr.sx", false) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, store_half_reg_tests, num_store_half_reg_tests, "strh.reg", true) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, load_imm_off_tests, num_load_imm_off_tests, "ldr.imm", false) && ret;
    ret = run_load_store_tests(group_cb, fail_cb, store_imm_off_tests, num_store_imm_off_tests, "str.imm", true) && ret;
    ret = run_sp_rel_load_store_tests(group_cb, fail_cb, load_sp_rel_tests, num_load_sp_rel_tests, "ldr.sp", false) && ret;
    ret = run_sp_rel_load_store_tests(group_cb, fail_cb, store_sp_rel_tests, num_store_sp_rel_tests, "str.sp", true) && ret;
    ret = run_load_addr_tests(group_cb, fail_cb, "ldaddr") && ret;
    ret = run_test_list(group_cb, fail_cb, sp_offset_tests, num_sp_offset_tests, "spoff", 13, false) && ret;
    ret = run_push_pop_tests(group_cb, fail_cb, "pushpop") && ret;

    return ret;
}