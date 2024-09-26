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
    {OP(0x0, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0xDAB00BAD, 0              , FLAG_Z                  }, // 18
    {OP(0x0, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0xDAB00BAD, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x0, 1,  2, 15, 0x000), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, 0              , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_C         , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x0, 1,  2, 15, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x0, 1,  2, 15, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C         , FLAG_C                  },
    {OP(0x0, 1,  2, 15, 0x000), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 0x400), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2, 15, 0x400), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 0x400), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 0x3FF), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 0x3AA), NO_SRC1, 0x55555555, 0xDAB00BAD, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 0x355), NO_SRC1, 0xAAAAAAAA, 0xDAB00BAD, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2, 15, 0x3AA), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 1,  2, 15, 0x3FF), NO_SRC1, 0xAAAAAAAA, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 0x355), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x0, 1,  2, 15, 0x3FF), NO_SRC1, 0x55555555, 0xDAB00BAD, FLAG_C         , FLAG_C                  },

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
    {OP(0x4, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0xDAB00BAD, 0              , FLAG_Z                  }, // 125
    {OP(0x4, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0xDAB00BAD, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x4, 1,  2, 15, 0x000), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x4, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_C         , 0                       },
    {OP(0x4, 1,  2, 15, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_N         , 0                       },
    {OP(0x4, 1,  2, 15, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2, 15, 0x000), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 0x400), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2, 15, 0x400), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x4, 1,  2, 15, 0x400), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , FLAG_C                  },
    {OP(0x4, 1,  2, 15, 0x3FF), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C                  },
    {OP(0x4, 1,  2, 15, 0x3AA), NO_SRC1, 0x55555555, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 0x355), NO_SRC1, 0xAAAAAAAA, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2, 15, 0x3AA), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x4, 1,  2, 15, 0x3FF), NO_SRC1, 0xAAAAAAAA, 0xDAB00BAD, FLAG_C         , FLAG_C                  },
    {OP(0x4, 1,  2, 15, 0x355), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 0x3FF), NO_SRC1, 0x55555555, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },

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
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0xDAB00BAD, 0              , FLAG_Z                  }, // 166
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0xDAB00BAD, PSR_VCZN       , FLAG_Z                  },
    {OP(0x8, 1,  2, 15, 0x001), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_Z | FLAG_N, 0                       },
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x00000001, 0xDAB00BAD, FLAG_C | FLAG_N, 0                       },
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, 0                       },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_Z, 0                       },
    {OP(0x8, 1,  2, 15, 0x001), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0x01000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2, 15, 0x400), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2, 15, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x400), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x400), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x8, 1,  2, 15, 0x000), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 0x001), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0x00000001, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2, 15, 0x400), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2, 15, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },

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
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0xDAB00BAD, 0              , FLAG_C | FLAG_Z         }, // 212
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x00000000, 0xDAB00BAD, PSR_VCZN       , FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2, 15, 0x001), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x00000001, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 0x001), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0x00000001, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0x7F000000, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2, 15, 0x400), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x4FE), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x400), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x400), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2, 15, 0x000), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_C, 0                       },
    {OP(0xD, 1,  2, 15, 0x001), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0x00000001, 0xDAB00BAD, FLAG_V | FLAG_N, 0                       },
    {OP(0xD, 1,  2, 15, 0x400), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0x80000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 0x3FF), NO_SRC1, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

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

// data processing, plain binary immediate
// only flags set here is Q by xSAT
// ... but only to 1

// ADDW/SUBW
#define OP12(op, rn, rd, imm) (0xF2000000 | (imm & 0x800) << 15 | op << 21 | rn << 16 | (imm & 0x700) << 4 | rd << 8 | (imm & 0xFF))
// MOVW/MOVT
#define OP16(op, rd, imm) (0xF2000000 | (imm & 0x800) << 15 | op << 21 | (imm & 0xF700) << 4 | rd << 8 | (imm & 0xFF))
// xSAT/xBFX/BFx
#define OP(op, rn, rd, imm, imm2) (0xF2000000 | (imm & 0x800) << 15 | op << 21 | rn << 16 | (imm & 0x1C) << 10 | rd << 8 | (imm & 0x3) << 6 | imm2)

