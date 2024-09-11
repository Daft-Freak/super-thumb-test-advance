#include "tests.h"
#include "tests_common.h"
#include "load_store_tests.h"
#include "thumb2_data_processing_tests.h"

#ifdef TARGET_32BLIT_HW
__attribute__((section(".data"))) // bss would be an entirely separate memory region
#endif
_Alignas(4) uint16_t code_buf[32];

// test list

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
    // carry is a "meaningless value" (v4)
    {OP(0xD, 2, 1), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 273
#if __ARM_ARCH >= 6
    {OP(0xD, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_V | FLAG_C | FLAG_Z},
#else
    {OP(0xD, 2, 1), 0x00000000, 0x00000000, 0x00000000, PSR_MASK       , FLAG_V | FLAG_Z         },
#endif
    {OP(0xD, 2, 1), 0x00000000, 0x00000001, 0x00000000, FLAG_N         , FLAG_Z                  }, // 0 * n
    {OP(0xD, 2, 1), 0x00000001, 0x00000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xD, 2, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x00000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0xD, 2, 1), 0x00000001, 0x00000001, 0x00000001, FLAG_N         , 0                       }, // 1 * n
    {OP(0xD, 2, 1), 0x00000001, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_N         , 0                       },
    {OP(0xD, 2, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0xD, 2, 1), 0x00000001, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
#if __ARM_ARCH >= 6
    {OP(0xD, 2, 1), 0x80000000, 0x00000001, 0x80000000, FLAG_Z         , FLAG_N                  },
#else
    {OP(0xD, 2, 1), 0x80000000, 0x00000001, 0x80000000, FLAG_Z         , FLAG_C | FLAG_N         },
#endif
    {OP(0xD, 2, 1), 0x00000001, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x00000002, 0x30000000, 0x60000000, FLAG_Z         , 0                       }, // 2 * n
    {OP(0xD, 2, 1), 0x30000000, 0x00000002, 0x60000000, FLAG_N         , 0                       },
    {OP(0xD, 2, 1), 0x00000002, 0x40000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x40000000, 0x00000002, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x00000002, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x7FFFFFFF, 0x00000002, 0xFFFFFFFE, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x00000002, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
#if __ARM_ARCH >= 6
    {OP(0xD, 2, 1), 0x80000000, 0x00000002, 0x00000000, FLAG_N         , FLAG_Z                  },
#else
    {OP(0xD, 2, 1), 0x80000000, 0x00000002, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
#endif
    {OP(0xD, 2, 1), 0x00000002, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x00000002, 0xFFFFFFFE, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0x00000003, 0x30000000, 0x90000000, FLAG_Z         , FLAG_N                  }, // 3 * n
    {OP(0xD, 2, 1), 0x30000000, 0x00000003, 0x90000000, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000001, FLAG_Z         , FLAG_N                  }, // -1 * n
    {OP(0xD, 2, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000001, FLAG_Z         , FLAG_N                  },
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
#if __ARM_ARCH >= 6
    {OP(0xD, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_N                  },
#else
    {OP(0xD, 2, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_C | FLAG_N         },
#endif
    {OP(0xD, 2, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, FLAG_Z         , 0                       },
    {OP(0xD, 2, 1), 0x01234567, 0x89ABCDEF, 0xC94E4629, FLAG_Z         , FLAG_N                  }, // extras
#if __ARM_ARCH >= 6
    {OP(0xD, 2, 1), 0x89ABCDEF, 0x01234567, 0xC94E4629, FLAG_Z         , FLAG_N                  },
#else
    {OP(0xD, 2, 1), 0x89ABCDEF, 0x01234567, 0xC94E4629, FLAG_Z         , FLAG_C | FLAG_N         },
#endif

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

#if __ARM_ARCH >= 6 // can't have unaligned SP
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

#if __ARM_ARCH >= 6 // can't have unaligned SP
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

    // some extras with hireg add
    {0x4495     , 0x00000000, 0x00000004, 0x00000004, 0, 0}, // 24
#if __ARM_ARCH >= 6 // can't have unaligned SP
    {0x4495     , 0x00000000, 0x00000002, 0x00000000, 0, 0},
    {0x4495     , 0x00000003, 0x00000001, 0x00000000, 0, 0},
    {0x4495     , 0x00000002, 0x00000002, 0x00000000, 0, 0},
#else
    {0x4495     , 0x00000000, 0x00000001, 0x00000001, 0, 0},
    {0x4495     , 0x00000000, 0x00000002, 0x00000002, 0, 0},
    {0x4495     , 0x00000003, 0x00000001, 0x00000004, 0, 0},
    {0x4495     , 0x00000002, 0x00000002, 0x00000004, 0, 0},
#endif
};

#undef OP

static const int num_sp_offset_tests = sizeof(sp_offset_tests) / sizeof(sp_offset_tests[0]);

// v6+ only tests
#if __ARM_ARCH >= 6

#define OP(op, rm, rd) (0xB200 | op <<  6 | rm << 3 | rd)

// none of these affect flags
static const struct TestInfo extend_tests[] = {
    // SXTH r0 r1
    {OP(0, 1, 0), 0x12340000, 0, 0x00000000, 0              , 0       }, // 0
    {OP(0, 1, 0), 0x89AB0000, 0, 0x00000000, PSR_MASK       , PSR_MASK},
    {OP(0, 1, 0), 0x123400FF, 0, 0x000000FF, 0              , 0       },
    {OP(0, 1, 0), 0x89AB00FF, 0, 0x000000FF, PSR_MASK       , PSR_MASK},
    {OP(0, 1, 0), 0x12347FFF, 0, 0x00007FFF, 0              , 0       },
    {OP(0, 1, 0), 0x89AB7FFF, 0, 0x00007FFF, PSR_MASK       , PSR_MASK},
    {OP(0, 1, 0), 0x12348000, 0, 0xFFFF8000, 0              , 0       },
    {OP(0, 1, 0), 0x89AB8000, 0, 0xFFFF8000, PSR_MASK       , PSR_MASK},

    // SXTB r0 r1
    {OP(1, 1, 0), 0x12345600, 0, 0x00000000, 0              , 0       }, // 8
    {OP(1, 1, 0), 0x89ABCD00, 0, 0x00000000, PSR_MASK       , PSR_MASK},
    {OP(1, 1, 0), 0x1234560F, 0, 0x0000000F, 0              , 0       },
    {OP(1, 1, 0), 0x89ABCD0F, 0, 0x0000000F, PSR_MASK       , PSR_MASK},
    {OP(1, 1, 0), 0x1234567F, 0, 0x0000007F, 0              , 0       },
    {OP(1, 1, 0), 0x89ABCD7F, 0, 0x0000007F, PSR_MASK       , PSR_MASK},
    {OP(1, 1, 0), 0x12345680, 0, 0xFFFFFF80, 0              , 0       },
    {OP(1, 1, 0), 0x89ABCD80, 0, 0xFFFFFF80, PSR_MASK       , PSR_MASK},

    // UXTH r0 r1
    {OP(2, 1, 0), 0x12340000, 0, 0x00000000, 0              , 0       }, // 16
    {OP(2, 1, 0), 0x89AB0000, 0, 0x00000000, PSR_MASK       , PSR_MASK},
    {OP(2, 1, 0), 0x123400FF, 0, 0x000000FF, 0              , 0       },
    {OP(2, 1, 0), 0x89AB00FF, 0, 0x000000FF, PSR_MASK       , PSR_MASK},
    {OP(2, 1, 0), 0x12347FFF, 0, 0x00007FFF, 0              , 0       },
    {OP(2, 1, 0), 0x89AB7FFF, 0, 0x00007FFF, PSR_MASK       , PSR_MASK},
    {OP(2, 1, 0), 0x12348000, 0, 0x00008000, 0              , 0       },
    {OP(2, 1, 0), 0x89AB8000, 0, 0x00008000, PSR_MASK       , PSR_MASK},

    // UXTB r0 r1
    {OP(3, 1, 0), 0x12345600, 0, 0x00000000, 0              , 0       }, // 24
    {OP(3, 1, 0), 0x89ABCD00, 0, 0x00000000, PSR_MASK       , PSR_MASK},
    {OP(3, 1, 0), 0x1234560F, 0, 0x0000000F, 0              , 0       },
    {OP(3, 1, 0), 0x89ABCD0F, 0, 0x0000000F, PSR_MASK       , PSR_MASK},
    {OP(3, 1, 0), 0x1234567F, 0, 0x0000007F, 0              , 0       },
    {OP(3, 1, 0), 0x89ABCD7F, 0, 0x0000007F, PSR_MASK       , PSR_MASK},
    {OP(3, 1, 0), 0x12345680, 0, 0x00000080, 0              , 0       },
    {OP(3, 1, 0), 0x89ABCD80, 0, 0x00000080, PSR_MASK       , PSR_MASK},
};

#undef OP

static const int num_extend_tests = sizeof(extend_tests) / sizeof(extend_tests[0]);

#define OP(op, rm, rd) (0xBA00 | op <<  6 | rm << 3 | rd)

// none of these affect flags
static const struct TestInfo reverse_tests[] = {
    // REV r0 r1
    {OP(0, 1, 0), 0x01234567, 0, 0x67452301, 0              , 0       }, // 0
    {OP(0, 1, 0), 0x01234567, 0, 0x67452301, PSR_MASK       , PSR_MASK},
    {OP(0, 1, 0), 0x67452301, 0, 0x01234567, 0              , 0       },
    {OP(0, 1, 0), 0x67452301, 0, 0x01234567, PSR_MASK       , PSR_MASK},
    {OP(0, 1, 0), 0x89ABCDEF, 0, 0xEFCDAB89, 0              , 0       },
    {OP(0, 1, 0), 0x89ABCDEF, 0, 0xEFCDAB89, PSR_MASK       , PSR_MASK},
    {OP(0, 1, 0), 0xEFCDAB89, 0, 0x89ABCDEF, 0              , 0       },
    {OP(0, 1, 0), 0xEFCDAB89, 0, 0x89ABCDEF, PSR_MASK       , PSR_MASK},

    // REV16 r0 r1
    {OP(1, 1, 0), 0x01234567, 0, 0x23016745, 0              , 0       }, // 8
    {OP(1, 1, 0), 0x01234567, 0, 0x23016745, PSR_MASK       , PSR_MASK},
    {OP(1, 1, 0), 0x23016745, 0, 0x01234567, 0              , 0       },
    {OP(1, 1, 0), 0x23016745, 0, 0x01234567, PSR_MASK       , PSR_MASK},
    {OP(1, 1, 0), 0x89ABCDEF, 0, 0xAB89EFCD, 0              , 0       },
    {OP(1, 1, 0), 0x89ABCDEF, 0, 0xAB89EFCD, PSR_MASK       , PSR_MASK},
    {OP(1, 1, 0), 0xAB89EFCD, 0, 0x89ABCDEF, 0              , 0       },
    {OP(1, 1, 0), 0xAB89EFCD, 0, 0x89ABCDEF, PSR_MASK       , PSR_MASK},

    // 2 is invalid

    // REVSH r0 r1
    {OP(3, 1, 0), 0x01234567, 0, 0x00006745, 0              , 0       }, // 16
    {OP(3, 1, 0), 0x01234567, 0, 0x00006745, PSR_MASK       , PSR_MASK},
    {OP(3, 1, 0), 0x23016745, 0, 0x00004567, 0              , 0       },
    {OP(3, 1, 0), 0x23016745, 0, 0x00004567, PSR_MASK       , PSR_MASK},
    {OP(3, 1, 0), 0x01236789, 0, 0xFFFF8967, 0              , 0       },
    {OP(3, 1, 0), 0x01236789, 0, 0xFFFF8967, PSR_MASK       , PSR_MASK},
    {OP(3, 1, 0), 0x23018967, 0, 0x00006789, 0              , 0       },
    {OP(3, 1, 0), 0x23018967, 0, 0x00006789, PSR_MASK       , PSR_MASK},
    {OP(3, 1, 0), 0x89AB0123, 0, 0x00002301, 0              , 0       },
    {OP(3, 1, 0), 0x89AB0123, 0, 0x00002301, PSR_MASK       , PSR_MASK},
    {OP(3, 1, 0), 0x89ABCDEF, 0, 0xFFFFEFCD, 0              , 0       },
    {OP(3, 1, 0), 0x89ABCDEF, 0, 0xFFFFEFCD, PSR_MASK       , PSR_MASK},
};

#undef OP

static const int num_reverse_tests = sizeof(reverse_tests) / sizeof(reverse_tests[0]);


#endif

#if __ARM_ARCH >= 6

#if defined(PICO_BUILD)
#include <pico.h>
#define in_ram(fn) __not_in_flash_func(fn)
#elif defined(TARGET_32BLIT_HW)
// best we can do, but we also need to move the test buffer
#define in_ram(fn) __attribute__((section(".data.f"))) fn
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

uint32_t get_cpsr_arm() {
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
uint32_t get_cpsr_arm() {
    uint32_t ret;
    asm volatile(
        "mrs %0, cpsr"
        : "=r"(ret)
    );

    return ret;
}

#endif

void invalidate_icache() {
#ifdef __ARM_ARCH_7EM__
    // specifically cortex-m7
    volatile uint32_t *scb_iciallu = (volatile uint32_t *)(0xE000E000 + 0x0D00 + 0x250);

    asm volatile ("dsb 0xF":::"memory");
    asm volatile ("isb 0xF":::"memory");

    *scb_iciallu = 0;

    asm volatile ("dsb 0xF":::"memory");
    asm volatile ("isb 0xF":::"memory");
#endif
}

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
        invalidate_icache();

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

        invalidate_icache();

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

            invalidate_icache();

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
        invalidate_icache();

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

        invalidate_icache();

        out = func(test->psr_in | psr_save, test->m_in, test->n_in, (intptr_t)get_cpsr_arm);
        out &= PSR_MASK;

        if(out != test->psr_out) {
            res = false;
            fail_cb(i, out, test->psr_out);
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
    invalidate_icache();

    uint32_t out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);

    uint32_t expected = (uintptr_t)code_buf + 4;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // now with an offset
    i++;
    code_buf[0] = OP(0, 0, 1); // r0 pc 4

    invalidate_icache();

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = (uintptr_t)code_buf + 8;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // big offset
    i++;
    code_buf[0] = OP(0, 0, 0xFF); // r0 pc 1020

    invalidate_icache();

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

    invalidate_icache();

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = spIn;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // now with an offset
    i++;
    code_buf[0] = OP(1, 0, 1); // r0 sp 4

    invalidate_icache();

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = spIn + 4;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    // big offset
    i++;
    code_buf[0] = OP(1, 0, 0xFF); // r0 sp 1020

    invalidate_icache();

    out = func(0xBAD, 0x1BAD, 0x2BAD, 0x3BAD);
    expected = spIn + 1020;

    if(out != expected) {
        res = false;
        fail_cb(i, out, expected);
    }

    #undef OP

    return res;
}

bool run_thumb2_test_list(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo32 *tests, int num_tests, const char *label, int dest, bool flags_for_val) {

    bool res = true;

    group_cb(label);

    uint32_t psr_save = get_cpsr_arm() & ~PSR_MASK;

    int set_cpsr_off = (uintptr_t)set_cpsr - ((uintptr_t)code_buf + 6);

    const int s_bit = 1 << 20;

    for(int i = 0; i < num_tests; i++) {
        const struct TestInfo32 *test = &tests[i];

        bool has_dest = (test->opcode >> 8 & 0xF) != 0xF;
    
        // value test
        uint16_t *ptr = code_buf;
        uint16_t *op_ptr;

        if(flags_for_val) {
            *ptr++ = 0xB500; // push lr
            *ptr++ = 0xF000 | ((set_cpsr_off >> 12) & 0x7FF); // bl set_cpsr
            *ptr++ = 0xF800 | ((set_cpsr_off >> 1) & 0x7FF); // bl set_cpsr
        }

        op_ptr = ptr;
        *ptr++ = test->opcode >> 16;
        *ptr++ = test->opcode;

        if(dest != 0) // assume < 8
            *ptr++ = dest << 3; // mov r0 rN
        
        if(flags_for_val)
            *ptr++ = 0xBD00; // pop pc
        else
            *ptr++ = 0x4770; // BX LR

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);
        invalidate_icache();

        uint32_t in0 = flags_for_val ? (test->psr_in | psr_save) : 0xBAD;

        uint32_t out = func(in0, test->m_in, test->n_in, 0x3BAD);

        if(out != test->d_out) {
            res = false;
            fail_cb(i, out, test->d_out);
        }

        if(has_dest) {
            // check that the opcode behaves the same without setting flags
            // this would be invalid for the ones that don't have a dest (TST, TEQ, CMN, CMP)
            *op_ptr = (test->opcode & ~s_bit) >> 16;
            invalidate_icache();

            out = func(in0, test->m_in, test->n_in, 0x3BAD);

            if(out != test->d_out) {
                res = false;
                fail_cb(i, out, test->d_out);
            }
        }

        // flags test
        // this relies on the PSR helpers not affecting anything other than R0
        code_buf[0] = 0xB500; // push lr
        code_buf[1] = 0xF000 | ((set_cpsr_off >> 12) & 0x7FF); // bl set_cpsr
        code_buf[2] = 0xF800 | ((set_cpsr_off >> 1) & 0x7FF); // bl set_cpsr

        code_buf[3] = test->opcode >> 16;
        code_buf[4] = test->opcode;

        code_buf[5] = 0xBC01; // pop r0
        code_buf[6] = 0x4686; // mov lr r0
        code_buf[7] = 0x4718; // bx r3

        invalidate_icache();

        out = func(test->psr_in | psr_save, test->m_in, test->n_in, (intptr_t)get_cpsr_arm);
        out &= PSR_MASK;

        if(out != test->psr_out) {
            res = false;
            fail_cb(i, out, test->psr_out);
        }

        if(has_dest) {
            // make sure flags are unmodified if not setting them
            code_buf[3] = (test->opcode & ~s_bit) >> 16;
            invalidate_icache();
            out = func(test->psr_in | psr_save, test->m_in, test->n_in, (intptr_t)get_cpsr_arm);
            out &= PSR_MASK;

            if(out != test->psr_in) {
                res = false;
                fail_cb(i, out, test->psr_in);
            }

            // restore opcode
            code_buf[3] = test->opcode >> 16;
        }

        // single flag tests
        // (to try and break my recompiler)
        for(int j = 0; j < 4; j++) {
            // Z C N V
            static const uint32_t flags[] = {FLAG_Z, FLAG_C, FLAG_N, FLAG_V};

            code_buf[5] = 0xD001 | (j << 9); // Bcc +4
            // flag not set
            code_buf[6] = 0x1A00; // sub r0 r0 r0
            code_buf[7] = 0xE001; // B + 4
            // flag set
            code_buf[8] = 0x1A00; // sub r0 r0 r0
            code_buf[9] = 0x3001; // add r0 1

            code_buf[10] = 0xBD00; // pop pc

            invalidate_icache();

            out = func(test->psr_in | psr_save, test->m_in, test->n_in, 0x3BAD);

            if(out != !!(test->psr_out & flags[j])) {
                res = false;
                fail_cb(i, out ? flags[j] : 0, test->psr_out & flags[j]);
            }
        }

    }

    return res;
}


bool run_tests(GroupCallback group_cb, FailCallback fail_cb) {
    bool ret = true;

    ret = run_test_list(group_cb, fail_cb, shift_imm_tests, num_shift_imm_tests, "sh.imm", 0, false) && ret;
    ret = run_test_list(group_cb, fail_cb, add_sub_tests, num_add_sub_tests, "addsub", 0, false) && ret;
    ret = run_test_list(group_cb, fail_cb, mov_cmp_add_sub_imm_tests, num_mov_cmp_add_sub_imm_tests, "dp.imm", 1, false) && ret;
    ret = run_test_list(group_cb, fail_cb, dp_tests, num_dp_tests, "dp", 1, true) && ret;
    ret = run_hi_reg_tests(group_cb, fail_cb, hi_reg_tests, num_hi_reg_tests, "hireg") && ret;
    ret = run_single_load_store_tests(group_cb, fail_cb) && ret;
    ret = run_load_addr_tests(group_cb, fail_cb, "ldaddr") && ret;
    ret = run_test_list(group_cb, fail_cb, sp_offset_tests, num_sp_offset_tests, "spoff", 13, false) && ret;
    ret = run_push_pop_tests(group_cb, fail_cb, "pushpop") && ret;
    ret = run_ldm_stm_tests(group_cb, fail_cb, "ldmstm") && ret;

#if __ARM_ARCH >= 6
    ret = run_test_list(group_cb, fail_cb, extend_tests, num_extend_tests, "*xt*", 0,  false);
    ret = run_test_list(group_cb, fail_cb, reverse_tests, num_reverse_tests, "rev", 0,  false);
#endif

#if __ARM_ARCH >= 7
    ret = run_ldm_stm_thumb2_tests(group_cb, fail_cb, "ldmstm.t2") && ret;
    ret = run_thumb2_data_processing_tests(group_cb, fail_cb) && ret;
#endif

    return ret;
}