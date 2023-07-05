#include "tests.h"

typedef uint32_t (*TestFunc)(uint32_t, uint32_t, uint32_t, uint32_t);

static _Alignas(4) uint16_t code_buf[16];

#define FLAG_V (1 << 28)
#define FLAG_C (1 << 29)
#define FLAG_Z (1 << 30)
#define FLAG_N (1 << 31)

#define PSR_MASK 0xF0000000

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

bool run_test_list(GroupCallback group_cb, FailCallback fail_cb, const struct TestInfo *tests, int num_tests, const char *label) {

    bool res = true;

    group_cb(label);

    uint32_t psr_save = get_cpsr_arm() & ~PSR_MASK;

    for(int i = 0; i < num_tests; i++) {
        const struct TestInfo *test = &tests[i];
    
        // value test
        code_buf[0] = test->opcode;
        code_buf[1] = 0x4770; // BX LR

        TestFunc func = (TestFunc)((uintptr_t)code_buf | 1);

        uint32_t out = func(0xBAD, test->m_in, test->n_in, 0x3BAD);

        if(out != test->d_out) {
            res = false;
            fail_cb(i, out, test->d_out);
        }

        // flags test
        // this relies on the PSR helpers not affecting anything other than R0
        int off = (uintptr_t)set_cpsr - ((uintptr_t)code_buf + 6);

        code_buf[0] = 0xB500; // push lr
        code_buf[1] = 0xF000 | ((off >> 12) & 0x7FF); // bl set_cpsr
        code_buf[2] = 0xF800 | ((off >> 1) & 0x7FF); // bl set_cpsr

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
                fail_cb(i, out, test->psr_out & flags[j]);
            }
        }

    }

    return res;
}

bool run_tests(GroupCallback group_cb, FailCallback fail_cb) {
    
    bool ret = true;

    ret = run_test_list(group_cb, fail_cb, shift_imm_tests, num_shift_imm_tests, "sh.imm") && ret;
    ret = run_test_list(group_cb, fail_cb, add_sub_tests, num_add_sub_tests, "addsub") && ret;

    return ret;
}