static const struct TestInfo32 dp_plain_imm_tests[] = {
    // ADDW r0 r2 #imm
    {OP12(0x0, 2, 0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 0
    {OP12(0x0, 2, 0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_MASK, PSR_MASK},
    {OP12(0x0, 2, 0, 0x001), NO_SRC1, 0x00000000, 0x00000001, 0       , 0       },
    {OP12(0x0, 2, 0, 0x500), NO_SRC1, 0x00000001, 0x00000501, PSR_MASK, PSR_MASK},
    {OP12(0x0, 2, 0, 0x800), NO_SRC1, 0x80000000, 0x80000800, 0       , 0       },
    {OP12(0x0, 2, 0, 0xFFF), NO_SRC1, 0xAAA00000, 0xAAA00FFF, PSR_MASK, PSR_MASK},
    {OP12(0x0, 2, 0, 0xFFF), NO_SRC1, 0xFFFFFFFF, 0x00000FFE, 0       , 0       },
    {OP12(0x0, 2, 0, 0x123), NO_SRC1, 0x76543210, 0x76543333, PSR_MASK, PSR_MASK},

    // MOVW r0 #imm
    {OP16(0x2, 0, 0x0000)  , NO_SRC1,    NO_SRC2, 0x00000000, 0       , 0       }, // 8
    {OP16(0x2, 0, 0x0000)  , NO_SRC1,    NO_SRC2, 0x00000000, PSR_MASK, PSR_MASK},
    {OP16(0x2, 0, 0x0001)  , NO_SRC1,    NO_SRC2, 0x00000001, 0       , 0       },
    {OP16(0x2, 0, 0x0500)  , NO_SRC1,    NO_SRC2, 0x00000500, PSR_MASK, PSR_MASK},
    {OP16(0x2, 0, 0x0800)  , NO_SRC1,    NO_SRC2, 0x00000800, 0       , 0       },
    {OP16(0x2, 0, 0xA000)  , NO_SRC1,    NO_SRC2, 0x0000A000, PSR_MASK, PSR_MASK},
    {OP16(0x2, 0, 0xFFFF)  , NO_SRC1,    NO_SRC2, 0x0000FFFF, 0       , 0       },
    {OP16(0x2, 0, 0x0123)  , NO_SRC1,    NO_SRC2, 0x00000123, PSR_MASK, PSR_MASK},

    // SUBW r0 r2 #imm
    {OP12(0x5, 2, 0, 0x000), NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 16
    {OP12(0x5, 2, 0, 0x000), NO_SRC1, 0x00000000, 0x00000000, PSR_MASK, PSR_MASK},
    {OP12(0x5, 2, 0, 0x001), NO_SRC1, 0x00000000, 0xFFFFFFFF, 0       , 0       },
    {OP12(0x5, 2, 0, 0x500), NO_SRC1, 0x00000001, 0xFFFFFB01, PSR_MASK, PSR_MASK},
    {OP12(0x5, 2, 0, 0x800), NO_SRC1, 0x80000000, 0x7FFFF800, 0       , 0       },
    {OP12(0x5, 2, 0, 0xFFF), NO_SRC1, 0xAAA00000, 0xAA9FF001, PSR_MASK, PSR_MASK},
    {OP12(0x5, 2, 0, 0xFFF), NO_SRC1, 0xFFFFFFFF, 0xFFFFF000, 0       , 0       },
    {OP12(0x5, 2, 0, 0x123), NO_SRC1, 0x76543210, 0x765430ED, PSR_MASK, PSR_MASK},

    // MOVT r0 #imm
    {OP16(0x6, 0, 0x0000)  , NO_SRC1,    NO_SRC2, 0x00000BAD, 0       , 0       }, // 24
    {OP16(0x6, 0, 0x0000)  , NO_SRC1,    NO_SRC2, 0x00000BAD, PSR_MASK, PSR_MASK},
    {OP16(0x6, 0, 0x0001)  , NO_SRC1,    NO_SRC2, 0x00010BAD, 0       , 0       },
    {OP16(0x6, 0, 0x0500)  , NO_SRC1,    NO_SRC2, 0x05000BAD, PSR_MASK, PSR_MASK},
    {OP16(0x6, 0, 0x0800)  , NO_SRC1,    NO_SRC2, 0x08000BAD, 0       , 0       },
    {OP16(0x6, 0, 0xA000)  , NO_SRC1,    NO_SRC2, 0xA0000BAD, PSR_MASK, PSR_MASK},
    {OP16(0x6, 0, 0xFFFF)  , NO_SRC1,    NO_SRC2, 0xFFFF0BAD, 0       , 0       },
    {OP16(0x6, 0, 0x0123)  , NO_SRC1,    NO_SRC2, 0x01230BAD, PSR_MASK, PSR_MASK},

    // SSAT r0 #imm r2
    {OP(0x8,  2, 0,  0,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 32
    {OP(0x8,  2, 0,  0,  0) , NO_SRC1, 0x00000000, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0x8,  2, 0,  0,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0,  0) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0,  0) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0,  0) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x8,  2, 0,  0,  1) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0x8,  2, 0,  0,  1) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0x8,  2, 0,  0,  1) , NO_SRC1, 0x7FFFFFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0,  1) , NO_SRC1, 0x80000000, 0xFFFFFFFE, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0,  1) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x8,  2, 0,  0,  3) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0x8,  2, 0,  0,  3) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0x8,  2, 0,  0,  3) , NO_SRC1, 0x7FFFFFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0,  3) , NO_SRC1, 0x80000000, 0xFFFFFFF8, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0,  3) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x8,  2, 0,  0,  7) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0x8,  2, 0,  0,  7) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0x8,  2, 0,  0,  7) , NO_SRC1, 0x7FFFFFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0,  7) , NO_SRC1, 0x80000000, 0xFFFFFF80, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0,  7) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x8,  2, 0,  0, 15) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0x8,  2, 0,  0, 15) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0x8,  2, 0,  0, 15) , NO_SRC1, 0x7FFFFFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0, 15) , NO_SRC1, 0x80000000, 0xFFFF8000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0, 15) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x8,  2, 0,  0, 30) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0x8,  2, 0,  0, 30) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0x8,  2, 0,  0, 30) , NO_SRC1, 0x7FFFFFFF, 0x3FFFFFFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0, 30) , NO_SRC1, 0x80000000, 0xC0000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  0, 30) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x8,  2, 0,  0, 31) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0x8,  2, 0,  0, 31) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0x8,  2, 0,  0, 31) , NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, 0       , 0       },
    {OP(0x8,  2, 0,  0, 31) , NO_SRC1, 0x80000000, 0x80000000, 0       , 0       },
    {OP(0x8,  2, 0,  0, 31) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    // ... LSL 3
    {OP(0x8,  2, 0,  3,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 68
    {OP(0x8,  2, 0,  3,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  0) , NO_SRC1, 0x0FFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  0) , NO_SRC1, 0x10000000, 0xFFFFFFFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  0) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  1) , NO_SRC1, 0x00000001, 0x00000001, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  1) , NO_SRC1, 0x0FFFFFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  1) , NO_SRC1, 0x10000000, 0xFFFFFFFE, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  1) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFE, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  3) , NO_SRC1, 0x00000001, 0x00000007, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  3) , NO_SRC1, 0x0FFFFFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  3) , NO_SRC1, 0x10000000, 0xFFFFFFF8, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  3) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFF8, 0       , 0       },
    {OP(0x8,  2, 0,  3,  7) , NO_SRC1, 0x00000001, 0x00000008, 0       , 0       },
    {OP(0x8,  2, 0,  3,  7) , NO_SRC1, 0x0FFFFFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  7) , NO_SRC1, 0x10000000, 0xFFFFFF80, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3,  7) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFF8, 0       , 0       },
    {OP(0x8,  2, 0,  3, 15) , NO_SRC1, 0x00000001, 0x00000008, 0       , 0       },
    {OP(0x8,  2, 0,  3, 15) , NO_SRC1, 0x0FFFFFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3, 15) , NO_SRC1, 0x10000000, 0xFFFF8000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3, 15) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFF8, 0       , 0       },
    {OP(0x8,  2, 0,  3, 30) , NO_SRC1, 0x00000001, 0x00000008, 0       , 0       },
    {OP(0x8,  2, 0,  3, 30) , NO_SRC1, 0x0FFFFFFF, 0x3FFFFFFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3, 30) , NO_SRC1, 0x10000000, 0xC0000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0,  3, 30) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFF8, 0       , 0       },
    {OP(0x8,  2, 0,  3, 31) , NO_SRC1, 0x00000001, 0x00000008, 0       , 0       },
    {OP(0x8,  2, 0,  3, 31) , NO_SRC1, 0x0FFFFFFF, 0x7FFFFFF8, 0       , 0       },
    {OP(0x8,  2, 0,  3, 31) , NO_SRC1, 0x10000000, 0x80000000, 0       , 0       },
    {OP(0x8,  2, 0,  3, 31) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFF8, 0       , 0       },
    // ... LSL 15
    {OP(0x8,  2, 0, 15,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 97
    {OP(0x8,  2, 0, 15,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  0) , NO_SRC1, 0x0000FFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  0) , NO_SRC1, 0x00010000, 0xFFFFFFFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  0) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  1) , NO_SRC1, 0x00000001, 0x00000001, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  1) , NO_SRC1, 0x0000FFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  1) , NO_SRC1, 0x00010000, 0xFFFFFFFE, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  1) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFE, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  3) , NO_SRC1, 0x00000001, 0x00000007, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  3) , NO_SRC1, 0x0000FFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  3) , NO_SRC1, 0x00010000, 0xFFFFFFF8, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  3) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFF8, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  7) , NO_SRC1, 0x00000001, 0x0000007F, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  7) , NO_SRC1, 0x0000FFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  7) , NO_SRC1, 0x00010000, 0xFFFFFF80, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15,  7) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFF80, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15, 15) , NO_SRC1, 0x00000001, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15, 15) , NO_SRC1, 0x0000FFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15, 15) , NO_SRC1, 0x00010000, 0xFFFF8000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15, 15) , NO_SRC1, 0xFFFFFFFF, 0xFFFF8000, 0       , 0       },
    {OP(0x8,  2, 0, 15, 30) , NO_SRC1, 0x00000001, 0x00008000, 0       , 0       },
    {OP(0x8,  2, 0, 15, 30) , NO_SRC1, 0x0000FFFF, 0x3FFFFFFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15, 30) , NO_SRC1, 0x00010000, 0xC0000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 15, 30) , NO_SRC1, 0xFFFFFFFF, 0xFFFF8000, 0       , 0       },
    {OP(0x8,  2, 0, 15, 31) , NO_SRC1, 0x00000001, 0x00008000, 0       , 0       },
    {OP(0x8,  2, 0, 15, 31) , NO_SRC1, 0x0000FFFF, 0x7FFF8000, 0       , 0       },
    {OP(0x8,  2, 0, 15, 31) , NO_SRC1, 0x00010000, 0x80000000, 0       , 0       },
    {OP(0x8,  2, 0, 15, 31) , NO_SRC1, 0xFFFFFFFF, 0xFFFF8000, 0       , 0       },
    // ... LSL 31
    {OP(0x8,  2, 0, 31,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 126
    {OP(0x8,  2, 0, 31,  0) , NO_SRC1, 0x00000001, 0xFFFFFFFF, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 31,  1) , NO_SRC1, 0x00000001, 0xFFFFFFFE, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 31,  3) , NO_SRC1, 0x00000001, 0xFFFFFFF8, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 31,  7) , NO_SRC1, 0x00000001, 0xFFFFFF80, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 31, 15) , NO_SRC1, 0x00000001, 0xFFFF8000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 31, 30) , NO_SRC1, 0x00000001, 0xC0000000, 0       , FLAG_Q  },
    {OP(0x8,  2, 0, 31, 31) , NO_SRC1, 0x00000001, 0x80000000, 0       , 0       },
    // ... ASR 0 is SSAT16, part of the DSP extension
    // ... ASR 3
    {OP(0x9,  2, 0,  3,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 134
    {OP(0x9,  2, 0,  3,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , 0       },
    {OP(0x9,  2, 0,  3,  0) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3,  0) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3,  0) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0,  3,  1) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0,  3,  1) , NO_SRC1, 0x7FFFFFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3,  1) , NO_SRC1, 0x80000000, 0xFFFFFFFE, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3,  1) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0,  3,  3) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0,  3,  3) , NO_SRC1, 0x7FFFFFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3,  3) , NO_SRC1, 0x80000000, 0xFFFFFFF8, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3,  3) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0,  3,  7) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0,  3,  7) , NO_SRC1, 0x7FFFFFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3,  7) , NO_SRC1, 0x80000000, 0xFFFFFF80, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3,  7) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0,  3, 15) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0,  3, 15) , NO_SRC1, 0x7FFFFFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3, 15) , NO_SRC1, 0x80000000, 0xFFFF8000, 0       , FLAG_Q  },
    {OP(0x9,  2, 0,  3, 15) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0,  3, 30) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0,  3, 30) , NO_SRC1, 0x7FFFFFFF, 0x0FFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0,  3, 30) , NO_SRC1, 0x80000000, 0xF0000000, 0       , 0       },
    {OP(0x9,  2, 0,  3, 30) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0,  3, 31) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0,  3, 31) , NO_SRC1, 0x7FFFFFFF, 0x0FFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0,  3, 31) , NO_SRC1, 0x80000000, 0xF0000000, 0       , 0       },
    {OP(0x9,  2, 0,  3, 31) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    // ... ASR 15
    {OP(0x9,  2, 0, 15,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 163
    {OP(0x9,  2, 0, 15,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , 0       },
    {OP(0x9,  2, 0, 15,  0) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15,  0) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15,  0) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 15,  1) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0, 15,  1) , NO_SRC1, 0x7FFFFFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15,  1) , NO_SRC1, 0x80000000, 0xFFFFFFFE, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15,  1) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 15,  3) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0, 15,  3) , NO_SRC1, 0x7FFFFFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15,  3) , NO_SRC1, 0x80000000, 0xFFFFFFF8, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15,  3) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 15,  7) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0, 15,  7) , NO_SRC1, 0x7FFFFFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15,  7) , NO_SRC1, 0x80000000, 0xFFFFFF80, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15,  7) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 15, 15) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0, 15, 15) , NO_SRC1, 0x7FFFFFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15, 15) , NO_SRC1, 0x80000000, 0xFFFF8000, 0       , FLAG_Q  },
    {OP(0x9,  2, 0, 15, 15) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 15, 30) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0, 15, 30) , NO_SRC1, 0x7FFFFFFF, 0x0000FFFF, 0       , 0       },
    {OP(0x9,  2, 0, 15, 30) , NO_SRC1, 0x80000000, 0xFFFF0000, 0       , 0       },
    {OP(0x9,  2, 0, 15, 30) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 15, 31) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0x9,  2, 0, 15, 31) , NO_SRC1, 0x7FFFFFFF, 0x0000FFFF, 0       , 0       },
    {OP(0x9,  2, 0, 15, 31) , NO_SRC1, 0x80000000, 0xFFFF0000, 0       , 0       },
    {OP(0x9,  2, 0, 15, 31) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    // ... ASR 31
    {OP(0x9,  2, 0, 31,  0) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       }, // 192
    {OP(0x9,  2, 0, 31,  0) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 31,  0) , NO_SRC1, 0xFFFFFFFF, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 31,  1) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0x9,  2, 0, 31,  1) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 31,  3) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0x9,  2, 0, 31,  3) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 31,  7) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0x9,  2, 0, 31,  7) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 31, 15) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0x9,  2, 0, 31, 15) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 31, 30) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0x9,  2, 0, 31, 30) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , 0       },
    {OP(0x9,  2, 0, 31, 31) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0x9,  2, 0, 31, 31) , NO_SRC1, 0x80000000, 0xFFFFFFFF, 0       , 0       },

    // SBFX r0 r2 #lsb #width
    {OP(0xA,  2, 0,  0,  0) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       }, // 207
    {OP(0xA,  2, 0,  0,  0) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  0,  1) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       },
    {OP(0xA,  2, 0,  0,  1) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  0,  3) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       },
    {OP(0xA,  2, 0,  0,  3) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFF8, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  0,  7) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFEF, 0       , 0       },
    {OP(0xA,  2, 0,  0,  7) , NO_SRC1, 0xFEDCBA98, 0xFFFFFF98, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  0, 15) , NO_SRC1, 0x89ABCDEF, 0xFFFFCDEF, 0       , 0       },
    {OP(0xA,  2, 0,  0, 15) , NO_SRC1, 0xFEDCBA98, 0xFFFFBA98, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  0, 30) , NO_SRC1, 0x89ABCDEF, 0x09ABCDEF, 0       , 0       },
    {OP(0xA,  2, 0,  0, 30) , NO_SRC1, 0xFEDCBA98, 0xFEDCBA98, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  0, 31) , NO_SRC1, 0x89ABCDEF, 0x89ABCDEF, 0       , 0       },
    {OP(0xA,  2, 0,  0, 31) , NO_SRC1, 0xFEDCBA98, 0xFEDCBA98, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  1,  0) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       }, // 221
    {OP(0xA,  2, 0,  1,  0) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  1,  1) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       },
    {OP(0xA,  2, 0,  1,  1) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  1,  3) , NO_SRC1, 0x89ABCDEF, 0x00000007, 0       , 0       },
    {OP(0xA,  2, 0,  1,  3) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFFC, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  1,  7) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFF7, 0       , 0       },
    {OP(0xA,  2, 0,  1,  7) , NO_SRC1, 0xFEDCBA98, 0x0000004C, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  1, 15) , NO_SRC1, 0x89ABCDEF, 0xFFFFE6F7, 0       , 0       },
    {OP(0xA,  2, 0,  1, 15) , NO_SRC1, 0xFEDCBA98, 0x00005D4C, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  1, 29) , NO_SRC1, 0x89ABCDEF, 0x04D5E6F7, 0       , 0       },
    {OP(0xA,  2, 0,  1, 29) , NO_SRC1, 0xFEDCBA98, 0xFF6E5D4C, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  1, 30) , NO_SRC1, 0x89ABCDEF, 0xC4D5E6F7, 0       , 0       },
    {OP(0xA,  2, 0,  1, 30) , NO_SRC1, 0xFEDCBA98, 0xFF6E5D4C, PSR_MASK, PSR_MASK},
    // lsb + width > 32 faults
    {OP(0xA,  2, 0,  3,  0) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       }, // 235
    {OP(0xA,  2, 0,  3,  0) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFFF, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  3,  1) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       },
    {OP(0xA,  2, 0,  3,  1) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFFF, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  3,  3) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFD, 0       , 0       },
    {OP(0xA,  2, 0,  3,  3) , NO_SRC1, 0xFEDCBA98, 0x00000003, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  3,  7) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFBD, 0       , 0       },
    {OP(0xA,  2, 0,  3,  7) , NO_SRC1, 0xFEDCBA98, 0x00000053, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  3, 15) , NO_SRC1, 0x89ABCDEF, 0x000079BD, 0       , 0       },
    {OP(0xA,  2, 0,  3, 15) , NO_SRC1, 0xFEDCBA98, 0xFFFF9753, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  3, 27) , NO_SRC1, 0x89ABCDEF, 0x013579BD, 0       , 0       },
    {OP(0xA,  2, 0,  3, 27) , NO_SRC1, 0xFEDCBA98, 0xFFDB9753, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0,  3, 28) , NO_SRC1, 0x89ABCDEF, 0xF13579BD, 0       , 0       },
    {OP(0xA,  2, 0,  3, 28) , NO_SRC1, 0xFEDCBA98, 0xFFDB9753, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 14,  0) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       }, // 249
    {OP(0xA,  2, 0, 14,  0) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 14,  1) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       },
    {OP(0xA,  2, 0, 14,  1) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFFE, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 14,  3) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       },
    {OP(0xA,  2, 0, 14,  3) , NO_SRC1, 0xFEDCBA98, 0x00000002, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 14,  7) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFAF, 0       , 0       },
    {OP(0xA,  2, 0, 14,  7) , NO_SRC1, 0xFEDCBA98, 0x00000072, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 14, 15) , NO_SRC1, 0x89ABCDEF, 0x000026AF, 0       , 0       },
    {OP(0xA,  2, 0, 14, 15) , NO_SRC1, 0xFEDCBA98, 0xFFFFFB72, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 14, 16) , NO_SRC1, 0x89ABCDEF, 0x000026AF, 0       , 0       },
    {OP(0xA,  2, 0, 14, 16) , NO_SRC1, 0xFEDCBA98, 0xFFFFFB72, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 14, 17) , NO_SRC1, 0x89ABCDEF, 0xFFFE26AF, 0       , 0       },
    {OP(0xA,  2, 0, 14, 17) , NO_SRC1, 0xFEDCBA98, 0xFFFFFB72, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 21,  0) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       }, // 263
    {OP(0xA,  2, 0, 21,  0) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 21,  1) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       },
    {OP(0xA,  2, 0, 21,  1) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFFE, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 21,  3) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFD, 0       , 0       },
    {OP(0xA,  2, 0, 21,  3) , NO_SRC1, 0xFEDCBA98, 0x00000006, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 21,  7) , NO_SRC1, 0x89ABCDEF, 0x0000004D, 0       , 0       },
    {OP(0xA,  2, 0, 21,  7) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFF6, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 21, 10) , NO_SRC1, 0x89ABCDEF, 0xFFFFFC4D, 0       , 0       },
    {OP(0xA,  2, 0, 21, 10) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFF6, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 30,  0) , NO_SRC1, 0x89ABCDEF, 0x00000000, 0       , 0       }, // 273
    {OP(0xA,  2, 0, 30,  0) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFFF, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 30,  1) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFE, 0       , 0       },
    {OP(0xA,  2, 0, 30,  1) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFFF, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 31,  0) , NO_SRC1, 0x89ABCDEF, 0xFFFFFFFF, 0       , 0       },
    {OP(0xA,  2, 0, 31,  0) , NO_SRC1, 0xFEDCBA98, 0xFFFFFFFF, PSR_MASK, PSR_MASK},
    {OP(0xA,  2, 0, 31,  0) , NO_SRC1, 0x76543210, 0x00000000, 0       , 0       },

    // BFI r0 r2 #lsb #width
    // (encoded as lsb msb)
    {OP(0xB,  2, 0,  0,  0) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAD, 0       , 0       }, // 280
    {OP(0xB,  2, 0,  0,  0) , NO_SRC1, 0xFEDCBA98, 0xDAB00BAC, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  0,  1) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAF, 0       , 0       },
    {OP(0xB,  2, 0,  0,  1) , NO_SRC1, 0xFEDCBA98, 0xDAB00BAC, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  0,  3) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAF, 0       , 0       },
    {OP(0xB,  2, 0,  0,  3) , NO_SRC1, 0xFEDCBA98, 0xDAB00BA8, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  0,  7) , NO_SRC1, 0x89ABCDEF, 0xDAB00BEF, 0       , 0       },
    {OP(0xB,  2, 0,  0,  7) , NO_SRC1, 0xFEDCBA98, 0xDAB00B98, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  0, 15) , NO_SRC1, 0x89ABCDEF, 0xDAB0CDEF, 0       , 0       },
    {OP(0xB,  2, 0,  0, 15) , NO_SRC1, 0xFEDCBA98, 0xDAB0BA98, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  0, 30) , NO_SRC1, 0x89ABCDEF, 0x89ABCDEF, 0       , 0       },
    {OP(0xB,  2, 0,  0, 30) , NO_SRC1, 0xFEDCBA98, 0xFEDCBA98, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  0, 31) , NO_SRC1, 0x89ABCDEF, 0x89ABCDEF, 0       , 0       },
    {OP(0xB,  2, 0,  0, 31) , NO_SRC1, 0xFEDCBA98, 0xFEDCBA98, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  1,  1) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAF, 0       , 0       }, // 294
    {OP(0xB,  2, 0,  1,  1) , NO_SRC1, 0xFEDCBA98, 0xDAB00BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  1,  2) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAF, 0       , 0       },
    {OP(0xB,  2, 0,  1,  2) , NO_SRC1, 0xFEDCBA98, 0xDAB00BA9, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  1,  4) , NO_SRC1, 0x89ABCDEF, 0xDAB00BBF, 0       , 0       },
    {OP(0xB,  2, 0,  1,  4) , NO_SRC1, 0xFEDCBA98, 0xDAB00BB1, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  1,  8) , NO_SRC1, 0x89ABCDEF, 0xDAB00BDF, 0       , 0       },
    {OP(0xB,  2, 0,  1,  8) , NO_SRC1, 0xFEDCBA98, 0xDAB00B31, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  1, 16) , NO_SRC1, 0x89ABCDEF, 0xDAB19BDF, 0       , 0       },
    {OP(0xB,  2, 0,  1, 16) , NO_SRC1, 0xFEDCBA98, 0xDAB17531, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  1, 30) , NO_SRC1, 0x89ABCDEF, 0x93579BDF, 0       , 0       },
    {OP(0xB,  2, 0,  1, 30) , NO_SRC1, 0xFEDCBA98, 0xFDB97531, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  1, 31) , NO_SRC1, 0x89ABCDEF, 0x13579BDF, 0       , 0       },
    {OP(0xB,  2, 0,  1, 31) , NO_SRC1, 0xFEDCBA98, 0xFDB97531, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  3,  3) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAD, 0       , 0       }, // 308
    {OP(0xB,  2, 0,  3,  3) , NO_SRC1, 0xFEDCBA98, 0xDAB00BA5, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  3,  4) , NO_SRC1, 0x89ABCDEF, 0xDAB00BBD, 0       , 0       },
    {OP(0xB,  2, 0,  3,  4) , NO_SRC1, 0xFEDCBA98, 0xDAB00BA5, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  3,  6) , NO_SRC1, 0x89ABCDEF, 0xDAB00BFD, 0       , 0       },
    {OP(0xB,  2, 0,  3,  6) , NO_SRC1, 0xFEDCBA98, 0xDAB00BC5, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  3, 10) , NO_SRC1, 0x89ABCDEF, 0xDAB00F7D, 0       , 0       },
    {OP(0xB,  2, 0,  3, 10) , NO_SRC1, 0xFEDCBA98, 0xDAB00CC5, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  3, 18) , NO_SRC1, 0x89ABCDEF, 0xDAB66F7D, 0       , 0       },
    {OP(0xB,  2, 0,  3, 18) , NO_SRC1, 0xFEDCBA98, 0xDAB5D4C5, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  3, 30) , NO_SRC1, 0x89ABCDEF, 0xCD5E6F7D, 0       , 0       },
    {OP(0xB,  2, 0,  3, 30) , NO_SRC1, 0xFEDCBA98, 0xF6E5D4C5, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0,  3, 31) , NO_SRC1, 0x89ABCDEF, 0x4D5E6F7D, 0       , 0       },
    {OP(0xB,  2, 0,  3, 31) , NO_SRC1, 0xFEDCBA98, 0xF6E5D4C5, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 14, 14) , NO_SRC1, 0x89ABCDEF, 0xDAB04BAD, 0       , 0       }, // 322
    {OP(0xB,  2, 0, 14, 14) , NO_SRC1, 0xFEDCBA98, 0xDAB00BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 14, 15) , NO_SRC1, 0x89ABCDEF, 0xDAB0CBAD, 0       , 0       },
    {OP(0xB,  2, 0, 14, 15) , NO_SRC1, 0xFEDCBA98, 0xDAB00BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 14, 17) , NO_SRC1, 0x89ABCDEF, 0xDAB3CBAD, 0       , 0       },
    {OP(0xB,  2, 0, 14, 17) , NO_SRC1, 0xFEDCBA98, 0xDAB20BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 14, 21) , NO_SRC1, 0x89ABCDEF, 0xDABBCBAD, 0       , 0       },
    {OP(0xB,  2, 0, 14, 21) , NO_SRC1, 0xFEDCBA98, 0xDAA60BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 14, 29) , NO_SRC1, 0x89ABCDEF, 0xF37BCBAD, 0       , 0       },
    {OP(0xB,  2, 0, 14, 29) , NO_SRC1, 0xFEDCBA98, 0xEEA60BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 14, 30) , NO_SRC1, 0x89ABCDEF, 0xF37BCBAD, 0       , 0       },
    {OP(0xB,  2, 0, 14, 30) , NO_SRC1, 0xFEDCBA98, 0xAEA60BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 14, 31) , NO_SRC1, 0x89ABCDEF, 0xF37BCBAD, 0       , 0       },
    {OP(0xB,  2, 0, 14, 31) , NO_SRC1, 0xFEDCBA98, 0x2EA60BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 21, 21) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAD, 0       , 0       }, // 336
    {OP(0xB,  2, 0, 21, 21) , NO_SRC1, 0xFEDCBA98, 0xDA900BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 21, 22) , NO_SRC1, 0x89ABCDEF, 0xDAF00BAD, 0       , 0       },
    {OP(0xB,  2, 0, 21, 22) , NO_SRC1, 0xFEDCBA98, 0xDA900BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 21, 24) , NO_SRC1, 0x89ABCDEF, 0xDBF00BAD, 0       , 0       },
    {OP(0xB,  2, 0, 21, 24) , NO_SRC1, 0xFEDCBA98, 0xDB100BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 21, 28) , NO_SRC1, 0x89ABCDEF, 0xDDF00BAD, 0       , 0       },
    {OP(0xB,  2, 0, 21, 28) , NO_SRC1, 0xFEDCBA98, 0xD3100BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 21, 31) , NO_SRC1, 0x89ABCDEF, 0xBDF00BAD, 0       , 0       },
    {OP(0xB,  2, 0, 21, 31) , NO_SRC1, 0xFEDCBA98, 0x53100BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 30, 30) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAD, 0       , 0       }, // 346
    {OP(0xB,  2, 0, 30, 30) , NO_SRC1, 0xFEDCBA98, 0x9AB00BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 30, 31) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAD, 0       , 0       },
    {OP(0xB,  2, 0, 30, 31) , NO_SRC1, 0xFEDCBA98, 0x1AB00BAD, PSR_MASK, PSR_MASK},
    {OP(0xB,  2, 0, 31, 31) , NO_SRC1, 0x89ABCDEF, 0xDAB00BAD, 0       , 0       },
    {OP(0xB,  2, 0, 31, 31) , NO_SRC1, 0xFEDCBA98, 0x5AB00BAD, PSR_MASK, PSR_MASK},

    // BFC r0 #lsb #width
    {OP(0xB, 15, 0,  0,  0) , NO_SRC1,    NO_SRC2, 0xDAB00BAC, 0       , 0       }, // 352
    {OP(0xB, 15, 0,  0,  1) , NO_SRC1,    NO_SRC2, 0xDAB00BAC, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0,  0,  3) , NO_SRC1,    NO_SRC2, 0xDAB00BA0, 0       , 0       },
    {OP(0xB, 15, 0,  0,  7) , NO_SRC1,    NO_SRC2, 0xDAB00B00, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0,  0, 15) , NO_SRC1,    NO_SRC2, 0xDAB00000, 0       , 0       },
    {OP(0xB, 15, 0,  0, 30) , NO_SRC1,    NO_SRC2, 0x80000000, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0,  0, 31) , NO_SRC1,    NO_SRC2, 0x00000000, 0       , 0       },
    {OP(0xB, 15, 0,  1,  1) , NO_SRC1,    NO_SRC2, 0xDAB00BAD, PSR_MASK, PSR_MASK}, // 359
    {OP(0xB, 15, 0,  1,  2) , NO_SRC1,    NO_SRC2, 0xDAB00BA9, 0       , 0       },
    {OP(0xB, 15, 0,  1,  4) , NO_SRC1,    NO_SRC2, 0xDAB00BA1, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0,  1,  8) , NO_SRC1,    NO_SRC2, 0xDAB00A01, 0       , 0       },
    {OP(0xB, 15, 0,  1, 16) , NO_SRC1,    NO_SRC2, 0xDAB00001, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0,  1, 30) , NO_SRC1,    NO_SRC2, 0x80000001, 0       , 0       },
    {OP(0xB, 15, 0,  1, 31) , NO_SRC1,    NO_SRC2, 0x00000001, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0,  3,  3) , NO_SRC1,    NO_SRC2, 0xDAB00BA5, 0       , 0       }, // 366
    {OP(0xB, 15, 0,  3,  4) , NO_SRC1,    NO_SRC2, 0xDAB00BA5, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0,  3,  6) , NO_SRC1,    NO_SRC2, 0xDAB00B85, 0       , 0       },
    {OP(0xB, 15, 0,  3, 10) , NO_SRC1,    NO_SRC2, 0xDAB00805, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0,  3, 18) , NO_SRC1,    NO_SRC2, 0xDAB00005, 0       , 0       },
    {OP(0xB, 15, 0,  3, 30) , NO_SRC1,    NO_SRC2, 0x80000005, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0,  3, 31) , NO_SRC1,    NO_SRC2, 0x00000005, 0       , 0       },
    {OP(0xB, 15, 0, 14, 14) , NO_SRC1,    NO_SRC2, 0xDAB00BAD, PSR_MASK, PSR_MASK}, // 373
    {OP(0xB, 15, 0, 14, 15) , NO_SRC1,    NO_SRC2, 0xDAB00BAD, 0       , 0       },
    {OP(0xB, 15, 0, 14, 17) , NO_SRC1,    NO_SRC2, 0xDAB00BAD, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0, 14, 21) , NO_SRC1,    NO_SRC2, 0xDA800BAD, 0       , 0       },
    {OP(0xB, 15, 0, 14, 29) , NO_SRC1,    NO_SRC2, 0xC0000BAD, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0, 14, 30) , NO_SRC1,    NO_SRC2, 0x80000BAD, 0       , 0       },
    {OP(0xB, 15, 0, 14, 31) , NO_SRC1,    NO_SRC2, 0x00000BAD, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0, 21, 21) , NO_SRC1,    NO_SRC2, 0xDA900BAD, 0       , 0       }, // 380
    {OP(0xB, 15, 0, 21, 22) , NO_SRC1,    NO_SRC2, 0xDA900BAD, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0, 21, 24) , NO_SRC1,    NO_SRC2, 0xDA100BAD, 0       , 0       },
    {OP(0xB, 15, 0, 21, 28) , NO_SRC1,    NO_SRC2, 0xC0100BAD, PSR_MASK, PSR_MASK},
    {OP(0xB, 15, 0, 21, 31) , NO_SRC1,    NO_SRC2, 0x00100BAD, 0       , 0       },
    {OP(0xB, 15, 0, 30, 30) , NO_SRC1,    NO_SRC2, 0x9AB00BAD, PSR_MASK, PSR_MASK}, // 385
    {OP(0xB, 15, 0, 30, 31) , NO_SRC1,    NO_SRC2, 0x1AB00BAD, 0       , 0       },
    {OP(0xB, 15, 0, 31, 31) , NO_SRC1,    NO_SRC2, 0x5AB00BAD, PSR_MASK, PSR_MASK},

    // USAT r0 #imm r2
    {OP(0xC,  2, 0,  0,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 388
    {OP(0xC,  2, 0,  0,  0) , NO_SRC1, 0x00000000, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xC,  2, 0,  0,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  0) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  0) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  0) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  1) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0xC,  2, 0,  0,  1) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0xC,  2, 0,  0,  1) , NO_SRC1, 0x7FFFFFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  1) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  1) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  3) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0xC,  2, 0,  0,  3) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0xC,  2, 0,  0,  3) , NO_SRC1, 0x7FFFFFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  3) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  3) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  7) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0xC,  2, 0,  0,  7) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0xC,  2, 0,  0,  7) , NO_SRC1, 0x7FFFFFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  7) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0,  7) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0, 15) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0xC,  2, 0,  0, 15) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0xC,  2, 0,  0, 15) , NO_SRC1, 0x7FFFFFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0, 15) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0, 15) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0, 30) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0xC,  2, 0,  0, 30) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0xC,  2, 0,  0, 30) , NO_SRC1, 0x7FFFFFFF, 0x3FFFFFFF, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0, 30) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0, 30) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0, 31) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       },
    {OP(0xC,  2, 0,  0, 31) , NO_SRC1, 0x00000001, 0x00000001, 0       , 0       },
    {OP(0xC,  2, 0,  0, 31) , NO_SRC1, 0x7FFFFFFF, 0x7FFFFFFF, 0       , 0       },
    {OP(0xC,  2, 0,  0, 31) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  0, 31) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    // ... LSL 3
    {OP(0xC,  2, 0,  3,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 424
    {OP(0xC,  2, 0,  3,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  0) , NO_SRC1, 0x0FFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  0) , NO_SRC1, 0x10000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  0) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  1) , NO_SRC1, 0x00000001, 0x00000001, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  1) , NO_SRC1, 0x0FFFFFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  1) , NO_SRC1, 0x10000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  1) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  3) , NO_SRC1, 0x00000001, 0x00000007, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  3) , NO_SRC1, 0x0FFFFFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  3) , NO_SRC1, 0x10000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  3) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  7) , NO_SRC1, 0x00000001, 0x00000008, 0       , 0       },
    {OP(0xC,  2, 0,  3,  7) , NO_SRC1, 0x0FFFFFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  7) , NO_SRC1, 0x10000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3,  7) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3, 15) , NO_SRC1, 0x00000001, 0x00000008, 0       , 0       },
    {OP(0xC,  2, 0,  3, 15) , NO_SRC1, 0x0FFFFFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3, 15) , NO_SRC1, 0x10000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3, 15) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3, 30) , NO_SRC1, 0x00000001, 0x00000008, 0       , 0       },
    {OP(0xC,  2, 0,  3, 30) , NO_SRC1, 0x0FFFFFFF, 0x3FFFFFFF, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3, 30) , NO_SRC1, 0x10000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3, 30) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3, 31) , NO_SRC1, 0x00000001, 0x00000008, 0       , 0       },
    {OP(0xC,  2, 0,  3, 31) , NO_SRC1, 0x0FFFFFFF, 0x7FFFFFF8, 0       , 0       },
    {OP(0xC,  2, 0,  3, 31) , NO_SRC1, 0x10000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0,  3, 31) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    // ... LSL 15
    {OP(0xC,  2, 0, 15,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 453
    {OP(0xC,  2, 0, 15,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  0) , NO_SRC1, 0x0000FFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  0) , NO_SRC1, 0x00010000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  0) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  1) , NO_SRC1, 0x00000001, 0x00000001, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  1) , NO_SRC1, 0x0000FFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  1) , NO_SRC1, 0x00010000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  1) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  3) , NO_SRC1, 0x00000001, 0x00000007, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  3) , NO_SRC1, 0x0000FFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  3) , NO_SRC1, 0x00010000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  3) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  7) , NO_SRC1, 0x00000001, 0x0000007F, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  7) , NO_SRC1, 0x0000FFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  7) , NO_SRC1, 0x00010000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15,  7) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15, 15) , NO_SRC1, 0x00000001, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15, 15) , NO_SRC1, 0x0000FFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15, 15) , NO_SRC1, 0x00010000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15, 15) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15, 30) , NO_SRC1, 0x00000001, 0x00008000, 0       , 0       },
    {OP(0xC,  2, 0, 15, 30) , NO_SRC1, 0x0000FFFF, 0x3FFFFFFF, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15, 30) , NO_SRC1, 0x00010000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15, 30) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15, 31) , NO_SRC1, 0x00000001, 0x00008000, 0       , 0       },
    {OP(0xC,  2, 0, 15, 31) , NO_SRC1, 0x0000FFFF, 0x7FFF8000, 0       , 0       },
    {OP(0xC,  2, 0, 15, 31) , NO_SRC1, 0x00010000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 15, 31) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    // ... LSL 31
    {OP(0xC,  2, 0, 31,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 482
    {OP(0xC,  2, 0, 31,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 31,  1) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 31,  3) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 31,  7) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 31, 15) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 31, 30) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    {OP(0xC,  2, 0, 31, 31) , NO_SRC1, 0x00000001, 0x00000000, 0       , FLAG_Q  },
    // ... ASR 0 is USAT16, part of the DSP extension
    // ... ASR 3
    {OP(0xD,  2, 0,  3,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 490
    {OP(0xD,  2, 0,  3,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , 0       },
    {OP(0xD,  2, 0,  3,  0) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  0) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  0) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  1) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0,  3,  1) , NO_SRC1, 0x7FFFFFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  1) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  1) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  3) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0,  3,  3) , NO_SRC1, 0x7FFFFFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  3) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  3) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  7) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0,  3,  7) , NO_SRC1, 0x7FFFFFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  7) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3,  7) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3, 15) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0,  3, 15) , NO_SRC1, 0x7FFFFFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3, 15) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3, 15) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3, 30) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0,  3, 30) , NO_SRC1, 0x7FFFFFFF, 0x0FFFFFFF, 0       , 0       },
    {OP(0xD,  2, 0,  3, 30) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3, 30) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3, 31) , NO_SRC1, 0x00000008, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0,  3, 31) , NO_SRC1, 0x7FFFFFFF, 0x0FFFFFFF, 0       , 0       },
    {OP(0xD,  2, 0,  3, 31) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0,  3, 31) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    // ... ASR 15
    {OP(0xD,  2, 0, 15,  0) , NO_SRC1, 0x00000000, 0x00000000, 0       , 0       }, // 519
    {OP(0xD,  2, 0, 15,  0) , NO_SRC1, 0x00000001, 0x00000000, 0       , 0       },
    {OP(0xD,  2, 0, 15,  0) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  0) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  0) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  1) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0, 15,  1) , NO_SRC1, 0x7FFFFFFF, 0x00000001, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  1) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  1) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  3) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0, 15,  3) , NO_SRC1, 0x7FFFFFFF, 0x00000007, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  3) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  3) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  7) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0, 15,  7) , NO_SRC1, 0x7FFFFFFF, 0x0000007F, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  7) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15,  7) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15, 15) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0, 15, 15) , NO_SRC1, 0x7FFFFFFF, 0x00007FFF, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15, 15) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15, 15) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15, 30) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0, 15, 30) , NO_SRC1, 0x7FFFFFFF, 0x0000FFFF, 0       , 0       },
    {OP(0xD,  2, 0, 15, 30) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15, 30) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15, 31) , NO_SRC1, 0x00008000, 0x00000001, 0       , 0       },
    {OP(0xD,  2, 0, 15, 31) , NO_SRC1, 0x7FFFFFFF, 0x0000FFFF, 0       , 0       },
    {OP(0xD,  2, 0, 15, 31) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 15, 31) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    // ... ASR 31
    {OP(0xD,  2, 0, 31,  0) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       }, // 548
    {OP(0xD,  2, 0, 31,  0) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 31,  0) , NO_SRC1, 0xFFFFFFFF, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 31,  1) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0xD,  2, 0, 31,  1) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 31,  3) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0xD,  2, 0, 31,  3) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 31,  7) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0xD,  2, 0, 31,  7) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 31, 15) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0xD,  2, 0, 31, 15) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 31, 30) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0xD,  2, 0, 31, 30) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },
    {OP(0xD,  2, 0, 31, 31) , NO_SRC1, 0x7FFFFFFF, 0x00000000, 0       , 0       },
    {OP(0xD,  2, 0, 31, 31) , NO_SRC1, 0x80000000, 0x00000000, 0       , FLAG_Q  },

    // UBFX r0 r2 #lsb #width
    {OP(0xE,  2, 0,  0,  0) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       }, // 563
    {OP(0xE,  2, 0,  0,  0) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  0,  1) , NO_SRC1, 0x89ABCDEF, 0x00000003, 0       , 0       },
    {OP(0xE,  2, 0,  0,  1) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  0,  3) , NO_SRC1, 0x89ABCDEF, 0x0000000F, 0       , 0       },
    {OP(0xE,  2, 0,  0,  3) , NO_SRC1, 0xFEDCBA98, 0x00000008, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  0,  7) , NO_SRC1, 0x89ABCDEF, 0x000000EF, 0       , 0       },
    {OP(0xE,  2, 0,  0,  7) , NO_SRC1, 0xFEDCBA98, 0x00000098, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  0, 15) , NO_SRC1, 0x89ABCDEF, 0x0000CDEF, 0       , 0       },
    {OP(0xE,  2, 0,  0, 15) , NO_SRC1, 0xFEDCBA98, 0x0000BA98, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  0, 30) , NO_SRC1, 0x89ABCDEF, 0x09ABCDEF, 0       , 0       },
    {OP(0xE,  2, 0,  0, 30) , NO_SRC1, 0xFEDCBA98, 0x7EDCBA98, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  0, 31) , NO_SRC1, 0x89ABCDEF, 0x89ABCDEF, 0       , 0       },
    {OP(0xE,  2, 0,  0, 31) , NO_SRC1, 0xFEDCBA98, 0xFEDCBA98, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  1,  0) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       }, // 577
    {OP(0xE,  2, 0,  1,  0) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  1,  1) , NO_SRC1, 0x89ABCDEF, 0x00000003, 0       , 0       },
    {OP(0xE,  2, 0,  1,  1) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  1,  3) , NO_SRC1, 0x89ABCDEF, 0x00000007, 0       , 0       },
    {OP(0xE,  2, 0,  1,  3) , NO_SRC1, 0xFEDCBA98, 0x0000000C, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  1,  7) , NO_SRC1, 0x89ABCDEF, 0x000000F7, 0       , 0       },
    {OP(0xE,  2, 0,  1,  7) , NO_SRC1, 0xFEDCBA98, 0x0000004C, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  1, 15) , NO_SRC1, 0x89ABCDEF, 0x0000E6F7, 0       , 0       },
    {OP(0xE,  2, 0,  1, 15) , NO_SRC1, 0xFEDCBA98, 0x00005D4C, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  1, 29) , NO_SRC1, 0x89ABCDEF, 0x04D5E6F7, 0       , 0       },
    {OP(0xE,  2, 0,  1, 29) , NO_SRC1, 0xFEDCBA98, 0x3F6E5D4C, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  1, 30) , NO_SRC1, 0x89ABCDEF, 0x44D5E6F7, 0       , 0       },
    {OP(0xE,  2, 0,  1, 30) , NO_SRC1, 0xFEDCBA98, 0x7F6E5D4C, PSR_MASK, PSR_MASK},
    // lsb + width > 32 faults
    {OP(0xE,  2, 0,  3,  0) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       }, // 591
    {OP(0xE,  2, 0,  3,  0) , NO_SRC1, 0xFEDCBA98, 0x00000001, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  3,  1) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       },
    {OP(0xE,  2, 0,  3,  1) , NO_SRC1, 0xFEDCBA98, 0x00000003, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  3,  3) , NO_SRC1, 0x89ABCDEF, 0x0000000D, 0       , 0       },
    {OP(0xE,  2, 0,  3,  3) , NO_SRC1, 0xFEDCBA98, 0x00000003, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  3,  7) , NO_SRC1, 0x89ABCDEF, 0x000000BD, 0       , 0       },
    {OP(0xE,  2, 0,  3,  7) , NO_SRC1, 0xFEDCBA98, 0x00000053, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  3, 15) , NO_SRC1, 0x89ABCDEF, 0x000079BD, 0       , 0       },
    {OP(0xE,  2, 0,  3, 15) , NO_SRC1, 0xFEDCBA98, 0x00009753, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  3, 27) , NO_SRC1, 0x89ABCDEF, 0x013579BD, 0       , 0       },
    {OP(0xE,  2, 0,  3, 27) , NO_SRC1, 0xFEDCBA98, 0x0FDB9753, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0,  3, 28) , NO_SRC1, 0x89ABCDEF, 0x113579BD, 0       , 0       },
    {OP(0xE,  2, 0,  3, 28) , NO_SRC1, 0xFEDCBA98, 0x1FDB9753, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 14,  0) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       }, // 605
    {OP(0xE,  2, 0, 14,  0) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 14,  1) , NO_SRC1, 0x89ABCDEF, 0x00000003, 0       , 0       },
    {OP(0xE,  2, 0, 14,  1) , NO_SRC1, 0xFEDCBA98, 0x00000002, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 14,  3) , NO_SRC1, 0x89ABCDEF, 0x0000000F, 0       , 0       },
    {OP(0xE,  2, 0, 14,  3) , NO_SRC1, 0xFEDCBA98, 0x00000002, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 14,  7) , NO_SRC1, 0x89ABCDEF, 0x000000AF, 0       , 0       },
    {OP(0xE,  2, 0, 14,  7) , NO_SRC1, 0xFEDCBA98, 0x00000072, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 14, 15) , NO_SRC1, 0x89ABCDEF, 0x000026AF, 0       , 0       },
    {OP(0xE,  2, 0, 14, 15) , NO_SRC1, 0xFEDCBA98, 0x0000FB72, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 14, 16) , NO_SRC1, 0x89ABCDEF, 0x000026AF, 0       , 0       },
    {OP(0xE,  2, 0, 14, 16) , NO_SRC1, 0xFEDCBA98, 0x0001FB72, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 14, 17) , NO_SRC1, 0x89ABCDEF, 0x000226AF, 0       , 0       },
    {OP(0xE,  2, 0, 14, 17) , NO_SRC1, 0xFEDCBA98, 0x0003FB72, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 21,  0) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       }, // 619
    {OP(0xE,  2, 0, 21,  0) , NO_SRC1, 0xFEDCBA98, 0x00000000, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 21,  1) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       },
    {OP(0xE,  2, 0, 21,  1) , NO_SRC1, 0xFEDCBA98, 0x00000002, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 21,  3) , NO_SRC1, 0x89ABCDEF, 0x0000000D, 0       , 0       },
    {OP(0xE,  2, 0, 21,  3) , NO_SRC1, 0xFEDCBA98, 0x00000006, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 21,  7) , NO_SRC1, 0x89ABCDEF, 0x0000004D, 0       , 0       },
    {OP(0xE,  2, 0, 21,  7) , NO_SRC1, 0xFEDCBA98, 0x000000F6, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 21, 10) , NO_SRC1, 0x89ABCDEF, 0x0000044D, 0       , 0       },
    {OP(0xE,  2, 0, 21, 10) , NO_SRC1, 0xFEDCBA98, 0x000007F6, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 30,  0) , NO_SRC1, 0x89ABCDEF, 0x00000000, 0       , 0       }, // 629
    {OP(0xE,  2, 0, 30,  0) , NO_SRC1, 0xFEDCBA98, 0x00000001, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 30,  1) , NO_SRC1, 0x89ABCDEF, 0x00000002, 0       , 0       },
    {OP(0xE,  2, 0, 30,  1) , NO_SRC1, 0xFEDCBA98, 0x00000003, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 31,  0) , NO_SRC1, 0x89ABCDEF, 0x00000001, 0       , 0       },
    {OP(0xE,  2, 0, 31,  0) , NO_SRC1, 0xFEDCBA98, 0x00000001, PSR_MASK, PSR_MASK},
    {OP(0xE,  2, 0, 31,  0) , NO_SRC1, 0x76543210, 0x00000000, 0       , 0       },
};

