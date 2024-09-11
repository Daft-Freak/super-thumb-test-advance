#include "thumb2_data_processing_tests.h"

#if __ARM_ARCH >= 7

// data processing, modified immediate
// for ops that don't set carry and use a rotated immediate, we get the carry from the rotate
#define OP(op, s, rn, rd, imm12) (0xF0000000 | (imm12 & 0x800) << 15 | op << 21 | s << 20 | rn << 16 | (imm12 & 0x700) << 4 | rd << 8 | (imm12 & 0xFF))

// some imm values:
// 0x0xx -> 0x000000xx
// 0x3xx -> 0xxxxxxxxx (repeated 4 times)
// 0x400 -> 0x80000000 (0x80 ROR 8)
// 0x4FE -> 0x7F000000 (0xFE ROR 9)

static const struct TestInfo32 dp_mod_imm_tests[] = {
    // AND r0 r2 #imm
    {OP(0x0, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 0
    {OP(0x0, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x0, 1,  2,  0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x00000000, 0              , FLAG_Z                  },
    {OP(0x0, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000000, 0x00000000, FLAG_C         , FLAG_Z                  },
    {OP(0x0, 1,  2,  0, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0x7F000000, 0              , 0                       },
    {OP(0x0, 1,  2,  0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x7F000000, 0              , 0                       },
    {OP(0x0, 1,  2,  0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x0, 1,  2,  0, 0x000), NO_SRC1, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2,  0, 0x400), NO_SRC1, 0x00000000, 0x00000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2,  0, 0x400), NO_SRC1, 0x80000000, 0x80000000, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2,  0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2,  0, 0x3FF), NO_SRC1, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2,  0, 0x3AA), NO_SRC1, 0x55555555, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2,  0, 0x355), NO_SRC1, 0xAAAAAAAA, 0x00000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2,  0, 0x3AA), NO_SRC1, 0xFFFFFFFF, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 1,  2,  0, 0x3FF), NO_SRC1, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2,  0, 0x355), NO_SRC1, 0xFFFFFFFF, 0x55555555, 0              , 0                       },
    {OP(0x0, 1,  2,  0, 0x3FF), NO_SRC1, 0x55555555, 0x55555555, FLAG_C         , FLAG_C                  },

    // TST r2 #imm
    {OP(0x0, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0x00000BAD, 0              , FLAG_Z                  }, // 18
    {OP(0x0, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0x00000BAD, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x0, 1,  2, 15, 0x000), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, 0              , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_C         , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, 0              , 0                       },
    {OP(0x0, 1,  2, 15, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, 0              , 0                       },
    {OP(0x0, 1,  2, 15, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_C         , FLAG_C                  },
    {OP(0x0, 1,  2, 15, 0x000), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 0x400), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2, 15, 0x400), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 0x400), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 0x3FF), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 0x3AA), NO_SRC1, 0x55555555, 0x00000BAD, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 0x355), NO_SRC1, 0xAAAAAAAA, 0x00000BAD, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2, 15, 0x3AA), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 1,  2, 15, 0x3FF), NO_SRC1, 0xAAAAAAAA, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 0x355), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, 0              , 0                       },
    {OP(0x0, 1,  2, 15, 0x3FF), NO_SRC1, 0x55555555, 0x00000BAD, FLAG_C         , FLAG_C                  },

    // BIC r0 r2 #imm
    {OP(0x1, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 36
    {OP(0x1, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x1, 1,  2,  0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x1, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000000, 0x00000000, 0              , FLAG_Z                  },
    {OP(0x1, 1,  2,  0, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0x00FFFFFF, FLAG_N         , 0                       },
    {OP(0x1, 1,  2,  0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x80FFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x1, 1,  2,  0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x00000000, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0x1, 1,  2,  0, 0x000), NO_SRC1, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x1, 1,  2,  0, 0x400), NO_SRC1, 0x00000000, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x1, 1,  2,  0, 0x400), NO_SRC1, 0x80000000, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x1, 1,  2,  0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x1, 1,  2,  0, 0x3FF), NO_SRC1, 0x80000000, 0x00000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x1, 1,  2,  0, 0x3AA), NO_SRC1, 0x55555555, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x1, 1,  2,  0, 0x355), NO_SRC1, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x1, 1,  2,  0, 0x3AA), NO_SRC1, 0xFFFFFFFF, 0x55555555, FLAG_C         , FLAG_C                  },
    {OP(0x1, 1,  2,  0, 0x3FF), NO_SRC1, 0xAAAAAAAA, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x1, 1,  2,  0, 0x355), NO_SRC1, 0xFFFFFFFF, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x1, 1,  2,  0, 0x3FF), NO_SRC1, 0x55555555, 0x00000000, FLAG_N         , FLAG_Z                  },

    // ORR r0 r2 #imm
    {OP(0x2, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 54
    {OP(0x2, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x2, 1,  2,  0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x2, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000000, 0x7F000000, FLAG_C         , 0                       },
    {OP(0x2, 1,  2,  0, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x2, 1,  2,  0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C         , FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 0x000), NO_SRC1, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 0x400), NO_SRC1, 0x00000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 0x400), NO_SRC1, 0x80000000, 0x80000000, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 0x400), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 0x3FF), NO_SRC1, 0x80000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 0x3AA), NO_SRC1, 0x55555555, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 0x355), NO_SRC1, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 0x3AA), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_N         , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 0x3FF), NO_SRC1, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 0x355), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 0x3FF), NO_SRC1, 0x55555555, 0xFFFFFFFF, FLAG_C         , FLAG_C | FLAG_N         },

    // MOV r0 #imm
    // not much to test here
    {OP(0x2, 1, 15,  0, 0x000), NO_SRC1,    NO_SRC2, 0x00000000, 0              , FLAG_Z                  }, // 72
    {OP(0x2, 1, 15,  0, 0x000), NO_SRC1,    NO_SRC2, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x2, 1, 15,  0, 0x4FE), NO_SRC1,    NO_SRC2, 0x7F000000, FLAG_C         , 0                       },
    {OP(0x2, 1, 15,  0, 0x3FF), NO_SRC1,    NO_SRC2, 0xFFFFFFFF, FLAG_C         , FLAG_C | FLAG_N         },
    {OP(0x2, 1, 15,  0, 0x400), NO_SRC1,    NO_SRC2, 0x80000000, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x2, 1, 15,  0, 0x3AA), NO_SRC1,    NO_SRC2, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x2, 1, 15,  0, 0x155), NO_SRC1,    NO_SRC2, 0x00550055, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x2, 1, 15,  0, 0x2A5), NO_SRC1,    NO_SRC2, 0xA500A500, FLAG_C         , FLAG_C | FLAG_N         },
    {OP(0x2, 1, 15,  0, 0xF80), NO_SRC1,    NO_SRC2, 0x00000100, FLAG_C         , 0                       },
    {OP(0x2, 1, 15,  0, 0xFFF), NO_SRC1,    NO_SRC2, 0x000001FE, FLAG_Z         , 0                       },

    // ORN r0 r2 #imm
    {OP(0x3, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0xFFFFFFFF, 0              , FLAG_N                  }, // 82
    {OP(0x3, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0xFFFFFFFF, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x3, 1,  2,  0, 0x000), NO_SRC1, 0x7FFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000000, 0x80FFFFFF, FLAG_C         , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x3, 1,  2,  0, 0x000), NO_SRC1, 0x80000000, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 0x400), NO_SRC1, 0x00000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_C                  },
    {OP(0x3, 1,  2,  0, 0x400), NO_SRC1, 0x80000000, 0xFFFFFFFF, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x3, 1,  2,  0, 0x400), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x3, 1,  2,  0, 0x3FF), NO_SRC1, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x3, 1,  2,  0, 0x3AA), NO_SRC1, 0x55555555, 0x55555555, FLAG_Z         ,                         },
    {OP(0x3, 1,  2,  0, 0x355), NO_SRC1, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x3, 1,  2,  0, 0x3AA), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_N         , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 0x3FF), NO_SRC1, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x3, 1,  2,  0, 0x355), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 0x3FF), NO_SRC1, 0x55555555, 0x55555555, FLAG_C         , FLAG_C                  },

    // MVN r0 #imm
    {OP(0x3, 1, 15,  0, 0x000), NO_SRC1,    NO_SRC2, 0xFFFFFFFF, 0              , FLAG_N                  }, // 100
    {OP(0x3, 1, 15,  0, 0x000), NO_SRC1,    NO_SRC2, 0xFFFFFFFF, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x3, 1, 15,  0, 0x4FE), NO_SRC1,    NO_SRC2, 0x80FFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x3, 1, 15,  0, 0x3FF), NO_SRC1,    NO_SRC2, 0x00000000, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0x3, 1, 15,  0, 0x400), NO_SRC1,    NO_SRC2, 0x7FFFFFFF, FLAG_N         , FLAG_C                  },
    {OP(0x3, 1, 15,  0, 0x355), NO_SRC1,    NO_SRC2, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x3, 1, 15,  0, 0x3AA), NO_SRC1,    NO_SRC2, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },

    // EOR r0 r2 #imm
    {OP(0x4, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 107
    {OP(0x4, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x4, 1,  2,  0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x4, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000000, 0x7F000000, FLAG_C         , 0                       },
    {OP(0x4, 1,  2,  0, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0x00FFFFFF, FLAG_N         , 0                       },
    {OP(0x4, 1,  2,  0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x80FFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2,  0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2,  0, 0x000), NO_SRC1, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2,  0, 0x400), NO_SRC1, 0x00000000, 0x80000000, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2,  0, 0x400), NO_SRC1, 0x80000000, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x4, 1,  2,  0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z         , FLAG_C                  },
    {OP(0x4, 1,  2,  0, 0x3FF), NO_SRC1, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_C                  },
    {OP(0x4, 1,  2,  0, 0x3AA), NO_SRC1, 0x55555555, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2,  0, 0x355), NO_SRC1, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2,  0, 0x3AA), NO_SRC1, 0xFFFFFFFF, 0x55555555, 0              , 0                       },
    {OP(0x4, 1,  2,  0, 0x3FF), NO_SRC1, 0xAAAAAAAA, 0x55555555, FLAG_C         , FLAG_C                  },
    {OP(0x4, 1,  2,  0, 0x355), NO_SRC1, 0xFFFFFFFF, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2,  0, 0x3FF), NO_SRC1, 0x55555555, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },

    // TEQ r2 #imm
    {OP(0x4, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0x00000BAD, 0              , FLAG_Z                  }, // 125
    {OP(0x4, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0x00000BAD, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x4, 1,  2, 15, 0x000), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, 0              , 0                       },
    {OP(0x4, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_C         , 0                       },
    {OP(0x4, 1,  2, 15, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_N         , 0                       },
    {OP(0x4, 1,  2, 15, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2, 15, 0x000), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 0x400), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2, 15, 0x400), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x4, 1,  2, 15, 0x400), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_Z         , FLAG_C                  },
    {OP(0x4, 1,  2, 15, 0x3FF), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_C                  },
    {OP(0x4, 1,  2, 15, 0x3AA), NO_SRC1, 0x55555555, 0x00000BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 0x355), NO_SRC1, 0xAAAAAAAA, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2, 15, 0x3AA), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, 0              , 0                       },
    {OP(0x4, 1,  2, 15, 0x3FF), NO_SRC1, 0xAAAAAAAA, 0x00000BAD, FLAG_C         , FLAG_C                  },
    {OP(0x4, 1,  2, 15, 0x355), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 0x3FF), NO_SRC1, 0x55555555, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },

    // ADD r0 r2 imm
    {OP(0x8, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 143
    {OP(0x8, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_Z                  },
    {OP(0x8, 1,  2,  0, 0x001), NO_SRC1, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(0x8, 1,  2,  0, 0x000), NO_SRC1, 0x00000001, 0x00000001, FLAG_C | FLAG_N, 0                       },
    {OP(0x8, 1,  2,  0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z, 0                       },
    {OP(0x8, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000000, 0x7F000000, FLAG_V | FLAG_Z, 0                       },
    {OP(0x8, 1,  2,  0, 0x001), NO_SRC1, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2,  0, 0x4FE), NO_SRC1, 0x01000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2,  0, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0xFEFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2,  0, 0x400), NO_SRC1, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 0x4FE), NO_SRC1, 0x80000000, 0xFF000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2,  0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x7EFFFFFF, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2,  0, 0x000), NO_SRC1, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 0x400), NO_SRC1, 0x00000000, 0x80000000, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 0x400), NO_SRC1, 0x80000000, 0x00000000, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x8, 1,  2,  0, 0x000), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 0x3FF), NO_SRC1, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 0x001), NO_SRC1, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2,  0, 0x3FF), NO_SRC1, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2,  0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2,  0, 0x3FF), NO_SRC1, 0x80000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2,  0, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    
    // CMN r2 #imm
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0x00000BAD, 0              , FLAG_Z                  }, // 166
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0x00000BAD, PSR_VCZN       , FLAG_Z                  },
    {OP(0x8, 1,  2, 15, 0x001), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_Z | FLAG_N, 0                       },
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x00000001, 0x00000BAD, FLAG_C | FLAG_N, 0                       },
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_C | FLAG_Z, 0                       },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_V | FLAG_Z, 0                       },
    {OP(0x8, 1,  2, 15, 0x001), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0x01000000, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2, 15, 0x400), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x400), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x400), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x001), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0x00000001, 0x00000BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2, 15, 0x400), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },

    // ADC/SBC elsewhere

    // SUB r0 r2 #imm
    {OP(0xD, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 189
    {OP(0xD, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2,  0, 0x001), NO_SRC1, 0x00000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2,  0, 0x000), NO_SRC1, 0x00000001, 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2,  0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000000, 0x81000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2,  0, 0x001), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000001, 0x81000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2,  0, 0x4FE), NO_SRC1, 0x7F000000, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2,  0, 0x400), NO_SRC1, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2,  0, 0x4FE), NO_SRC1, 0x80000000, 0x01000000, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xD, 1,  2,  0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2,  0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x80FFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2,  0, 0x000), NO_SRC1, 0x80000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2,  0, 0x400), NO_SRC1, 0x00000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2,  0, 0x400), NO_SRC1, 0x80000000, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2,  0, 0x000), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2,  0, 0x3FF), NO_SRC1, 0x00000000, 0x00000001, FLAG_V | FLAG_C, 0                       },
    {OP(0xD, 1,  2,  0, 0x001), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2,  0, 0x3FF), NO_SRC1, 0x00000001, 0x00000002, FLAG_V | FLAG_N, 0                       },
    {OP(0xD, 1,  2,  0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2,  0, 0x3FF), NO_SRC1, 0x80000000, 0x80000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2,  0, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // CMP r2 #imm
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0x00000BAD, 0              , FLAG_C | FLAG_Z         }, // 212
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0x00000BAD, PSR_VCZN       , FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2, 15, 0x001), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x00000001, 0x00000BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 0x001), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000001, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0x7F000000, 0x00000BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2, 15, 0x400), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x400), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x400), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0x00000000, 0x00000BAD, FLAG_V | FLAG_C, 0                       },
    {OP(0xD, 1,  2, 15, 0x001), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0x00000001, 0x00000BAD, FLAG_V | FLAG_N, 0                       },
    {OP(0xD, 1,  2, 15, 0x400), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0x80000000, 0x00000BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0x00000BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // RSB r0 r2 imm
    {OP(0xE, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 235
    {OP(0xE, 1,  2,  0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_C | FLAG_Z         },
    {OP(0xE, 1,  2,  0, 0x000), NO_SRC1, 0x00000001, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xE, 1,  2,  0, 0x001), NO_SRC1, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xE, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000000, 0x7F000000, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xE, 1,  2,  0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x80000001, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xE, 1,  2,  0, 0x4FE), NO_SRC1, 0x00000001, 0x7EFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xE, 1,  2,  0, 0x001), NO_SRC1, 0x7FFFFFFF, 0x80000002, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xE, 1,  2,  0, 0x4FE), NO_SRC1, 0x7F000000, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xE, 1,  2,  0, 0x4FE), NO_SRC1, 0x80000000, 0xFF000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xE, 1,  2,  0, 0x400), NO_SRC1, 0x7FFFFFFF, 0x00000001, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xE, 1,  2,  0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x7F000001, FLAG_C | FLAG_Z, 0                       },
    {OP(0xE, 1,  2,  0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 1,  2,  0, 0x400), NO_SRC1, 0x00000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 1,  2,  0, 0x000), NO_SRC1, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xE, 1,  2,  0, 0x400), NO_SRC1, 0x80000000, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xE, 1,  2,  0, 0x3FF), NO_SRC1, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 1,  2,  0, 0x000), NO_SRC1, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_C, 0                       },
    {OP(0xE, 1,  2,  0, 0x3FF), NO_SRC1, 0x00000001, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 1,  2,  0, 0x001), NO_SRC1, 0xFFFFFFFF, 0x00000002, FLAG_V | FLAG_N, 0                       },
    {OP(0xE, 1,  2,  0, 0x3FF), NO_SRC1, 0x80000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xE, 1,  2,  0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x80000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xE, 1,  2,  0, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
};