#undef OP12
#undef OP16
#undef OP

static const int num_dp_plain_imm_tests = sizeof(dp_plain_imm_tests) / sizeof(dp_plain_imm_tests[0]);

// data processing, shifted reg
// for ops that don't set carry and use a non-zero shift, we get the carry from the shift
#define OP(op, s, rn, rd, rm, type, shift) (0xEA000000 | op << 21 | s << 20 | rn << 16 | (shift & 0x1C) << 10 | rd << 8 | (shift & 3) << 6 | type << 4 | rm)

static const struct TestInfo32 dp_shift_reg_tests[] = {
    // AND r0 r2 r1
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 0
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x00000000, 0              , FLAG_Z                  },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x00000000, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x00000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x80000000, 0x00000000, 0x00000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x80000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0x55555555, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x55555555, 0xAAAAAAAA, 0x00000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0xFFFFFFFF, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0x55555555, 0xFFFFFFFF, 0x55555555, 0              , 0                       },
    {OP(0x0, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x55555555, 0x55555555, FLAG_C         , FLAG_C                  },

    // TST r2 r1
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000000, 0xDAB00BAD, 0              , FLAG_Z                  }, // 18
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000000, 0xDAB00BAD, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0xDAB00BAD, 0              , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0xDAB00BAD, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C         , FLAG_C                  },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x00000000, 0x80000000, 0xDAB00BAD, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x80000000, 0x00000000, 0xDAB00BAD, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x80000000, 0x80000000, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0xAAAAAAAA, 0x55555555, 0xDAB00BAD, FLAG_N         , FLAG_Z                  },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x55555555, 0xAAAAAAAA, 0xDAB00BAD, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0xAAAAAAAA, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0xAAAAAAAA, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0x55555555, 0xFFFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x0, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x55555555, 0xDAB00BAD, FLAG_C         , FLAG_C                  },

    // BIC r0 r2 r1
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 36
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x00000000, 0              , FLAG_Z                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x80000000, 0x00000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x80000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x00000000, FLAG_C | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0x55555555, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0xFFFFFFFF, 0x55555555, FLAG_C         , FLAG_C                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0xAAAAAAAA, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0x55555555, 0xFFFFFFFF, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x1, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x55555555, 0x00000000, FLAG_N         , FLAG_Z                  },

    // ORR r0 r2 r1
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 54
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C         , FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x80000000, 0x00000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x80000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0x55555555, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x55555555, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_N         , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0x55555555, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x2, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x55555555, 0xFFFFFFFF, FLAG_C         , FLAG_C | FLAG_N         },

    // MOV r0 r1
    // also all the shifts...
    {OP(0x2, 1, 15,  0, 1, 0, 0), 0x00000000,    NO_SRC2, 0x00000000, 0              , FLAG_Z                  }, // 72
    {OP(0x2, 1, 15,  0, 1, 0, 0), 0x00000000,    NO_SRC2, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x2, 1, 15,  0, 1, 0, 0), 0x7FFFFFFF,    NO_SRC2, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x2, 1, 15,  0, 1, 0, 0), 0xFFFFFFFF,    NO_SRC2, 0xFFFFFFFF, FLAG_C         , FLAG_C | FLAG_N         },
    {OP(0x2, 1, 15,  0, 1, 0, 0), 0x80000000,    NO_SRC2, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x2, 1, 15,  0, 1, 0, 0), 0xAAAAAAAA,    NO_SRC2, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },

    // LSL r0 r1
    // 0 would be MOV
    {OP(0x2, 1, 15, 0, 1, 0,  1), 0x55555555,    NO_SRC2, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_N                  }, // 78
    {OP(0x2, 1, 15, 0, 1, 0,  1), 0xAAAAAAAA,    NO_SRC2, 0x55555554, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x2, 1, 15, 0, 1, 0, 31), 0x55555555,    NO_SRC2, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x2, 1, 15, 0, 1, 0, 31), 0xAAAAAAAA,    NO_SRC2, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },

    // LSR r0 r1
    {OP(0x2, 1, 15, 0, 1, 1,  0), 0x55555555,    NO_SRC2, 0x00000000, FLAG_Z | FLAG_N, FLAG_Z                  }, // 82
    {OP(0x2, 1, 15, 0, 1, 1,  0), 0xAAAAAAAA,    NO_SRC2, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x2, 1, 15, 0, 1, 1,  1), 0x55555555,    NO_SRC2, 0x2AAAAAAA, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x2, 1, 15, 0, 1, 1,  1), 0xAAAAAAAA,    NO_SRC2, 0x55555555, FLAG_C | FLAG_N, 0                       },
    {OP(0x2, 1, 15, 0, 1, 1, 31), 0x55555555,    NO_SRC2, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x2, 1, 15, 0, 1, 1, 31), 0xAAAAAAAA,    NO_SRC2, 0x00000001, FLAG_Z | FLAG_N, 0                       },

    // ASR r0 r1
    {OP(0x2, 1, 15, 0, 1, 2,  0), 0x55555555,    NO_SRC2, 0x00000000, FLAG_Z | FLAG_N, FLAG_Z                  }, // 88
    {OP(0x2, 1, 15, 0, 1, 2,  0), 0xAAAAAAAA,    NO_SRC2, 0xFFFFFFFF, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x2, 1, 15, 0, 1, 2,  1), 0x55555555,    NO_SRC2, 0x2AAAAAAA, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x2, 1, 15, 0, 1, 2,  1), 0xAAAAAAAA,    NO_SRC2, 0xD5555555, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x2, 1, 15, 0, 1, 2, 31), 0x55555555,    NO_SRC2, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0x2, 1, 15, 0, 1, 2, 31), 0xAAAAAAAA,    NO_SRC2, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },

    // ROR r0 r1
    // 0 would be RRX
    {OP(0x2, 1, 15, 0, 1, 3,  1), 0x55555555,    NO_SRC2, 0xAAAAAAAA, FLAG_Z | FLAG_N, FLAG_C | FLAG_N         }, // 94
    {OP(0x2, 1, 15, 0, 1, 3,  1), 0xAAAAAAAA,    NO_SRC2, 0x55555555, FLAG_C | FLAG_N, 0                       },
    {OP(0x2, 1, 15, 0, 1, 3, 31), 0x55555555,    NO_SRC2, 0xAAAAAAAA, FLAG_Z | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x2, 1, 15, 0, 1, 3, 31), 0xAAAAAAAA,    NO_SRC2, 0x55555555, FLAG_C | FLAG_N, 0                       },
    {OP(0x2, 1, 15, 0, 1, 3,  1), 0x12345678,    NO_SRC2, 0x091A2B3C, FLAG_Z | FLAG_N, 0                       },
    {OP(0x2, 1, 15, 0, 1, 3,  4), 0x12345678,    NO_SRC2, 0x81234567, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x2, 1, 15, 0, 1, 3, 16), 0x12345678,    NO_SRC2, 0x56781234, FLAG_Z | FLAG_N, 0                       },

    // ORN r0 r2 r1
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0xFFFFFFFF, 0              , FLAG_N                  }, // 101
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0xFFFFFFFF, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x80000000, FLAG_C         , FLAG_C | FLAG_N         },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x00000000, 0x80000000, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x80000000, 0x00000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_C                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x80000000, 0x80000000, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0x55555555, 0x55555555, FLAG_Z         ,                         },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_N         , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0x55555555, 0xFFFFFFFF, 0xFFFFFFFF, 0              , FLAG_N                  },
    {OP(0x3, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x55555555, 0x55555555, FLAG_C         , FLAG_C                  },

    // MVN r0 r1
    {OP(0x3, 1, 15,  0, 1, 0, 0), 0x00000000,    NO_SRC2, 0xFFFFFFFF, 0              , FLAG_N                  }, // 119
    {OP(0x3, 1, 15,  0, 1, 0, 0), 0x00000000,    NO_SRC2, 0xFFFFFFFF, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0x3, 1, 15,  0, 1, 0, 0), 0x7FFFFFFF,    NO_SRC2, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x3, 1, 15,  0, 1, 0, 0), 0xFFFFFFFF,    NO_SRC2, 0x00000000, FLAG_C         , FLAG_C | FLAG_Z         },
    {OP(0x3, 1, 15,  0, 1, 0, 0), 0x80000000,    NO_SRC2, 0x7FFFFFFF, FLAG_N         , 0                       },
    {OP(0x3, 1, 15,  0, 1, 0, 0), 0x55555555,    NO_SRC2, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x3, 1, 15,  0, 1, 0, 0), 0xAAAAAAAA,    NO_SRC2, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },

    // EOR r0 r2 r1
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 126
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0              , 0                       },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_C         , FLAG_C                  },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x80000000, 0x00000000, 0x80000000, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x80000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_Z                  },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, FLAG_C                  },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0x55555555, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x55555555, 0xAAAAAAAA, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0xAAAAAAAA, 0xFFFFFFFF, 0x55555555, 0              , 0                       },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, FLAG_C         , FLAG_C                  },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0x55555555, 0xFFFFFFFF, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },

    // TEQ r2 r1
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000000, 0xDAB00BAD, 0              , FLAG_Z                  }, // 144
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000000, 0xDAB00BAD, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0xDAB00BAD, FLAG_C         , FLAG_C                  },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xDAB00BAD, FLAG_N         , FLAG_Z                  },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x00000000, 0x80000000, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x80000000, 0x00000000, 0xDAB00BAD, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x80000000, 0x80000000, 0xDAB00BAD, FLAG_N         , FLAG_Z                  },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , 0                       },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C                  },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0xAAAAAAAA, 0x55555555, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x55555555, 0xAAAAAAAA, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0xAAAAAAAA, 0xFFFFFFFF, 0xDAB00BAD, 0              , 0                       },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0xAAAAAAAA, 0xDAB00BAD, FLAG_C         , FLAG_C                  },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0x55555555, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z         , FLAG_N                  },
    {OP(0x4, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x55555555, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_C | FLAG_N         },

    // ADD r0 r2 imm
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 162
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_Z                  },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x00000001, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000001, 0x00000001, FLAG_C | FLAG_N, 0                       },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z, 0                       },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_V | FLAG_Z, 0                       },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x00000001, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000001, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFE, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x80000000, 0x00000000, 0x80000000, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x80000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x00000001, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    
    // CMN r2 r1
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000000, 0xDAB00BAD, 0              , FLAG_Z                  }, // 185
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000000, 0xDAB00BAD, PSR_VCZN       , FLAG_Z                  },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x00000001, 0x00000000, 0xDAB00BAD, FLAG_Z | FLAG_N, 0                       },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000001, 0xDAB00BAD, FLAG_C | FLAG_N, 0                       },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, 0                       },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_Z, 0                       },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x00000001, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x01000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x80000000, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x80000000, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x00000000, 0x80000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x80000000, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x80000000, 0x80000000, 0xDAB00BAD, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x00000000, 0xFFFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_C, FLAG_N                  },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x00000001, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x00000001, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0x8, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },

    // ADC/SBC elsewhere

    // SUB r0 r2 r1
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 208
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x00000001, 0x00000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000001, 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x80000001, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x00000001, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000001, 0x80000002, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x80000000, 0x00000001, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x80000000, 0x00000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x80000000, 0x80000000, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x00000000, 0x00000001, FLAG_V | FLAG_C, 0                       },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x00000001, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x00000001, 0x00000002, FLAG_V | FLAG_N, 0                       },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x80000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // CMP r2 r1
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000000, 0xDAB00BAD, 0              , FLAG_C | FLAG_Z         }, // 231
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000000, 0xDAB00BAD, PSR_VCZN       , FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x00000001, 0x00000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x00000000, 0x00000001, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x00000001, 0x7FFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x00000001, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x80000000, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0x80000000, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x00000000, 0x80000000, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x80000000, 0x00000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x80000000, 0x80000000, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x00000000, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x00000000, 0xDAB00BAD, FLAG_V | FLAG_C, 0                       },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x00000001, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x00000001, 0xDAB00BAD, FLAG_V | FLAG_N, 0                       },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0xDAB00BAD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0xDAB00BAD, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2, 15, 1, 0, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0xDAB00BAD, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // RSB r0 r2 imm
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_C | FLAG_Z         }, // 254
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, PSR_VCZN       , FLAG_C | FLAG_Z         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x00000000, 0x00000001, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x00000001, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x80000001, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x00000001, 0x7FFFFFFF, 0x80000002, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x80000000, 0x7FFFFFFF, 0x00000001, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x80000000, 0x00000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x80000000, 0x80000000, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x00000000, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_C, 0                       },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x00000001, 0xFFFFFFFF, 0x00000002, FLAG_V | FLAG_N, 0                       },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x80000001, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xE, 1,  2,  0, 1, 0, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    // a few from everything, but with shifts
    {OP(0x0, 1,  2,  0, 1, 0, 31), 0x00000001, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  }, // 277
    {OP(0x0, 1,  2,  0, 1, 3,  1), 0x00000001, 0x80000000, 0x80000000, FLAG_Z         , FLAG_C |FLAG_N          },
    {OP(0x0, 1,  2,  0, 1, 2,  0), 0x80000000, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_V | FLAG_Z, FLAG_V | FLAG_C | FLAG_N},
    {OP(0x0, 1,  2,  0, 1, 2, 31), 0x80000000, 0xAAAAAAAA, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_N                  },

    {OP(0x1, 1,  2,  0, 1, 0, 31), 0x00000001, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C         , 0                       },
    {OP(0x1, 1,  2,  0, 1, 3,  1), 0x00000001, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_N         , FLAG_C                  },

    {OP(0x2, 1,  2,  0, 1, 0, 31), 0x00000001, 0x00000000, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x2, 1,  2,  0, 1, 3,  1), 0x00000001, 0x00000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_V | FLAG_C | FLAG_N},
    {OP(0x2, 1,  2,  0, 1, 2,  0), 0x80000000, 0x55555555, 0xFFFFFFFF, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(0x2, 1,  2,  0, 1, 2, 31), 0x80000000, 0x55555555, 0xFFFFFFFF, FLAG_C         , FLAG_N                  },

    {OP(0x3, 1,  2,  0, 1, 0, 31), 0x00000001, 0x00000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, 0                       },
    {OP(0x3, 1,  2,  0, 1, 3,  1), 0x00000001, 0x00000000, 0x7FFFFFFF, FLAG_V | FLAG_Z, FLAG_V | FLAG_C         },

    {OP(0x3, 1, 15,  0, 1, 0, 31), 0x00000001,    NO_SRC2, 0x7FFFFFFF, FLAG_N         , 0                       },
    {OP(0x3, 1, 15,  0, 1, 3,  1), 0x00000001,    NO_SRC2, 0x7FFFFFFF, FLAG_N         , FLAG_C                  },
    {OP(0x3, 1, 15,  0, 1, 2,  0), 0x80000000,    NO_SRC2, 0x00000000, FLAG_V         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0x3, 1, 15,  0, 1, 2, 31), 0x80000000,    NO_SRC2, 0x00000000, FLAG_C         , FLAG_Z                  },

    {OP(0x4, 1,  2,  0, 1, 0, 31), 0x00000001, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0x4, 1,  2,  0, 1, 3,  1), 0x00000001, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z         , FLAG_C                  },
    {OP(0x4, 1,  2,  0, 1, 2,  0), 0x80000000, 0x80000000, 0x7FFFFFFF, FLAG_V | FLAG_Z, FLAG_V | FLAG_C         },
    {OP(0x4, 1,  2,  0, 1, 2, 31), 0x80000000, 0x80000000, 0x7FFFFFFF, FLAG_C | FLAG_Z, 0                       },

    {OP(0x8, 1,  2,  0, 1, 1, 31), 0xC0000000, 0x00000000, 0x00000001, FLAG_Z | FLAG_N,                         },
    {OP(0x8, 1,  2,  0, 1, 0, 31), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 1, 3,  1), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0x8, 1,  2,  0, 1, 2,  0), 0x80000000, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0x8, 1,  2,  0, 1, 2, 31), 0x80000000, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    {OP(0xD, 1,  2,  0, 1, 1, 31), 0xC0000000, 0x00000000, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xD, 1,  2,  0, 1, 0, 31), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2,  0, 1, 3,  1), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_V | FLAG_N         },
    {OP(0xD, 1,  2,  0, 1, 2,  0), 0x80000000, 0x00000001, 0x00000002, FLAG_V | FLAG_N, 0                       },
    {OP(0xD, 1,  2,  0, 1, 2, 31), 0x80000000, 0x00000001, 0x00000002, FLAG_V | FLAG_N, 0                       },

    {OP(0xE, 1,  2,  0, 1, 1, 31), 0xC0000000, 0x7FFFFFFF, 0x80000002, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0xE, 1,  2,  0, 1, 0, 31), 0x00000001, 0x7FFFFFFF, 0x00000001, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xE, 1,  2,  0, 1, 3,  1), 0x00000001, 0x7FFFFFFF, 0x00000001, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xE, 1,  2,  0, 1, 2,  0), 0x80000000, 0x00000001, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xE, 1,  2,  0, 1, 2, 31), 0x80000000, 0x00000001, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
};

static const int num_dp_shift_reg_tests = sizeof(dp_shift_reg_tests) / sizeof(dp_shift_reg_tests[0]);

static const struct TestInfo32 dp_shift_reg_c_tests[] = {
    // ADC r0 r2 r1
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, 0              , FLAG_Z                  }, // 0
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000001, PSR_VCZN       , 0                       },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000001, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0x00000001, 0x00000001, FLAG_N         , 0                       },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x7FFFFFFF, FLAG_V | FLAG_Z, 0                       },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000001, 0x7FFFFFFF, 0x80000000, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x00000001, 0x80000000, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFE, FLAG_V | FLAG_N, FLAG_C                  },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x80000000, 0x00000000, 0x80000000, FLAG_V         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x80000000, 0x80000000, 0x00000000, FLAG_N         , FLAG_V | FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, FLAG_V         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000001, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, FLAG_Z | FLAG_N, FLAG_V | FLAG_C         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },

    // now with C set
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000001, 0x00000000, 0x00000002, FLAG_C | FLAG_Z | FLAG_N, 0              }, // 23
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0x00000001, 0x00000002, FLAG_C | FLAG_N         , 0              },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x80000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000001, 0x7FFFFFFF, 0x80000001, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x00000001, 0x80000001, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x80000000, 0x7FFFFFFF, 0x00000000, FLAG_C | FLAG_Z         , FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x80000000, 0x00000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_C | FLAG_N, FLAG_C         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0x80000000, 0x80000001, FLAG_C | FLAG_Z         , FLAG_N         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x80000000, 0x00000000, 0x80000001, FLAG_V | FLAG_C         , FLAG_N         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x80000000, 0x80000000, 0x00000001, FLAG_C | FLAG_N         , FLAG_V | FLAG_C},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000000, 0xFFFFFFFF, 0x00000000, FLAG_C | FLAG_Z         , FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x00000000, 0x00000000, FLAG_V | FLAG_C         , FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x00000001, 0xFFFFFFFF, 0x00000001, FLAG_V | FLAG_C | FLAG_N, FLAG_C         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x00000001, 0x00000001, FLAG_V | FLAG_C | FLAG_N, FLAG_C         },
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x80000000, FLAG_C | FLAG_Z | FLAG_N, FLAG_C | FLAG_N},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x80000000, FLAG_C | FLAG_Z | FLAG_N, FLAG_C | FLAG_N},
    {OP(0xA, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},

    // SBC r0 r2 r1
    // start with C set
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0x00000000, 0xFFFFFFFF, 0                       , FLAG_N         }, // 44
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0x00000000, 0x00000000, PSR_VCZN                , FLAG_C | FLAG_Z},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000001, 0x00000000, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0x00000001, 0x00000001, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x80000001, FLAG_V | FLAG_C | FLAG_Z, FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000001, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x00000001, 0x80000002, FLAG_C | FLAG_Z         , FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C | FLAG_Z},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x80000000, 0x00000001, FLAG_C | FLAG_Z | FLAG_N, FLAG_V | FLAG_C},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0x80000000, 0x80000000, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x80000000, 0x00000000, 0x80000000, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x80000000, 0x80000000, 0x00000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C | FLAG_Z},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x00000000, 0x00000001, FLAG_V | FLAG_C | FLAG_C, 0              },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000001, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_C | FLAG_Z, FLAG_C | FLAG_N},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x00000001, 0x00000002, FLAG_V | FLAG_C | FLAG_N, 0              },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_C | FLAG_Z | FLAG_N, FLAG_C         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x80000001, FLAG_C | FLAG_Z         , FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, FLAG_V | FLAG_C | FLAG_N, FLAG_C | FLAG_Z},

    // now with C clear
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000001, 0x00000000, 0xFFFFFFFE,          FLAG_Z, FLAG_N                  }, // 67
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0x00000001, 0x00000000, FLAG_Z | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0x7FFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x00000000, 0x80000000, FLAG_V | FLAG_Z, FLAG_N                  },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000001, 0x7FFFFFFF, 0x7FFFFFFD, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x00000001, 0x80000001, FLAG_Z         , FLAG_N                  },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_N                  },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x80000000, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0x80000000, 0x00000000, FLAG_Z | FLAG_N, FLAG_V | FLAG_C | FLAG_Z},
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, FLAG_V | FLAG_Z, FLAG_V | FLAG_C         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0x80000000, 0x7FFFFFFF, FLAG_V | FLAG_Z, FLAG_V | FLAG_C         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x80000000, 0x00000000, 0x7FFFFFFF, FLAG_Z         , 0                       },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x80000000, 0x80000000, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_N                  },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000000, 0xFFFFFFFF, 0xFFFFFFFE, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x00000000, 0x00000000, FLAG_V         , FLAG_Z                  },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x00000001, 0xFFFFFFFF, 0xFFFFFFFD, FLAG_V | FLAG_Z, FLAG_C | FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x00000001, 0x00000001, FLAG_V | FLAG_N, 0                       },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFE, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0x80000000, 0x80000000, FLAG_Z         , FLAG_N                  },
    {OP(0xB, 1, 2, 0, 1, 0, 0), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, FLAG_V | FLAG_N, FLAG_N                  },

    // RRX r0 r1
    {OP(0x2, 1, 15, 0, 1, 3, 0), 0x55555555,    NO_SRC2, 0x2AAAAAAA, FLAG_Z | FLAG_N, FLAG_C                  }, // 88
    {OP(0x2, 1, 15, 0, 1, 3, 0), 0xAAAAAAAA,    NO_SRC2, 0x55555555, FLAG_V | FLAG_N, FLAG_V                  },
    {OP(0x2, 1, 15, 0, 1, 3, 0), 0x55555555,    NO_SRC2, 0xAAAAAAAA, FLAG_C | FLAG_N, FLAG_C | FLAG_N         },
    {OP(0x2, 1, 15, 0, 1, 3, 0), 0xAAAAAAAA,    NO_SRC2, 0xD5555555, FLAG_C | FLAG_N, FLAG_N                  },

    // shifty
    {OP(0xA, 1, 2, 0, 1, 1, 31), 0xC0000000, 0x00000000, 0x00000001, FLAG_Z | FLAG_N, 0                       }, // 92
    {OP(0xA, 1, 2, 0, 1, 0, 31), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 1, 3,  1), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_Z         , FLAG_N                  },
    {OP(0xA, 1, 2, 0, 1, 2,  0), 0x80000000, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },
    {OP(0xA, 1, 2, 0, 1, 2, 31), 0x80000000, 0x00000001, 0x00000000, FLAG_V | FLAG_N, FLAG_C | FLAG_Z         },

    {OP(0xA, 1, 2, 0, 1, 1, 31), 0xC0000000, 0x00000000, 0x00000002, FLAG_C | FLAG_Z | FLAG_N, 0              },
    {OP(0xA, 1, 2, 0, 1, 0, 31), 0x00000001, 0x7FFFFFFF, 0x00000000, FLAG_C | FLAG_Z         , FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 1, 3,  1), 0x00000001, 0x7FFFFFFF, 0x00000000, FLAG_C | FLAG_Z         , FLAG_C | FLAG_Z},
    {OP(0xA, 1, 2, 0, 1, 2,  0), 0x80000000, 0x00000001, 0x00000001, FLAG_V | FLAG_C | FLAG_N, FLAG_C         },
    {OP(0xA, 1, 2, 0, 1, 2, 31), 0x80000000, 0x00000001, 0x00000001, FLAG_V | FLAG_C | FLAG_N, FLAG_C         },

    {OP(0xB, 1, 2, 0, 1, 1, 31), 0xC0000000, 0x00000000, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 0, 31), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xB, 1, 2, 0, 1, 3,  1), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, FLAG_C | FLAG_Z         , FLAG_V | FLAG_N},
    {OP(0xB, 1, 2, 0, 1, 2,  0), 0x80000000, 0x00000001, 0x00000002, FLAG_V | FLAG_C | FLAG_N, 0              },
    {OP(0xB, 1, 2, 0, 1, 2, 31), 0x80000000, 0x00000001, 0x00000002, FLAG_V | FLAG_C | FLAG_N, 0              },

    {OP(0xB, 1, 2, 0, 1, 1, 31), 0xC0000000, 0x00000000, 0xFFFFFFFE,          FLAG_Z, FLAG_N                  },
    {OP(0xB, 1, 2, 0, 1, 0, 31), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 3,  1), 0x00000001, 0x7FFFFFFF, 0xFFFFFFFE, FLAG_Z         , FLAG_V | FLAG_N         },
    {OP(0xB, 1, 2, 0, 1, 2,  0), 0x80000000, 0x00000001, 0x00000001, FLAG_V | FLAG_N, 0                       },
    {OP(0xB, 1, 2, 0, 1, 2, 31), 0x80000000, 0x00000001, 0x00000001, FLAG_V | FLAG_N, 0                       },
};