static const int num_dp_mod_imm_tests = sizeof(dp_mod_imm_tests) / sizeof(dp_mod_imm_tests[0]);

static const struct TestInfo32 dp_mod_imm_c_tests[] = {
    // ADC r0 r2 #imm
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 0
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0x00000000, 0x00000001, PSR_VCZN       , 0                       },
    {OP(0xA, 1, 2, 0, 0x001), NO_SRC1, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0x00000001, 0x00000001, FLAG_N         , 0                       },
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0x00000000, 0x7F000000, FLAG_V | FLAG_Z, 0                       },
    {OP(0xA, 1, 2, 0, 0x001), NO_SRC1, 0x7FFFFFFF, 0x80000000, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0x01000000, 0x80000000, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0xFEFFFFFF, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xA, 1, 2, 0, 0x400), NO_SRC1, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0x80000000, 0xFF000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x7EFFFFFF, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 0x400), NO_SRC1, 0x00000000, 0x80000000, FLAG_V         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 0x400), NO_SRC1, 0x80000000, 0x00000000, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0x00000000, 0xFFFFFFFF, FLAG_V         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 0x001), NO_SRC1, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xA, 1, 2, 0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0x80000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },

    // now with C set
    {OP(0xA, 1, 2, 0, 0x001), NO_SRC1, 0x00000000, 0x00000002, FLAG_C | FLAG_Z | FLAG_N, 0              }, // 23
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0x00000001, 0x00000002, FLAG_C | FLAG_N         , 0              },
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0x00FFFFFF, 0x80000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 0x001), NO_SRC1, 0x7FFFFFFF, 0x80000001, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0x01000000, 0x80000001, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0xFF000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 0x400), NO_SRC1, 0x7FFFFFFF, 0x00000000, FLAG_C | FLAG_Z         , FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0x80FFFFFF, 0x00000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xA, 1, 2, 0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x7F000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C         },
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0x80000000, 0x80000001, FLAG_C | FLAG_Z         , FLAG_N         },
    {OP(0xA, 1, 2, 0, 0x400), NO_SRC1, 0x00000000, 0x80000001, FLAG_V | FLAG_C         , FLAG_N         },
    {OP(0xA, 1, 2, 0, 0x400), NO_SRC1, 0x80000000, 0x00000001, FLAG_C | FLAG_N         , FLAG_V | FLAG_C},
    {OP(0xA, 1, 2, 0, 0x000), NO_SRC1, 0xFFFFFFFF, 0x00000000, FLAG_C | FLAG_Z         , FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0x00000000, 0x00000000, FLAG_V | FLAG_C         , FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 0x001), NO_SRC1, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_C | FLAG_N, FLAG_C         },
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0x00000001, 0x00000001, FLAG_V | FLAG_C | FLAG_N, FLAG_C         },
    {OP(0xA, 1, 2, 0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z | FLAG_N, FLAG_C | FLAG_N},
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0x80000000, 0x80000000, FLAG_C | FLAG_Z | FLAG_N, FLAG_C | FLAG_N},
    {OP(0xA, 1, 2, 0, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},

    // SBC r0 r2 #imm
    // start with C set
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0x00000000, 0xFFFFFFFF, 0                       , FLAG_N         }, // 44
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_VCZN                , FLAG_C | FLAG_Z},
    {OP(0xB, 1, 2, 0, 0x001), NO_SRC1, 0x00000000, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_N         },
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0x00000001, 0x00000001, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0x00000000, 0x81000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_N         },
    {OP(0xB, 1, 2, 0, 0x001), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0x00000001, 0x81000001, FLAG_C | FLAG_Z         , FLAG_N         },
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0x7F000000, 0x00000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C | FLAG_Z},
    {OP(0xB, 1, 2, 0, 0x400), NO_SRC1, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0x80000000, 0x01000000, FLAG_C | FLAG_Z | FLAG_N, FLAG_V | FLAG_C},
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0x80FFFFFF, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0x80000000, 0x80000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},
    {OP(0xB, 1, 2, 0, 0x400), NO_SRC1, 0x00000000, 0x80000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xB, 1, 2, 0, 0x400), NO_SRC1, 0x80000000, 0x00000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C | FLAG_Z},
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0x00000000, 0x00000001, FLAG_V | FLAG_C | FLAG_C, 0              },
    {OP(0xB, 1, 2, 0, 0x001), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0x00000001, 0x00000002, FLAG_V | FLAG_C | FLAG_N, 0              },
    {OP(0xB, 1, 2, 0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0x80000000, 0x80000001, FLAG_C | FLAG_Z         , FLAG_N         },
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C | FLAG_Z},

    // now with C clear
    {OP(0xB, 1, 2, 0, 0x001), NO_SRC1, 0x00000000, 0xFFFFFFFE,          FLAG_Z, FLAG_N                  }, // 67
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0x00000001, 0x00000000, FLAG_Z | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0x00000000, 0x80FFFFFF, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xB, 1, 2, 0, 0x001), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0x00000001, 0x81000000, FLAG_Z         , FLAG_N                  },
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0x7F000000, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_N                  },
    {OP(0xB, 1, 2, 0, 0x400), NO_SRC1, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0x7F000001, 0x00000000, FLAG_Z | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0xB, 1, 2, 0, 0x4FE), NO_SRC1, 0xFF000000, 0x7FFFFFFF, FLAG_V | FLAG_Z, FLAG_V | FLAG_C         },
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0x80000000, 0x7FFFFFFF, FLAG_V | FLAG_Z, FLAG_V | FLAG_C         },
    {OP(0xB, 1, 2, 0, 0x400), NO_SRC1, 0x00000000, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0xB, 1, 2, 0, 0x400), NO_SRC1, 0x80000000, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_N                  },
    {OP(0xB, 1, 2, 0, 0x000), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0x00000000, 0x00000000, FLAG_V         , FLAG_Z                  },
    {OP(0xB, 1, 2, 0, 0x001), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0x00000001, 0x00000001, FLAG_V | FLAG_N, 0                       },
    {OP(0xB, 1, 2, 0, 0x400), NO_SRC1, 0xFFFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xB, 1, 2, 0, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_N                  },
};

#undef OP

static const int num_dp_mod_imm_c_tests = sizeof(dp_mod_imm_c_tests) / sizeof(dp_mod_imm_c_tests[0]);

bool run_thumb2_data_processing_tests(GroupCallback group_cb, FailCallback fail_cb) {
    bool ret = true;

    ret = run_thumb2_test_list(group_cb, fail_cb, dp_mod_imm_tests, num_dp_mod_imm_tests, "dp.modimm", 0, false) && ret;
    // ADC/SBC in separate group to set C in
    ret = run_thumb2_test_list(group_cb, fail_cb, dp_mod_imm_c_tests, num_dp_mod_imm_c_tests, "dp.c.modimm", 0, true) && ret;

    return ret;
}
#endif