#undef OP

static const int num_dp_shift_reg_c_tests = sizeof(dp_shift_reg_c_tests) / sizeof(dp_shift_reg_c_tests[0]);

// data processing, register
// first the shifts
#define OP(op, s, rn, rd, rm) (0xFA00F000 | op << 21 | s << 20 | rn << 16 | rd << 8 | rm)

static const struct TestInfo32 dp_reg_shift_tests[] = {
    // LSL r0 r1 r2
    {OP(0, 1, 1, 0, 2), 0x55555555, 0x00000000, 0x55555555, FLAG_Z | FLAG_N, 0                       }, // 0
    {OP(0, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000000, 0xAAAAAAAA, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(0, 1, 1, 0, 2), 0x55555555, 0x00000001, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000001, 0x55555554, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(0, 1, 1, 0, 2), 0x55555555, 0x0000001F, 0x80000000, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(0, 1, 1, 0, 2), 0xAAAAAAAA, 0x0000001F, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0, 1, 1, 0, 2), 0x55555555, 0x00000020, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(0, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000020, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(0, 1, 1, 0, 2), 0x55555555, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  }, // too big
    {OP(0, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(0, 1, 1, 0, 2), 0x55555555, 0x00000100, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  }, // only low byte is used
    {OP(0, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000100, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(0, 1, 1, 0, 2), 0x55555555, 0x80000000, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(0, 1, 1, 0, 2), 0xAAAAAAAA, 0x80000000, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },

    // LSR r0 r1 r2
    {OP(1, 1, 1, 0, 2), 0x55555555, 0x00000000, 0x55555555, FLAG_Z | FLAG_N, 0                       }, // 14
    {OP(1, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000000, 0xAAAAAAAA, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(1, 1, 1, 0, 2), 0x55555555, 0x00000001, 0x2AAAAAAA, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(1, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000001, 0x55555555, FLAG_C | FLAG_N, 0                       },
    {OP(1, 1, 1, 0, 2), 0x55555555, 0x0000001F, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(1, 1, 1, 0, 2), 0xAAAAAAAA, 0x0000001F, 0x00000001, FLAG_Z | FLAG_N, 0                       },
    {OP(1, 1, 1, 0, 2), 0x55555555, 0x00000020, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(1, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000020, 0x00000000, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_Z},
    {OP(1, 1, 1, 0, 2), 0x55555555, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(1, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(1, 1, 1, 0, 2), 0x55555555, 0x00000100, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(1, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000100, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(1, 1, 1, 0, 2), 0x55555555, 0x80000000, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(1, 1, 1, 0, 2), 0xAAAAAAAA, 0x80000000, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },

    // ASR r0 r1 r2
    {OP(2, 1, 1, 0, 2), 0x55555555, 0x00000000, 0x55555555, FLAG_Z | FLAG_N, 0                       }, // 28
    {OP(2, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000000, 0xAAAAAAAA, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(2, 1, 1, 0, 2), 0x55555555, 0x00000001, 0x2AAAAAAA, FLAG_Z | FLAG_N, FLAG_C                  },
    {OP(2, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000001, 0xD5555555, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(2, 1, 1, 0, 2), 0x55555555, 0x0000001F, 0x00000000, FLAG_N         , FLAG_C | FLAG_Z         },
    {OP(2, 1, 1, 0, 2), 0xAAAAAAAA, 0x0000001F, 0xFFFFFFFF, FLAG_C | FLAG_Z, FLAG_N                  },
    {OP(2, 1, 1, 0, 2), 0x55555555, 0x00000020, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(2, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000020, 0xFFFFFFFF, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(2, 1, 1, 0, 2), 0x55555555, 0x00000080, 0x00000000, FLAG_C | FLAG_N, FLAG_Z                  },
    {OP(2, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000080, 0xFFFFFFFF, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(2, 1, 1, 0, 2), 0x55555555, 0x00000100, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(2, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000100, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(2, 1, 1, 0, 2), 0x55555555, 0x80000000, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(2, 1, 1, 0, 2), 0xAAAAAAAA, 0x80000000, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },

    // ROR r0 r1 r2
    {OP(3, 1, 1, 0, 2), 0x55555555, 0x00000000, 0x55555555, FLAG_Z | FLAG_N, 0                       }, // 32
    {OP(3, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000000, 0xAAAAAAAA, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(3, 1, 1, 0, 2), 0x55555555, 0x00000001, 0xAAAAAAAA, FLAG_Z | FLAG_N, FLAG_C | FLAG_N         },
    {OP(3, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000001, 0x55555555, FLAG_C | FLAG_N, 0                       },
    {OP(3, 1, 1, 0, 2), 0x55555555, 0x0000001F, 0xAAAAAAAA, FLAG_Z | FLAG_N, FLAG_C | FLAG_N         },
    {OP(3, 1, 1, 0, 2), 0xAAAAAAAA, 0x0000001F, 0x55555555, FLAG_C | FLAG_N, 0                       },
    {OP(3, 1, 1, 0, 2), 0x55555555, 0x00000020, 0x55555555, FLAG_Z | FLAG_N, 0                       },
    {OP(3, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000020, 0xAAAAAAAA, PSR_VCZN       , FLAG_V | FLAG_C | FLAG_N},
    {OP(3, 1, 1, 0, 2), 0x55555555, 0x00000080, 0x55555555, FLAG_Z | FLAG_N, 0                       },
    {OP(3, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000080, 0xAAAAAAAA, FLAG_C | FLAG_Z, FLAG_N | FLAG_C         },
    {OP(3, 1, 1, 0, 2), 0x55555555, 0x00000100, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(3, 1, 1, 0, 2), 0xAAAAAAAA, 0x00000100, 0xAAAAAAAA, FLAG_Z         , FLAG_N                  },
    {OP(3, 1, 1, 0, 2), 0x55555555, 0x80000000, 0x55555555, FLAG_C | FLAG_N, FLAG_C                  },
    {OP(3, 1, 1, 0, 2), 0xAAAAAAAA, 0x80000000, 0xAAAAAAAA,          FLAG_Z, FLAG_N                  },
    {OP(3, 1, 1, 0, 2), 0x12345678, 0x00000001, 0x091A2B3C, FLAG_Z | FLAG_N, 0                       },
    {OP(3, 1, 1, 0, 2), 0x12345678, 0x00000004, 0x81234567, FLAG_Z         , FLAG_C | FLAG_N         },
    {OP(3, 1, 1, 0, 2), 0x12345678, 0x00000010, 0x56781234, FLAG_Z | FLAG_N, 0                       },
    {OP(3, 1, 1, 0, 2), 0x12345678, 0x00000020, 0x12345678, FLAG_C | FLAG_Z, 0                       },
};

#undef OP

static const int num_dp_reg_shift_tests = sizeof(dp_reg_shift_tests) / sizeof(dp_reg_shift_tests[0]);

#define OP(op, op2, rn, rd, rm) (0xFA00F000 | op << 20 | rn << 16 | rd << 8 | op2 << 4 | rm)

static const struct TestInfo32 dp_reg_tests[] = {
    // SXTH r0 r1
    {OP(0x0, 0x8, 15, 0, 1), 0x12340000, NO_SRC2, 0x00000000, 0              , 0       }, // 0
    {OP(0x0, 0x8, 15, 0, 1), 0x89AB0000, NO_SRC2, 0x00000000, PSR_MASK       , PSR_MASK},
    {OP(0x0, 0x8, 15, 0, 1), 0x123400FF, NO_SRC2, 0x000000FF, 0              , 0       },
    {OP(0x0, 0x8, 15, 0, 1), 0x89AB00FF, NO_SRC2, 0x000000FF, PSR_MASK       , PSR_MASK},
    {OP(0x0, 0x8, 15, 0, 1), 0x12347FFF, NO_SRC2, 0x00007FFF, 0              , 0       },
    {OP(0x0, 0x8, 15, 0, 1), 0x89AB7FFF, NO_SRC2, 0x00007FFF, PSR_MASK       , PSR_MASK},
    {OP(0x0, 0x8, 15, 0, 1), 0x12348000, NO_SRC2, 0xFFFF8000, 0              , 0       },
    {OP(0x0, 0x8, 15, 0, 1), 0x89AB8000, NO_SRC2, 0xFFFF8000, PSR_MASK       , PSR_MASK},
    {OP(0x0, 0x9, 15, 0, 1), 0x12347FFF, NO_SRC2, 0x0000347F, 0              , 0       },
    {OP(0x0, 0x9, 15, 0, 1), 0x89AB7FFF, NO_SRC2, 0xFFFFAB7F, PSR_MASK       , PSR_MASK},
    {OP(0x0, 0xA, 15, 0, 1), 0x12347FFF, NO_SRC2, 0x00001234, 0              , 0       },
    {OP(0x0, 0xA, 15, 0, 1), 0x89AB7FFF, NO_SRC2, 0xFFFF89AB, PSR_MASK       , PSR_MASK},
    {OP(0x0, 0xB, 15, 0, 1), 0x12347FFF, NO_SRC2, 0xFFFFFF12, 0              , 0       },
    {OP(0x0, 0xB, 15, 0, 1), 0x89AB7FFF, NO_SRC2, 0xFFFFFF89, PSR_MASK       , PSR_MASK},

    // UXTH r0 r1
    {OP(0x1, 0x8, 15, 0, 1), 0x12340000, NO_SRC2, 0x00000000, 0              , 0       }, // 14
    {OP(0x1, 0x8, 15, 0, 1), 0x89AB0000, NO_SRC2, 0x00000000, PSR_MASK       , PSR_MASK},
    {OP(0x1, 0x8, 15, 0, 1), 0x123400FF, NO_SRC2, 0x000000FF, 0              , 0       },
    {OP(0x1, 0x8, 15, 0, 1), 0x89AB00FF, NO_SRC2, 0x000000FF, PSR_MASK       , PSR_MASK},
    {OP(0x1, 0x8, 15, 0, 1), 0x12347FFF, NO_SRC2, 0x00007FFF, 0              , 0       },
    {OP(0x1, 0x8, 15, 0, 1), 0x89AB7FFF, NO_SRC2, 0x00007FFF, PSR_MASK       , PSR_MASK},
    {OP(0x1, 0x8, 15, 0, 1), 0x12348000, NO_SRC2, 0x00008000, 0              , 0       },
    {OP(0x1, 0x8, 15, 0, 1), 0x89AB8000, NO_SRC2, 0x00008000, PSR_MASK       , PSR_MASK},
    {OP(0x1, 0x9, 15, 0, 1), 0x12347FFF, NO_SRC2, 0x0000347F, 0              , 0       },
    {OP(0x1, 0x9, 15, 0, 1), 0x89AB7FFF, NO_SRC2, 0x0000AB7F, PSR_MASK       , PSR_MASK},
    {OP(0x1, 0xA, 15, 0, 1), 0x12347FFF, NO_SRC2, 0x00001234, 0              , 0       },
    {OP(0x1, 0xA, 15, 0, 1), 0x89AB7FFF, NO_SRC2, 0x000089AB, PSR_MASK       , PSR_MASK},
    {OP(0x1, 0xB, 15, 0, 1), 0x12347FFF, NO_SRC2, 0x0000FF12, 0              , 0       },
    {OP(0x1, 0xB, 15, 0, 1), 0x89AB7FFF, NO_SRC2, 0x0000FF89, PSR_MASK       , PSR_MASK},

    // SXTB r0 r1
    {OP(0x4, 0x8, 15, 0, 1), 0x12345600, NO_SRC2, 0x00000000, 0              , 0       }, // 28
    {OP(0x4, 0x8, 15, 0, 1), 0x89ABCD00, NO_SRC2, 0x00000000, PSR_MASK       , PSR_MASK},
    {OP(0x4, 0x8, 15, 0, 1), 0x1234560F, NO_SRC2, 0x0000000F, 0              , 0       },
    {OP(0x4, 0x8, 15, 0, 1), 0x89ABCD0F, NO_SRC2, 0x0000000F, PSR_MASK       , PSR_MASK},
    {OP(0x4, 0x8, 15, 0, 1), 0x1234567F, NO_SRC2, 0x0000007F, 0              , 0       },
    {OP(0x4, 0x8, 15, 0, 1), 0x89ABCD7F, NO_SRC2, 0x0000007F, PSR_MASK       , PSR_MASK},
    {OP(0x4, 0x8, 15, 0, 1), 0x12345680, NO_SRC2, 0xFFFFFF80, 0              , 0       },
    {OP(0x4, 0x8, 15, 0, 1), 0x89ABCD80, NO_SRC2, 0xFFFFFF80, PSR_MASK       , PSR_MASK},
    {OP(0x4, 0x9, 15, 0, 1), 0x1234567F, NO_SRC2, 0x00000056, 0              , 0       },
    {OP(0x4, 0x9, 15, 0, 1), 0x89ABCD7F, NO_SRC2, 0xFFFFFFCD, PSR_MASK       , PSR_MASK},
    {OP(0x4, 0xA, 15, 0, 1), 0x1234567F, NO_SRC2, 0x00000034, 0              , 0       },
    {OP(0x4, 0xA, 15, 0, 1), 0x89ABCD7F, NO_SRC2, 0xFFFFFFAB, PSR_MASK       , PSR_MASK},
    {OP(0x4, 0xB, 15, 0, 1), 0x1234567F, NO_SRC2, 0x00000012, 0              , 0       },
    {OP(0x4, 0xB, 15, 0, 1), 0x89ABCD7F, NO_SRC2, 0xFFFFFF89, PSR_MASK       , PSR_MASK},

    // UXTB r0 r1
    {OP(0x5, 0x8, 15, 0, 1), 0x12345600, NO_SRC2, 0x00000000, 0              , 0       }, // 42
    {OP(0x5, 0x8, 15, 0, 1), 0x89ABCD00, NO_SRC2, 0x00000000, PSR_MASK       , PSR_MASK},
    {OP(0x5, 0x8, 15, 0, 1), 0x1234560F, NO_SRC2, 0x0000000F, 0              , 0       },
    {OP(0x5, 0x8, 15, 0, 1), 0x89ABCD0F, NO_SRC2, 0x0000000F, PSR_MASK       , PSR_MASK},
    {OP(0x5, 0x8, 15, 0, 1), 0x1234567F, NO_SRC2, 0x0000007F, 0              , 0       },
    {OP(0x5, 0x8, 15, 0, 1), 0x89ABCD7F, NO_SRC2, 0x0000007F, PSR_MASK       , PSR_MASK},
    {OP(0x5, 0x8, 15, 0, 1), 0x12345680, NO_SRC2, 0x00000080, 0              , 0       },
    {OP(0x5, 0x8, 15, 0, 1), 0x89ABCD80, NO_SRC2, 0x00000080, PSR_MASK       , PSR_MASK},
    {OP(0x5, 0x9, 15, 0, 1), 0x1234567F, NO_SRC2, 0x00000056, 0              , 0       },
    {OP(0x5, 0x9, 15, 0, 1), 0x89ABCD7F, NO_SRC2, 0x000000CD, PSR_MASK       , PSR_MASK},
    {OP(0x5, 0xA, 15, 0, 1), 0x1234567F, NO_SRC2, 0x00000034, 0              , 0       },
    {OP(0x5, 0xA, 15, 0, 1), 0x89ABCD7F, NO_SRC2, 0x000000AB, PSR_MASK       , PSR_MASK},
    {OP(0x5, 0xB, 15, 0, 1), 0x1234567F, NO_SRC2, 0x00000012, 0              , 0       },
    {OP(0x5, 0xB, 15, 0, 1), 0x89ABCD7F, NO_SRC2, 0x00000089, PSR_MASK       , PSR_MASK},

    // REV r0 r1
    // rm is encoded twice
    {OP(0x9, 0x8, 1, 0, 1), 0x01234567, NO_SRC2, 0x67452301, 0              , 0       }, // 56
    {OP(0x9, 0x8, 1, 0, 1), 0x01234567, NO_SRC2, 0x67452301, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0x8, 1, 0, 1), 0x67452301, NO_SRC2, 0x01234567, 0              , 0       },
    {OP(0x9, 0x8, 1, 0, 1), 0x67452301, NO_SRC2, 0x01234567, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0x8, 1, 0, 1), 0x89ABCDEF, NO_SRC2, 0xEFCDAB89, 0              , 0       },
    {OP(0x9, 0x8, 1, 0, 1), 0x89ABCDEF, NO_SRC2, 0xEFCDAB89, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0x8, 1, 0, 1), 0xEFCDAB89, NO_SRC2, 0x89ABCDEF, 0              , 0       },
    {OP(0x9, 0x8, 1, 0, 1), 0xEFCDAB89, NO_SRC2, 0x89ABCDEF, PSR_MASK       , PSR_MASK},

    // REV16 r0 r1
    {OP(0x9, 0x9, 1, 0, 1), 0x01234567, NO_SRC2, 0x23016745, 0              , 0       }, // 64
    {OP(0x9, 0x9, 1, 0, 1), 0x01234567, NO_SRC2, 0x23016745, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0x9, 1, 0, 1), 0x23016745, NO_SRC2, 0x01234567, 0              , 0       },
    {OP(0x9, 0x9, 1, 0, 1), 0x23016745, NO_SRC2, 0x01234567, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0x9, 1, 0, 1), 0x89ABCDEF, NO_SRC2, 0xAB89EFCD, 0              , 0       },
    {OP(0x9, 0x9, 1, 0, 1), 0x89ABCDEF, NO_SRC2, 0xAB89EFCD, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0x9, 1, 0, 1), 0xAB89EFCD, NO_SRC2, 0x89ABCDEF, 0              , 0       },
    {OP(0x9, 0x9, 1, 0, 1), 0xAB89EFCD, NO_SRC2, 0x89ABCDEF, PSR_MASK       , PSR_MASK},

    // RBIT r0 r1
    {OP(0x9, 0xA, 1, 0, 1), 0x01234567, NO_SRC2, 0xE6A2C480, 0              , 0       }, // 72
    {OP(0x9, 0xA, 1, 0, 1), 0x01234567, NO_SRC2, 0xE6A2C480, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0xA, 1, 0, 1), 0xE6A2C480, NO_SRC2, 0x01234567, 0              , 0       },
    {OP(0x9, 0xA, 1, 0, 1), 0xE6A2C480, NO_SRC2, 0x01234567, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0xA, 1, 0, 1), 0x89ABCDEF, NO_SRC2, 0xF7B3D591, 0              , 0       },
    {OP(0x9, 0xA, 1, 0, 1), 0x89ABCDEF, NO_SRC2, 0xF7B3D591, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0xA, 1, 0, 1), 0xF7B3D591, NO_SRC2, 0x89ABCDEF, 0              , 0       },
    {OP(0x9, 0xA, 1, 0, 1), 0xF7B3D591, NO_SRC2, 0x89ABCDEF, PSR_MASK       , PSR_MASK},

    // REVSH r0 r1
    {OP(0x9, 0xB, 1, 0, 1), 0x01234567, NO_SRC2, 0x00006745, 0              , 0       }, // 80
    {OP(0x9, 0xB, 1, 0, 1), 0x01234567, NO_SRC2, 0x00006745, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0xB, 1, 0, 1), 0x23016745, NO_SRC2, 0x00004567, 0              , 0       },
    {OP(0x9, 0xB, 1, 0, 1), 0x23016745, NO_SRC2, 0x00004567, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0xB, 1, 0, 1), 0x01236789, NO_SRC2, 0xFFFF8967, 0              , 0       },
    {OP(0x9, 0xB, 1, 0, 1), 0x01236789, NO_SRC2, 0xFFFF8967, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0xB, 1, 0, 1), 0x23018967, NO_SRC2, 0x00006789, 0              , 0       },
    {OP(0x9, 0xB, 1, 0, 1), 0x23018967, NO_SRC2, 0x00006789, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0xB, 1, 0, 1), 0x89AB0123, NO_SRC2, 0x00002301, 0              , 0       },
    {OP(0x9, 0xB, 1, 0, 1), 0x89AB0123, NO_SRC2, 0x00002301, PSR_MASK       , PSR_MASK},
    {OP(0x9, 0xB, 1, 0, 1), 0x89ABCDEF, NO_SRC2, 0xFFFFEFCD, 0              , 0       },
    {OP(0x9, 0xB, 1, 0, 1), 0x89ABCDEF, NO_SRC2, 0xFFFFEFCD, PSR_MASK       , PSR_MASK},

    // CLZ r0 r1
    {OP(0xB, 0x8, 1, 0, 1), 0x01234567, NO_SRC2, 0x00000007, 0              , 0       }, // 92
    {OP(0xB, 0x8, 1, 0, 1), 0x00234567, NO_SRC2, 0x0000000A, PSR_MASK       , PSR_MASK},
    {OP(0xB, 0x8, 1, 0, 1), 0x00034567, NO_SRC2, 0x0000000E, 0              , 0       },
    {OP(0xB, 0x8, 1, 0, 1), 0x00004567, NO_SRC2, 0x00000011, PSR_MASK       , PSR_MASK},
    {OP(0xB, 0x8, 1, 0, 1), 0x00000567, NO_SRC2, 0x00000015, 0              , 0       },
    {OP(0xB, 0x8, 1, 0, 1), 0x00000067, NO_SRC2, 0x00000019, PSR_MASK       , PSR_MASK},
    {OP(0xB, 0x8, 1, 0, 1), 0x00000007, NO_SRC2, 0x0000001D, 0              , 0       },
    {OP(0xB, 0x8, 1, 0, 1), 0x00000000, NO_SRC2, 0x00000020, PSR_MASK       , PSR_MASK},
    {OP(0xB, 0x8, 1, 0, 1), 0x89ABCDEF, NO_SRC2, 0x00000000, 0              , 0       },
    {OP(0xB, 0x8, 1, 0, 1), 0x09ABCDEF, NO_SRC2, 0x00000004, PSR_MASK       , PSR_MASK},
    {OP(0xB, 0x8, 1, 0, 1), 0x00ABCDEF, NO_SRC2, 0x00000008, 0              , 0       },
    {OP(0xB, 0x8, 1, 0, 1), 0x000BCDEF, NO_SRC2, 0x0000000C, PSR_MASK       , PSR_MASK},
    {OP(0xB, 0x8, 1, 0, 1), 0x0000CDEF, NO_SRC2, 0x00000010, 0              , 0       },
    {OP(0xB, 0x8, 1, 0, 1), 0x00000DEF, NO_SRC2, 0x00000014, PSR_MASK       , PSR_MASK},
    {OP(0xB, 0x8, 1, 0, 1), 0x000000EF, NO_SRC2, 0x00000018, 0              , 0       },
    {OP(0xB, 0x8, 1, 0, 1), 0x0000000F, NO_SRC2, 0x0000001C, PSR_MASK       , PSR_MASK},
};

#undef OP

static const int num_dp_reg_tests = sizeof(dp_reg_tests) / sizeof(dp_reg_tests[0]);

// multiply
// accum in in psr_in
#define OP(op, op2, rn, ra, rd, rm) (0xFB000000 | op << 20 | rn << 16 | ra << 12 | rd << 8 | op2 << 4 | rm)

static const struct TestInfo32 mul_tests[] = {
    // MUL r0 r2 r1
    {OP(0, 0, 2, 15, 0, 1), 0x00000000, 0x00000000, 0x00000000, 0x01234567, 0}, // 0
    {OP(0, 0, 2, 15, 0, 1), 0x00000000, 0x00000001, 0x00000000, 0x01234567, 0}, // 0 * n
    {OP(0, 0, 2, 15, 0, 1), 0x00000001, 0x00000000, 0x00000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0xFFFFFFFF, 0x00000000, 0x00000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000001, 0x00000001, 0x00000001, 0x01234567, 0}, // 1 * n
    {OP(0, 0, 2, 15, 0, 1), 0x00000001, 0x7FFFFFFF, 0x7FFFFFFF, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFF, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000001, 0x80000000, 0x80000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x80000000, 0x00000001, 0x80000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000001, 0xFFFFFFFF, 0xFFFFFFFF, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000002, 0x30000000, 0x60000000, 0x01234567, 0}, // 2 * n
    {OP(0, 0, 2, 15, 0, 1), 0x30000000, 0x00000002, 0x60000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000002, 0x40000000, 0x80000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x40000000, 0x00000002, 0x80000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000002, 0x7FFFFFFF, 0xFFFFFFFE, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x7FFFFFFF, 0x00000002, 0xFFFFFFFE, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000002, 0x80000000, 0x00000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x80000000, 0x00000002, 0x00000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000002, 0xFFFFFFFF, 0xFFFFFFFE, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0xFFFFFFFF, 0x00000002, 0xFFFFFFFE, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x00000003, 0x30000000, 0x90000000, 0x01234567, 0}, // 3 * n
    {OP(0, 0, 2, 15, 0, 1), 0x30000000, 0x00000003, 0x90000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000001, 0x01234567, 0}, // -1 * n
    {OP(0, 0, 2, 15, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000001, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0xFFFFFFFF, 0x80000000, 0x80000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, 0x01234567, 0},
    {OP(0, 0, 2, 15, 0, 1), 0x01234567, 0x89ABCDEF, 0xC94E4629, 0x01234567, 0}, // extras
    {OP(0, 0, 2, 15, 0, 1), 0x89ABCDEF, 0x01234567, 0xC94E4629, 0x01234567, 0},

    // MLA r0 r2 r3
    {OP(0, 0, 2,  3, 0, 1), 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0}, // 31
    {OP(0, 0, 2,  3, 0, 1), 0x00000000, 0x00000001, 0x00000000, 0x00000000, 0}, // 0 * n
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000000, 0x00000001, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000000, 0x80000000, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0x00000001, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0x80000000, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000001, 0x00000001, 0x00000000, 0}, // 1 * n
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000001, 0x00000002, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000001, 0x80000000, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000001, 0x80000001, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x00000001, 0x00000000, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFF, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0x80000000, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0xFFFFFFFE, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0xFFFFFFFF, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0x7FFFFFFE, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0x80000000, 0x80000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000001, 0x80000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000001, 0x80000001, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000001, 0xFFFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000001, 0x00000000, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000001, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0x00000000, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0x7FFFFFFE, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0x7FFFFFFF, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0xFFFFFFFE, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000002, 0x30000000, 0x60000000, 0x00000000, 0}, // 2 * n
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000002, 0x60000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000002, 0x60000001, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000002, 0xDFFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000002, 0xE0000000, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000002, 0x5FFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000002, 0x40000000, 0x80000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x40000000, 0x00000002, 0x80000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x40000000, 0x00000002, 0x80000001, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x40000000, 0x00000002, 0xFFFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x40000000, 0x00000002, 0x00000000, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x40000000, 0x00000002, 0x7FFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000002, 0x7FFFFFFF, 0xFFFFFFFE, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0xFFFFFFFE, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0xFFFFFFFF, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0x7FFFFFFD, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0x7FFFFFFE, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0xFFFFFFFD, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000002, 0x80000000, 0x00000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000002, 0x00000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000002, 0x00000001, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000002, 0x7FFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000002, 0x80000000, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0x00000002, 0xFFFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000002, 0xFFFFFFFF, 0xFFFFFFFE, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0xFFFFFFFE, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0xFFFFFFFF, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0x7FFFFFFD, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0x7FFFFFFE, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0xFFFFFFFD, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x00000003, 0x30000000, 0x90000000, 0x00000000, 0}, // 3 * n
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000003, 0x90000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000003, 0x90000001, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000003, 0x0FFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000003, 0x10000000, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x30000000, 0x00000003, 0x8FFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x80000001, 0x00000000, 0}, // -1 * n
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000001, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000002, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x00000000, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x00000001, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0x80000000, 0x80000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0x80000001, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0x00000000, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000002, 0x00000001, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x80000001, 0x80000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x01234567, 0x89ABCDEF, 0xC94E4629, 0x00000000, 0}, // extras
    {OP(0, 0, 2,  3, 0, 1), 0x89ABCDEF, 0x01234567, 0xC94E4629, 0x00000000, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x89ABCDEF, 0x01234567, 0x3FA27839, 0x76543210, 0},
    {OP(0, 0, 2,  3, 0, 1), 0x89ABCDEF, 0x01234567, 0xC82B00C1, 0xFEDCBA98, 0},

    // MLS r0 r2 r3
    {OP(0, 1, 2,  3, 0, 1), 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0}, // 124
    {OP(0, 1, 2,  3, 0, 1), 0x00000000, 0x00000001, 0x00000000, 0x00000000, 0}, // 0 * n
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000000, 0x00000001, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000000, 0x80000000, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0x00000001, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0x80000000, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0},

    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000001, 0xFFFFFFFF, 0x00000000, 0}, // 1 * n
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000001, 0x00000000, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000001, 0x7FFFFFFE, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000001, 0x7FFFFFFF, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x00000001, 0xFFFFFFFE, 0xFFFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x7FFFFFFF, 0x80000001, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0x80000001, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0x80000002, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0x00000000, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0x00000001, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000001, 0x80000000, 0xFFFFFFFF, 0}, // 147 -
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0x80000000, 0x80000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000001, 0x80000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000001, 0x80000001, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000001, 0xFFFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000001, 0x00000000, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000001, 0x7FFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000001, 0xFFFFFFFF, 0x00000001, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0x00000001, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0x00000002, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0x80000000, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0x80000001, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000001, 0x00000000, 0xFFFFFFFF, 0},

    {OP(0, 1, 2,  3, 0, 1), 0x00000002, 0x30000000, 0xA0000000, 0x00000000, 0}, // 2 * n
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000002, 0xA0000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000002, 0xA0000001, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000002, 0x1FFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000002, 0x20000000, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000002, 0x9FFFFFFF, 0xFFFFFFFF, 0}, // 165 -
    {OP(0, 1, 2,  3, 0, 1), 0x00000002, 0x40000000, 0x80000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x40000000, 0x00000002, 0x80000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x40000000, 0x00000002, 0x80000001, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x40000000, 0x00000002, 0xFFFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x40000000, 0x00000002, 0x00000000, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x40000000, 0x00000002, 0x7FFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000002, 0x7FFFFFFF, 0x00000002, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0x00000002, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0x00000003, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0x80000001, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0x80000002, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0x00000002, 0x00000001, 0xFFFFFFFF, 0}, // 177 -
    {OP(0, 1, 2,  3, 0, 1), 0x00000002, 0x80000000, 0x00000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000002, 0x00000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000002, 0x00000001, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000002, 0x7FFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000002, 0x80000000, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0x00000002, 0xFFFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x00000002, 0xFFFFFFFF, 0x00000002, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0x00000002, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0x00000003, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0x80000001, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0x80000002, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x00000002, 0x00000001, 0xFFFFFFFF, 0},

    {OP(0, 1, 2,  3, 0, 1), 0x00000003, 0x30000000, 0x70000000, 0x00000000, 0}, // 3 * n
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000003, 0x70000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000003, 0x70000001, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000003, 0xEFFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000003, 0xF0000000, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x30000000, 0x00000003, 0x6FFFFFFF, 0xFFFFFFFF, 0},

    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000, 0}, // -1 * n
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x7FFFFFFF, 0xFFFFFFFF, 0x7FFFFFFE, 0xFFFFFFFF, 0}, // 201 -
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0x80000000, 0x80000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0x80000000, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0x80000001, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0x00000000, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000001, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFE, 0x7FFFFFFF, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFF, 0},

    {OP(0, 1, 2,  3, 0, 1), 0x01234567, 0x89ABCDEF, 0x36B1B9D7, 0x00000000, 0}, // extras
    {OP(0, 1, 2,  3, 0, 1), 0x89ABCDEF, 0x01234567, 0x36B1B9D7, 0x00000000, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x89ABCDEF, 0x01234567, 0xAD05EBE7, 0x76543210, 0},
    {OP(0, 1, 2,  3, 0, 1), 0x89ABCDEF, 0x01234567, 0x358E746F, 0xFEDCBA98, 0},
};

#undef OP

static const int num_mul_tests = sizeof(mul_tests) / sizeof(mul_tests[0]);

// these set no flags and may have three sources
static bool run_thumb2_mul_div_tests(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo32 *tests, int num_tests, const char *label, int dest) {

    bool res = true;

    group_cb(label);

    uint32_t psr_save = get_cpsr_arm() & ~PSR_MASK;

    int set_cpsr_off = (uintptr_t)set_cpsr - ((uintptr_t)code_buf + 6);

    for(int i = 0; i < num_tests; i++) {
        const struct TestInfo32 *test = &tests[i];

        // value test
        uint16_t *ptr = code_buf;

        *ptr++ = test->opcode >> 16;
        *ptr++ = test->opcode;

        if(dest != 0) // assume < 8
            *ptr++ = dest << 3; // mov r0 rN
        
        *ptr++ = 0x4770; // BX LR

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);
        invalidate_icache();

        // psr_in used as accum in
        uint32_t out = func(0xDAB00BAD, test->m_in, test->n_in, test->psr_in);

        if(out != test->d_out) {
            res = false;
            fail_cb(i, out, test->d_out);
        }


        // flags test (make sure flags are unmodified)
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
        out = func(PSR_VCZN | psr_save, test->m_in, test->n_in, (intptr_t)get_cpsr_arm);
        out &= PSR_MASK;

        if(out != PSR_VCZN) {
            res = false;
            fail_cb(i, out, PSR_VCZN);
        }
    }

    return res;
}

bool run_thumb2_data_processing_tests(GroupCallback group_cb, FailCallback fail_cb) {
    bool ret = true;

    ret = run_thumb2_test_list(group_cb, fail_cb, dp_mod_imm_tests, num_dp_mod_imm_tests, "dp.modimm", 0, false, true) && ret;
    // ADC/SBC in separate group to set C in
    ret = run_thumb2_test_list(group_cb, fail_cb, dp_mod_imm_c_tests, num_dp_mod_imm_c_tests, "dp.c.modimm", 0, true, true) && ret;

    ret = run_thumb2_test_list(group_cb, fail_cb, dp_plain_imm_tests, num_dp_plain_imm_tests, "dp.plimm", 0, false, true) && ret;

    ret = run_thumb2_test_list(group_cb, fail_cb, dp_shift_reg_tests, num_dp_shift_reg_tests, "dp.shreg", 0, false, true) && ret;
    // ADC/SBC/RRX in separate group to set C in
    ret = run_thumb2_test_list(group_cb, fail_cb, dp_shift_reg_c_tests, num_dp_shift_reg_c_tests, "dp.c.shreg", 0, true, true) && ret;

    ret = run_thumb2_test_list(group_cb, fail_cb, dp_reg_shift_tests, num_dp_reg_shift_tests, "dp.reg.sh", 0, false, true) && ret;
    ret = run_thumb2_test_list(group_cb, fail_cb, dp_reg_tests, num_dp_reg_tests, "dp.reg", 0, false, false) && ret;

    ret = run_thumb2_mul_div_tests(group_cb, fail_cb, mul_tests, num_mul_tests, "mul", 0) && ret;

    return ret;
}
#endif