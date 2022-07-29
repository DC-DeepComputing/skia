/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorPriv.h"
#include "include/core/SkStream.h"
#include "include/private/SkColorData.h"
#include "src/core/SkCpu.h"
#include "src/core/SkMSAN.h"
#include "src/core/SkVM.h"
#include "tests/Test.h"

template <typename Fn>
static void test_interpreter(const skvm::Builder& b, Fn&& test) {
    skvm::Program p = b.done();
    test(p);
}

DEF_TEST(SkVM_eliminate_dead_code, r) {
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<int>();
        skvm::I32 l = b.load32(arg);
        skvm::I32 a = b.add(l, l);
        b.add(a, b.splat(7));
    }

    std::vector<skvm::Instruction> program = b.program();
    REPORTER_ASSERT(r, program.size() == 4);

    program = skvm::eliminate_dead_code(program);
    REPORTER_ASSERT(r, program.size() == 0);
}

DEF_TEST(SkVM_Pointless, r) {
    // Let's build a program with no memory arguments.
    // It should all be pegged as dead code, but we should be able to "run" it.
    skvm::Builder b;
    {
        b.add(b.splat(5.0f),
              b.splat(4.0f));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        for (int N = 0; N < 64; N++) {
            program.eval(N);
        }
    });

    for (const skvm::OptimizedInstruction& inst : b.optimize()) {
        REPORTER_ASSERT(r, inst.death == 0 && inst.can_hoist == true);
    }
}

DEF_TEST(SkVM_memset, r) {
    skvm::Builder b;
    b.store32(b.varying<int>(), b.splat(42));

    test_interpreter(b, [&](const skvm::Program& p) {
        int buf[18];
        buf[17] = 47;

        p.eval(17, buf);
        for (int i = 0; i < 17; i++) {
            REPORTER_ASSERT(r, buf[i] == 42);
        }
        REPORTER_ASSERT(r, buf[17] == 47);
    });
}

DEF_TEST(SkVM_memcpy, r) {
    skvm::Builder b;
    {
        auto src = b.varying<int>(),
             dst = b.varying<int>();
        b.store32(dst, b.load32(src));
    }

    test_interpreter(b, [&](const skvm::Program& p) {
        int src[] = {1,2,3,4,5,6,7,8,9},
            dst[] = {0,0,0,0,0,0,0,0,0};

        p.eval(std::size(src)-1, src, dst);
        for (size_t i = 0; i < std::size(src)-1; i++) {
            REPORTER_ASSERT(r, dst[i] == src[i]);
        }
        size_t i = std::size(src)-1;
        REPORTER_ASSERT(r, dst[i] == 0);
    });
}

DEF_TEST(SkVM_LoopCounts, r) {
    // Make sure we cover all the exact N we want.

    // buf[i] += 1
    skvm::Builder b;
    skvm::Ptr arg = b.varying<int>();
    b.store32(arg,
              b.add(b.splat(1),
                    b.load32(arg)));

    test_interpreter(b, [&](const skvm::Program& program) {
        int buf[64];
        for (int N = 0; N <= (int)std::size(buf); N++) {
            for (int i = 0; i < (int)std::size(buf); i++) {
                buf[i] = i;
            }
            program.eval(N, buf);

            for (int i = 0; i < N; i++) {
                REPORTER_ASSERT(r, buf[i] == i+1);
            }
            for (int i = N; i < (int)std::size(buf); i++) {
                REPORTER_ASSERT(r, buf[i] == i);
            }
        }
    });
}

DEF_TEST(SkVM_gather32, r) {
    skvm::Builder b;
    {
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf = b.varying<int>();
        skvm::I32 x = b.load32(buf);
        b.store32(buf, b.gather32(uniforms,0, b.bit_and(x, b.splat(7))));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        const int img[] = {12,34,56,78, 90,98,76,54};

        int buf[20];
        for (int i = 0; i < 20; i++) {
            buf[i] = i;
        }

        struct Uniforms {
            const int* img;
        } uniforms{img};

        program.eval(20, &uniforms, buf);
        int i = 0;
        REPORTER_ASSERT(r, buf[i] == 12); i++;
        REPORTER_ASSERT(r, buf[i] == 34); i++;
        REPORTER_ASSERT(r, buf[i] == 56); i++;
        REPORTER_ASSERT(r, buf[i] == 78); i++;
        REPORTER_ASSERT(r, buf[i] == 90); i++;
        REPORTER_ASSERT(r, buf[i] == 98); i++;
        REPORTER_ASSERT(r, buf[i] == 76); i++;
        REPORTER_ASSERT(r, buf[i] == 54); i++;

        REPORTER_ASSERT(r, buf[i] == 12); i++;
        REPORTER_ASSERT(r, buf[i] == 34); i++;
        REPORTER_ASSERT(r, buf[i] == 56); i++;
        REPORTER_ASSERT(r, buf[i] == 78); i++;
        REPORTER_ASSERT(r, buf[i] == 90); i++;
        REPORTER_ASSERT(r, buf[i] == 98); i++;
        REPORTER_ASSERT(r, buf[i] == 76); i++;
        REPORTER_ASSERT(r, buf[i] == 54); i++;

        REPORTER_ASSERT(r, buf[i] == 12); i++;
        REPORTER_ASSERT(r, buf[i] == 34); i++;
        REPORTER_ASSERT(r, buf[i] == 56); i++;
        REPORTER_ASSERT(r, buf[i] == 78); i++;
    });
}

DEF_TEST(SkVM_gathers, r) {
    skvm::Builder b;
    {
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf32    = b.varying<int>(),
                  buf16    = b.varying<uint16_t>(),
                  buf8     = b.varying<uint8_t>();

        skvm::I32 x = b.load32(buf32);

        b.store32(buf32, b.gather32(uniforms,0, b.bit_and(x, b.splat( 7))));
        b.store16(buf16, b.gather16(uniforms,0, b.bit_and(x, b.splat(15))));
        b.store8 (buf8 , b.gather8 (uniforms,0, b.bit_and(x, b.splat(31))));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        const int img[] = {12,34,56,78, 90,98,76,54};

        constexpr int N = 20;
        int      buf32[N];
        uint16_t buf16[N];
        uint8_t  buf8 [N];

        for (int i = 0; i < 20; i++) {
            buf32[i] = i;
        }

        struct Uniforms {
            const int* img;
        } uniforms{img};

        program.eval(N, &uniforms, buf32, buf16, buf8);
        int i = 0;
        REPORTER_ASSERT(r, buf32[i] == 12 && buf16[i] == 12 && buf8[i] == 12); i++;
        REPORTER_ASSERT(r, buf32[i] == 34 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 56 && buf16[i] == 34 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 78 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 90 && buf16[i] == 56 && buf8[i] == 34); i++;
        REPORTER_ASSERT(r, buf32[i] == 98 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 76 && buf16[i] == 78 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 54 && buf16[i] ==  0 && buf8[i] ==  0); i++;

        REPORTER_ASSERT(r, buf32[i] == 12 && buf16[i] == 90 && buf8[i] == 56); i++;
        REPORTER_ASSERT(r, buf32[i] == 34 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 56 && buf16[i] == 98 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 78 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 90 && buf16[i] == 76 && buf8[i] == 78); i++;
        REPORTER_ASSERT(r, buf32[i] == 98 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 76 && buf16[i] == 54 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 54 && buf16[i] ==  0 && buf8[i] ==  0); i++;

        REPORTER_ASSERT(r, buf32[i] == 12 && buf16[i] == 12 && buf8[i] == 90); i++;
        REPORTER_ASSERT(r, buf32[i] == 34 && buf16[i] ==  0 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 56 && buf16[i] == 34 && buf8[i] ==  0); i++;
        REPORTER_ASSERT(r, buf32[i] == 78 && buf16[i] ==  0 && buf8[i] ==  0); i++;
    });
}

DEF_TEST(SkVM_gathers2, r) {
    skvm::Builder b;
    {
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf32    = b.varying<int>(),
                  buf16    = b.varying<uint16_t>(),
                  buf8     = b.varying<uint8_t>();

        skvm::I32 x = b.load32(buf32);

        b.store32(buf32, b.gather32(uniforms,0, x));
        b.store16(buf16, b.gather16(uniforms,0, x));
        b.store8 (buf8 , b.gather8 (uniforms,0, x));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        uint8_t img[256];
        for (int i = 0; i < 256; i++) {
            img[i] = i;
        }

        int      buf32[64];
        uint16_t buf16[64];
        uint8_t  buf8 [64];

        for (int i = 0; i < 64; i++) {
            buf32[i] = (i*47)&63;
            buf16[i] = 0;
            buf8 [i] = 0;
        }

        struct Uniforms {
            const uint8_t* img;
        } uniforms{img};

        program.eval(64, &uniforms, buf32, buf16, buf8);

        for (int i = 0; i < 64; i++) {
            REPORTER_ASSERT(r, buf8[i] == ((i*47)&63));  // 0,47,30,13,60,...
        }

        REPORTER_ASSERT(r, buf16[ 0] == 0x0100);
        REPORTER_ASSERT(r, buf16[63] == 0x2322);

        REPORTER_ASSERT(r, buf32[ 0] == 0x03020100);
        REPORTER_ASSERT(r, buf32[63] == 0x47464544);
    });
}

DEF_TEST(SkVM_bitops, r) {
    skvm::Builder b;
    {
        skvm::Ptr ptr = b.varying<int>();

        skvm::I32 x = b.load32(ptr);

        x = b.bit_and  (x, b.splat(0xf1));  // 0x40
        x = b.bit_or   (x, b.splat(0x80));  // 0xc0
        x = b.bit_xor  (x, b.splat(0xfe));  // 0x3e
        x = b.bit_clear(x, b.splat(0x30));  // 0x0e

        x = b.shl(x, 28);  // 0xe000'0000
        x = b.sra(x, 28);  // 0xffff'fffe
        x = b.shr(x,  1);  // 0x7fff'ffff

        b.store32(ptr, x);
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        int x = 0x42;
        program.eval(1, &x);
        REPORTER_ASSERT(r, x == 0x7fff'ffff);
    });
}

DEF_TEST(SkVM_select_is_NaN, r) {
    skvm::Builder b;
    {
        skvm::Ptr src = b.varying<float>(),
                  dst = b.varying<float>();

        skvm::F32 x = b.loadF(src);
        x = select(is_NaN(x), b.splat(0.0f)
                            , x);
        b.storeF(dst, x);
    }

    std::vector<skvm::OptimizedInstruction> program = b.optimize();
    REPORTER_ASSERT(r, program.size() == 4);
    REPORTER_ASSERT(r, program[0].op == skvm::Op::load32);
    REPORTER_ASSERT(r, program[1].op == skvm::Op::neq_f32);
    REPORTER_ASSERT(r, program[2].op == skvm::Op::bit_clear);
    REPORTER_ASSERT(r, program[3].op == skvm::Op::store32);

    test_interpreter(b, [&](const skvm::Program& program) {
        // ±NaN, ±0, ±1, ±inf
        uint32_t src[] = {0x7f80'0001, 0xff80'0001, 0x0000'0000, 0x8000'0000,
                          0x3f80'0000, 0xbf80'0000, 0x7f80'0000, 0xff80'0000};
        uint32_t dst[std::size(src)];
        program.eval(std::size(src), src, dst);

        for (int i = 0; i < (int)std::size(src); i++) {
            REPORTER_ASSERT(r, dst[i] == (i < 2 ? 0 : src[i]));
        }
    });
}

DEF_TEST(SkVM_f32, r) {
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<float>();

        skvm::F32 x = b.loadF(arg),
                  y = b.add(x,x),   // y = 2x
                  z = b.sub(y,x),   // z = 2x-x = x
                  w = b.div(z,x);   // w = x/x = 1
        b.storeF(arg, w);
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        float buf[] = { 1,2,3,4,5,6,7,8,9 };
        program.eval(std::size(buf), buf);
        for (float v : buf) {
            REPORTER_ASSERT(r, v == 1.0f);
        }
    });
}

DEF_TEST(SkVM_cmp_i32, r) {
    skvm::Builder b;
    {
        skvm::I32 x = b.load32(b.varying<int>());

        auto to_bit = [&](int shift, skvm::I32 mask) {
            return b.shl(b.bit_and(mask, b.splat(0x1)), shift);
        };

        skvm::I32 m = b.splat(0);
        m = b.bit_or(m, to_bit(0, b. eq(x, b.splat(0))));
        m = b.bit_or(m, to_bit(1, b.neq(x, b.splat(1))));
        m = b.bit_or(m, to_bit(2, b. lt(x, b.splat(2))));
        m = b.bit_or(m, to_bit(3, b.lte(x, b.splat(3))));
        m = b.bit_or(m, to_bit(4, b. gt(x, b.splat(4))));
        m = b.bit_or(m, to_bit(5, b.gte(x, b.splat(5))));

        b.store32(b.varying<int>(), m);
    }
    test_interpreter(b, [&](const skvm::Program& program) {
        int in[] = { 0,1,2,3,4,5,6,7,8,9 };
        int out[std::size(in)];

        program.eval(std::size(in), in, out);

        REPORTER_ASSERT(r, out[0] == 0b001111);
        REPORTER_ASSERT(r, out[1] == 0b001100);
        REPORTER_ASSERT(r, out[2] == 0b001010);
        REPORTER_ASSERT(r, out[3] == 0b001010);
        REPORTER_ASSERT(r, out[4] == 0b000010);
        for (int i = 5; i < (int)std::size(out); i++) {
            REPORTER_ASSERT(r, out[i] == 0b110010);
        }
    });
}

DEF_TEST(SkVM_cmp_f32, r) {
    skvm::Builder b;
    {
        skvm::F32 x = b.loadF(b.varying<float>());

        auto to_bit = [&](int shift, skvm::I32 mask) {
            return b.shl(b.bit_and(mask, b.splat(0x1)), shift);
        };

        skvm::I32 m = b.splat(0);
        m = b.bit_or(m, to_bit(0, b. eq(x, b.splat(0.0f))));
        m = b.bit_or(m, to_bit(1, b.neq(x, b.splat(1.0f))));
        m = b.bit_or(m, to_bit(2, b. lt(x, b.splat(2.0f))));
        m = b.bit_or(m, to_bit(3, b.lte(x, b.splat(3.0f))));
        m = b.bit_or(m, to_bit(4, b. gt(x, b.splat(4.0f))));
        m = b.bit_or(m, to_bit(5, b.gte(x, b.splat(5.0f))));

        b.store32(b.varying<int>(), m);
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        float in[] = { 0,1,2,3,4,5,6,7,8,9 };
        int out[std::size(in)];

        program.eval(std::size(in), in, out);

        REPORTER_ASSERT(r, out[0] == 0b001111);
        REPORTER_ASSERT(r, out[1] == 0b001100);
        REPORTER_ASSERT(r, out[2] == 0b001010);
        REPORTER_ASSERT(r, out[3] == 0b001010);
        REPORTER_ASSERT(r, out[4] == 0b000010);
        for (int i = 5; i < (int)std::size(out); i++) {
            REPORTER_ASSERT(r, out[i] == 0b110010);
        }
    });
}

DEF_TEST(SkVM_index, r) {
    skvm::Builder b;
    b.store32(b.varying<int>(), b.index());

    test_interpreter(b, [&](const skvm::Program& program) {
        int buf[23];
        program.eval(std::size(buf), buf);
        for (int i = 0; i < (int)std::size(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == (int)std::size(buf)-i);
        }
    });
}

DEF_TEST(SkVM_mad, r) {
    // This program is designed to exercise the tricky corners of instruction
    // and register selection for Op::mad_f32.

    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<int>();

        skvm::F32 x = b.to_F32(b.load32(arg)),
                  y = b.mad(x,x,x),   // x is needed in the future, so r[x] != r[y].
                  z = b.mad(y,y,x),   // y is needed in the future, but r[z] = r[x] is ok.
                  w = b.mad(z,z,y),   // w can alias z but not y.
                  v = b.mad(w,y,w);   // Got to stop somewhere.
        b.store32(arg, b.trunc(v));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        int x = 2;
        program.eval(1, &x);
        // x = 2
        // y = 2*2 + 2 = 6
        // z = 6*6 + 2 = 38
        // w = 38*38 + 6 = 1450
        // v = 1450*6 + 1450 = 10150
        REPORTER_ASSERT(r, x == 10150);
    });
}

DEF_TEST(SkVM_fms, r) {
    // Create a pattern that can be peepholed into an Op::fms_f32.
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<int>();

        skvm::F32 x = b.to_F32(b.load32(arg)),
                  v = b.sub(b.mul(x, b.splat(2.0f)),
                            b.splat(1.0f));
        b.store32(arg, b.trunc(v));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        int buf[] = {0,1,2,3,4,5,6,7,8,9,10};
        program.eval((int)std::size(buf), &buf);

        for (int i = 0; i < (int)std::size(buf); i++) {
            REPORTER_ASSERT(r, buf[i] = 2*i-1);
        }
    });
}

DEF_TEST(SkVM_fnma, r) {
    // Create a pattern that can be peepholed into an Op::fnma_f32.
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<int>();

        skvm::F32 x = b.to_F32(b.load32(arg)),
                  v = b.sub(b.splat(1.0f),
                            b.mul(x, b.splat(2.0f)));
        b.store32(arg, b.trunc(v));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        int buf[] = {0,1,2,3,4,5,6,7,8,9,10};
        program.eval((int)std::size(buf), &buf);

        for (int i = 0; i < (int)std::size(buf); i++) {
            REPORTER_ASSERT(r, buf[i] = 1-2*i);
        }
    });
}

DEF_TEST(SkVM_madder, r) {
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<float>();

        skvm::F32 x = b.loadF(arg),
                  y = b.mad(x,x,x),   // x is needed in the future, so r[x] != r[y].
                  z = b.mad(y,x,y),   // r[x] can be reused after this instruction, but not r[y].
                  w = b.mad(y,y,z);
        b.storeF(arg, w);
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        float x = 2.0f;
        // y = 2*2 + 2 = 6
        // z = 6*2 + 6 = 18
        // w = 6*6 + 18 = 54
        program.eval(1, &x);
        REPORTER_ASSERT(r, x == 54.0f);
    });
}

DEF_TEST(SkVM_floor, r) {
    skvm::Builder b;
    {
        skvm::Ptr arg = b.varying<float>();
        b.storeF(arg, b.floor(b.loadF(arg)));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        float buf[]  = { -2.0f, -1.5f, -1.0f, 0.0f, 1.0f, 1.5f, 2.0f };
        float want[] = { -2.0f, -2.0f, -1.0f, 0.0f, 1.0f, 1.0f, 2.0f };
        program.eval(std::size(buf), buf);
        for (int i = 0; i < (int)std::size(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == want[i]);
        }
    });
}

DEF_TEST(SkVM_round, r) {
    skvm::Builder b;
    {
        skvm::Ptr src = b.varying<float>();
        skvm::Ptr dst = b.varying<int>();
        b.store32(dst, b.round(b.loadF(src)));
    }

    // The test cases on exact 0.5f boundaries assume the current rounding mode is nearest even.
    // We haven't explicitly guaranteed that here... it just probably is.
    test_interpreter(b, [&](const skvm::Program& program) {
        float buf[]  = { -1.5f, -0.5f, 0.0f, 0.5f, 0.2f, 0.6f, 1.0f, 1.4f, 1.5f, 2.0f };
        int want[] =   { -2   ,  0   , 0   , 0   , 0   , 1   , 1   , 1   , 2   , 2    };
        int dst[std::size(buf)];

        program.eval(std::size(buf), buf, dst);
        for (int i = 0; i < (int)std::size(dst); i++) {
            REPORTER_ASSERT(r, dst[i] == want[i]);
        }
    });
}

DEF_TEST(SkVM_min, r) {
    skvm::Builder b;
    {
        skvm::Ptr src1 = b.varying<float>();
        skvm::Ptr src2 = b.varying<float>();
        skvm::Ptr dst = b.varying<float>();

        b.storeF(dst, b.min(b.loadF(src1), b.loadF(src2)));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        float s1[]  =  { 0.0f, 1.0f, 4.0f, -1.0f, -1.0f};
        float s2[]  =  { 0.0f, 2.0f, 3.0f,  1.0f, -2.0f};
        float want[] = { 0.0f, 1.0f, 3.0f, -1.0f, -2.0f};
        float d[std::size(s1)];
        program.eval(std::size(d), s1, s2, d);
        for (int i = 0; i < (int)std::size(d); i++) {
          REPORTER_ASSERT(r, d[i] == want[i]);
        }
    });
}

DEF_TEST(SkVM_max, r) {
    skvm::Builder b;
    {
        skvm::Ptr src1 = b.varying<float>();
        skvm::Ptr src2 = b.varying<float>();
        skvm::Ptr dst = b.varying<float>();

        b.storeF(dst, b.max(b.loadF(src1), b.loadF(src2)));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        float s1[]  =  { 0.0f, 1.0f, 4.0f, -1.0f, -1.0f};
        float s2[]  =  { 0.0f, 2.0f, 3.0f,  1.0f, -2.0f};
        float want[] = { 0.0f, 2.0f, 4.0f,  1.0f, -1.0f};
        float d[std::size(s1)];
        program.eval(std::size(d), s1, s2, d);
        for (int i = 0; i < (int)std::size(d); i++) {
          REPORTER_ASSERT(r, d[i] == want[i]);
        }
    });
}

DEF_TEST(SkVM_select, r) {
    skvm::Builder b;
    {
        skvm::Ptr buf = b.varying<int>();

        skvm::I32 x = b.load32(buf);

        x = b.select( b.gt(x, b.splat(4)), x, b.splat(42) );

        b.store32(buf, x);
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        int buf[] = { 0,1,2,3,4,5,6,7,8 };
        program.eval(std::size(buf), buf);
        for (int i = 0; i < (int)std::size(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == (i > 4 ? i : 42));
        }
    });
}

DEF_TEST(SkVM_swap, r) {
    skvm::Builder b;
    {
        // This program is the equivalent of
        //     x = *X
        //     y = *Y
        //     *X = y
        //     *Y = x
        // One rescheduling of the program based only on data flow of Op arguments is
        //     x = *X
        //     *Y = x
        //     y = *Y
        //     *X = y
        // but this reordering does not produce the same results and is invalid.
        skvm::Ptr X = b.varying<int>(),
                  Y = b.varying<int>();

        skvm::I32 x = b.load32(X),
                  y = b.load32(Y);

        b.store32(X, y);
        b.store32(Y, x);
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        int b1[] = { 0,1,2,3 };
        int b2[] = { 4,5,6,7 };
        program.eval(std::size(b1), b1, b2);
        for (int i = 0; i < (int)std::size(b1); i++) {
            REPORTER_ASSERT(r, b1[i] == 4 + i);
            REPORTER_ASSERT(r, b2[i] == i);
        }
    });
}

DEF_TEST(SkVM_NewOps, r) {
    // Exercise a somewhat arbitrary set of new ops.
    skvm::Builder b;
    {
        skvm::Ptr buf = b.varying<int16_t>();
        skvm::UPtr uniforms = b.uniform();

        skvm::I32 x = b.load16(buf);

        const size_t kPtr = sizeof(const int*);

        x = b.add(x, b.uniform32(uniforms, kPtr+0));
        x = b.mul(x, b.uniform32(uniforms, kPtr+4));
        x = b.sub(x, b.uniform32(uniforms, kPtr+8));

        skvm::I32 limit = b.uniform32(uniforms, kPtr+12);
        x = b.select(b.lt(x, b.splat(0)), b.splat(0), x);
        x = b.select(b.gt(x, limit     ), limit     , x);

        x = b.gather8(uniforms,0, x);

        b.store16(buf, x);
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        const int N = 31;
        int16_t buf[N];
        for (int i = 0; i < N; i++) {
            buf[i] = i;
        }

        const int M = 16;
        uint8_t img[M];
        for (int i = 0; i < M; i++) {
            img[i] = i*i;
        }

        struct {
            const uint8_t* img;
            int      add   = 5;
            int      mul   = 3;
            int      sub   = 18;
            int      limit = M-1;
        } uniforms{img};

        program.eval(N, buf, &uniforms);

        for (int i = 0; i < N; i++) {
            // Our first math calculates x = (i+5)*3 - 18 a.k.a 3*(i-1).
            int x = 3*(i-1);

            // Then that's pinned to the limits of img.
            if (i < 2) { x =  0; }  // Notice i == 1 hits x == 0 exactly...
            if (i > 5) { x = 15; }  // ...and i == 6 hits x == 15 exactly
            REPORTER_ASSERT(r, buf[i] == img[x]);
        }
    });
}

DEF_TEST(SKVM_array32, r) {



    skvm::Builder b;
    skvm::Uniforms uniforms(b.uniform(), 0);
    // Take up the first slot, so other uniforms are not at 0 offset.
    uniforms.push(0);
    int i[] = {3, 7};
    skvm::Uniform array = uniforms.pushArray(i);
    float f[] = {5, 9};
    skvm::Uniform arrayF = uniforms.pushArrayF(f);
    {
        skvm::Ptr buf0     = b.varying<int32_t>(),
                  buf1     = b.varying<int32_t>(),
                  buf2     = b.varying<int32_t>();

        skvm::I32 j = b.array32(array, 0);
        b.store32(buf0, j);
        skvm::I32 k = b.array32(array, 1);
        b.store32(buf1, k);

        skvm::F32 x = b.arrayF(arrayF, 0);
        skvm::F32 y = b.arrayF(arrayF, 1);
        b.store32(buf2, b.trunc(b.add(x, y)));
    }

    test_interpreter(b, [&](const skvm::Program& program) {
        const int K = 10;
        int32_t buf0[K],
                buf1[K],
                buf2[K];

        // reset the i[0] for the two tests.
        i[0] = 3;
        f[1] = 9;
        program.eval(K, uniforms.buf.data(), buf0, buf1, buf2);
        for (auto v : buf0) {
            REPORTER_ASSERT(r, v == 3);
        }
        for (auto v : buf1) {
            REPORTER_ASSERT(r, v == 7);
        }
        for (auto v : buf2) {
            REPORTER_ASSERT(r, v == 14);
        }
        i[0] = 4;
        f[1] = 10;
        program.eval(K, uniforms.buf.data(), buf0, buf1, buf2);
        for (auto v : buf0) {
            REPORTER_ASSERT(r, v == 4);
        }
        for (auto v : buf1) {
            REPORTER_ASSERT(r, v == 7);
        }
        for (auto v : buf2) {
            REPORTER_ASSERT(r, v == 15);
        }
    });
}

DEF_TEST(SkVM_sqrt, r) {
    skvm::Builder b;
    auto buf = b.varying<int>();
    b.storeF(buf, b.sqrt(b.loadF(buf)));

    test_interpreter(b, [&](const skvm::Program& program) {
        constexpr int K = 17;
        float buf[K];
        for (int i = 0; i < K; i++) {
            buf[i] = (float)(i*i);
        }

        // x^2 -> x
        program.eval(K, buf);

        for (int i = 0; i < K; i++) {
            REPORTER_ASSERT(r, buf[i] == (float)i);
        }
    });
}

DEF_TEST(SkVM_assert, r) {
    skvm::Builder b;
    b.assert_true(b.lt(b.load32(b.varying<int>()),
                       b.splat(42)));

    test_interpreter(b, [&](const skvm::Program& program) {
        int buf[] = { 0,1,2,3,4,5,6,7,8,9 };
        program.eval(std::size(buf), buf);
    });
}

DEF_TEST(SkVM_trace_line, r) {
    class TestTraceHook : public skvm::TraceHook {
    public:
        void var(int, int32_t) override { fBuffer.push_back(-9999999); }
        void enter(int) override        { fBuffer.push_back(-9999999); }
        void exit(int) override         { fBuffer.push_back(-9999999); }
        void scope(int) override        { fBuffer.push_back(-9999999); }
        void line(int lineNum) override { fBuffer.push_back(lineNum); }

        std::vector<int> fBuffer;
    };

    skvm::Builder b;
    TestTraceHook testTrace;
    int traceHookID = b.attachTraceHook(&testTrace);
    b.trace_line(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 123);
    b.trace_line(traceHookID, b.splat(0x00000000), b.splat(0xFFFFFFFF), 456);
    b.trace_line(traceHookID, b.splat(0xFFFFFFFF), b.splat(0x00000000), 567);
    b.trace_line(traceHookID, b.splat(0x00000000), b.splat(0x00000000), 678);
    b.trace_line(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 789);
    skvm::Program p = b.done();
    p.eval(1);

    REPORTER_ASSERT(r, (testTrace.fBuffer == std::vector<int>{123, 789}));
}

DEF_TEST(SkVM_trace_var, r) {
    class TestTraceHook : public skvm::TraceHook {
    public:
        void line(int) override                  { fBuffer.push_back(-9999999); }
        void enter(int) override                 { fBuffer.push_back(-9999999); }
        void exit(int) override                  { fBuffer.push_back(-9999999); }
        void scope(int) override                 { fBuffer.push_back(-9999999); }
        void var(int slot, int32_t val) override {
            fBuffer.push_back(slot);
            fBuffer.push_back(val);
        }

        std::vector<int> fBuffer;
    };

    skvm::Builder b;
    TestTraceHook testTrace;
    int traceHookID = b.attachTraceHook(&testTrace);
    b.trace_var(traceHookID, b.splat(0x00000000), b.splat(0xFFFFFFFF), 2, b.splat(333));
    b.trace_var(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 4, b.splat(555));
    b.trace_var(traceHookID, b.splat(0x00000000), b.splat(0x00000000), 5, b.splat(666));
    b.trace_var(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 6, b.splat(777));
    b.trace_var(traceHookID, b.splat(0xFFFFFFFF), b.splat(0x00000000), 8, b.splat(999));
    skvm::Program p = b.done();
    p.eval(1);

    REPORTER_ASSERT(r, (testTrace.fBuffer == std::vector<int>{4, 555, 6, 777}));
}

DEF_TEST(SkVM_trace_enter_exit, r) {
    class TestTraceHook : public skvm::TraceHook {
    public:
        void line(int) override                   { fBuffer.push_back(-9999999); }
        void var(int, int32_t) override           { fBuffer.push_back(-9999999); }
        void scope(int) override                  { fBuffer.push_back(-9999999); }
        void enter(int fnIdx) override {
            fBuffer.push_back(fnIdx);
            fBuffer.push_back(1);
        }
        void exit(int fnIdx) override {
            fBuffer.push_back(fnIdx);
            fBuffer.push_back(0);
        }

        std::vector<int> fBuffer;
    };

    skvm::Builder b;
    TestTraceHook testTrace;
    int traceHookID = b.attachTraceHook(&testTrace);
    b.trace_enter(traceHookID, b.splat(0x00000000), b.splat(0x00000000), 99);
    b.trace_enter(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 12);
    b.trace_enter(traceHookID, b.splat(0x00000000), b.splat(0xFFFFFFFF), 34);
    b.trace_exit(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 56);
    b.trace_exit(traceHookID, b.splat(0xFFFFFFFF), b.splat(0x00000000), 78);
    b.trace_exit(traceHookID, b.splat(0x00000000), b.splat(0x00000000), 90);
    skvm::Program p = b.done();
    p.eval(1);

    REPORTER_ASSERT(r, (testTrace.fBuffer == std::vector<int>{12, 1, 56, 0}));
}

DEF_TEST(SkVM_trace_scope, r) {
    class TestTraceHook : public skvm::TraceHook {
    public:
        void var(int, int32_t) override { fBuffer.push_back(-9999999); }
        void enter(int) override        { fBuffer.push_back(-9999999); }
        void exit(int) override         { fBuffer.push_back(-9999999); }
        void line(int) override         { fBuffer.push_back(-9999999); }
        void scope(int delta) override  { fBuffer.push_back(delta); }

        std::vector<int> fBuffer;
    };

    skvm::Builder b;
    TestTraceHook testTrace;
    int traceHookID = b.attachTraceHook(&testTrace);
    b.trace_scope(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 1);
    b.trace_scope(traceHookID, b.splat(0xFFFFFFFF), b.splat(0x00000000), -2);
    b.trace_scope(traceHookID, b.splat(0x00000000), b.splat(0x00000000), 3);
    b.trace_scope(traceHookID, b.splat(0x00000000), b.splat(0xFFFFFFFF), 4);
    b.trace_scope(traceHookID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), -5);
    skvm::Program p = b.done();
    p.eval(1);

    REPORTER_ASSERT(r, (testTrace.fBuffer == std::vector<int>{1, -5}));
}

DEF_TEST(SkVM_trace_multiple_hooks, r) {
    class TestTraceHook : public skvm::TraceHook {
    public:
        void var(int, int32_t) override { fBuffer.push_back(-9999999); }
        void enter(int) override        { fBuffer.push_back(-9999999); }
        void exit(int) override         { fBuffer.push_back(-9999999); }
        void scope(int) override        { fBuffer.push_back(-9999999); }
        void line(int lineNum) override { fBuffer.push_back(lineNum); }

        std::vector<int> fBuffer;
    };

    skvm::Builder b;
    TestTraceHook testTraceA, testTraceB, testTraceC;
    int traceHookAID = b.attachTraceHook(&testTraceA);
    int traceHookBID = b.attachTraceHook(&testTraceB);
    int traceHookCID = b.attachTraceHook(&testTraceC);
    b.trace_line(traceHookCID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 111);
    b.trace_line(traceHookAID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 222);
    b.trace_line(traceHookCID, b.splat(0x00000000), b.splat(0x00000000), 333);
    b.trace_line(traceHookBID, b.splat(0xFFFFFFFF), b.splat(0x00000000), 444);
    b.trace_line(traceHookAID, b.splat(0x00000000), b.splat(0xFFFFFFFF), 555);
    b.trace_line(traceHookBID, b.splat(0xFFFFFFFF), b.splat(0xFFFFFFFF), 666);
    skvm::Program p = b.done();
    p.eval(1);

    REPORTER_ASSERT(r, (testTraceA.fBuffer == std::vector<int>{222}));
    REPORTER_ASSERT(r, (testTraceB.fBuffer == std::vector<int>{666}));
    REPORTER_ASSERT(r, (testTraceC.fBuffer == std::vector<int>{111}));
}

DEF_TEST(SkVM_premul, reporter) {
    // Test that premul is short-circuited when alpha is known opaque.
    {
        skvm::Builder p;
        auto rptr = p.varying<int>(),
             aptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.loadF(aptr);

        p.premul(&r, &g, &b, a);
        p.storeF(rptr, r);

        // load red, load alpha, red *= alpha, store red
        REPORTER_ASSERT(reporter, p.done().instructions().size() == 4);
    }

    {
        skvm::Builder p;
        auto rptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.splat(1.0f);

        p.premul(&r, &g, &b, a);
        p.storeF(rptr, r);

        // load red, store red
        REPORTER_ASSERT(reporter, p.done().instructions().size() == 2);
    }

    // Same deal for unpremul.
    {
        skvm::Builder p;
        auto rptr = p.varying<int>(),
             aptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.loadF(aptr);

        p.unpremul(&r, &g, &b, a);
        p.storeF(rptr, r);

        // load red, load alpha, a bunch of unpremul instructions, store red
        REPORTER_ASSERT(reporter, p.done().instructions().size() >= 4);
    }

    {
        skvm::Builder p;
        auto rptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.splat(1.0f);

        p.unpremul(&r, &g, &b, a);
        p.storeF(rptr, r);

        // load red, store red
        REPORTER_ASSERT(reporter, p.done().instructions().size() == 2);
    }
}

DEF_TEST(SkVM_approx_math, r) {
    auto eval = [](int N, float values[], auto fn) {
        skvm::Builder b;
        skvm::Ptr inout  = b.varying<float>();

        b.storeF(inout, fn(&b, b.loadF(inout)));

        b.done().eval(N, values);
    };

    auto compare = [r](int N, const float values[], const float expected[]) {
        for (int i = 0; i < N; ++i) {
            REPORTER_ASSERT(r, (values[i] == expected[i]) ||
                               SkScalarNearlyEqual(values[i], expected[i], 0.001f),
                               "evaluated to %g, but expected %g", values[i], expected[i]);
        }
    };

    // log2
    {
        float values[] = {0.25f, 0.5f, 1, 2, 4, 8};
        constexpr int N = std::size(values);
        eval(N, values, [](skvm::Builder* b, skvm::F32 v) {
            return b->approx_log2(v);
        });
        const float expected[] = {-2, -1, 0, 1, 2, 3};
        compare(N, values, expected);
    }

    // pow2
    {
        float values[] = {-80, -5, -2, -1, 0, 1, 2, 3, 5, 160};
        constexpr int N = std::size(values);
        eval(N, values, [](skvm::Builder* b, skvm::F32 v) {
            return b->approx_pow2(v);
        });
        const float expected[] = {0, 0.03125f, 0.25f, 0.5f, 1, 2, 4, 8, 32, INFINITY};
        compare(N, values, expected);
    }
    // powf -- 1^x
    {
        float exps[] = {-2, -1, 0, 1, 2};
        constexpr int N = std::size(exps);
        eval(N, exps, [](skvm::Builder* b, skvm::F32 exp) {
            return b->approx_powf(b->splat(1.0f), exp);
        });
        const float expected[] = {1, 1, 1, 1, 1};
        compare(N, exps, expected);
    }
    // powf -- 2^x
    {
        float exps[] = {-80, -5, -2, -1, 0, 1, 2, 3, 5, 160};
        constexpr int N = std::size(exps);
        eval(N, exps, [](skvm::Builder* b, skvm::F32 exp) {
            return b->approx_powf(2.0, exp);
        });
        const float expected[] = {0, 0.03125f, 0.25f, 0.5f, 1, 2, 4, 8, 32, INFINITY};
        compare(N, exps, expected);
    }
    // powf -- 3^x
    {
        float exps[] = {-2, -1, 0, 1, 2};
        constexpr int N = std::size(exps);
        eval(N, exps, [](skvm::Builder* b, skvm::F32 exp) {
            return b->approx_powf(b->splat(3.0f), exp);
        });
        const float expected[] = {1/9.0f, 1/3.0f, 1, 3, 9};
        compare(N, exps, expected);
    }
    // powf -- x^0.5
    {
        float bases[] = {0, 1, 4, 9, 16};
        constexpr int N = std::size(bases);
        eval(N, bases, [](skvm::Builder* b, skvm::F32 base) {
            return b->approx_powf(base, b->splat(0.5f));
        });
        const float expected[] = {0, 1, 2, 3, 4};
        compare(N, bases, expected);
    }
    // powf -- x^1
    {
        float bases[] = {0, 1, 2, 3, 4};
        constexpr int N = std::size(bases);
        eval(N, bases, [](skvm::Builder* b, skvm::F32 base) {
            return b->approx_powf(base, b->splat(1.0f));
        });
        const float expected[] = {0, 1, 2, 3, 4};
        compare(N, bases, expected);
    }
    // powf -- x^2
    {
        float bases[] = {0, 1, 2, 3, 4};
        constexpr int N = std::size(bases);
        eval(N, bases, [](skvm::Builder* b, skvm::F32 base) {
            return b->approx_powf(base, b->splat(2.0f));
        });
        const float expected[] = {0, 1, 4, 9, 16};
        compare(N, bases, expected);
    }

    auto test = [r](float arg, float expected, float tolerance, auto prog) {
        skvm::Builder b;
        skvm::Ptr inout  = b.varying<float>();
        b.storeF(inout, prog(b.loadF(inout)));
        float actual = arg;
        b.done().eval(1, &actual);

        float err = std::abs(actual - expected);

        if (err > tolerance) {
    //        SkDebugf("arg %g, expected %g, actual %g\n", arg, expected, actual);
            REPORTER_ASSERT(r, true);
        }
        return err;
    };

    auto test2 = [r](float arg0, float arg1, float expected, float tolerance, auto prog) {
        skvm::Builder b;
        skvm::Ptr in0  = b.varying<float>();
        skvm::Ptr in1  = b.varying<float>();
        skvm::Ptr out  = b.varying<float>();
        b.storeF(out, prog(b.loadF(in0), b.loadF(in1)));
        float actual;
        b.done().eval(1, &arg0, &arg1, &actual);

        float err = std::abs(actual - expected);

        if (err > tolerance) {
    //        SkDebugf("[%g, %g]: expected %g, actual %g\n", arg0, arg1, expected, actual);
            REPORTER_ASSERT(r, true);
        }
        return err;
    };

    // sine, cosine, tangent
    {
        constexpr float P = SK_ScalarPI;
        constexpr float tol = 0.00175f;
        for (float rad = -5*P; rad <= 5*P; rad += 0.1f) {
            test(rad, sk_float_sin(rad), tol, [](skvm::F32 x) {
                return approx_sin(x);
            });
            test(rad, sk_float_cos(rad), tol, [](skvm::F32 x) {
                return approx_cos(x);
            });
        }

        // Our tangent diverge more as we get near infinities (x near +- Pi/2),
        // so bring in the domain a little.
        constexpr float eps = 0.16f;
        float err = 0;
        for (float rad = -P/2 + eps; rad <= P/2 - eps; rad += 0.01f) {
            err += test(rad, sk_float_tan(rad), tol, [](skvm::F32 x) {
                return approx_tan(x);
            });
            // try again with some multiples of P, to check our periodicity
            test(rad, sk_float_tan(rad), tol, [=](skvm::F32 x) {
                return approx_tan(x + 3*P);
            });
            test(rad, sk_float_tan(rad), tol, [=](skvm::F32 x) {
                return approx_tan(x - 3*P);
            });
        }
        if ((false)) { SkDebugf("tan error %g\n", err); }
    }

    // asin, acos, atan
    {
        constexpr float tol = 0.00175f;
        float err = 0;
        for (float x = -1; x <= 1; x += 1.0f/64) {
            err += test(x, asin(x), tol, [](skvm::F32 x) {
                return approx_asin(x);
            });
            test(x, acos(x), tol, [](skvm::F32 x) {
                return approx_acos(x);
            });
        }
        if ((false)) { SkDebugf("asin error %g\n", err); }

        err = 0;
        for (float x = -10; x <= 10; x += 1.0f/16) {
            err += test(x, atan(x), tol, [](skvm::F32 x) {
                return approx_atan(x);
            });
        }
        if ((false)) { SkDebugf("atan error %g\n", err); }

        for (float y = -3; y <= 3; y += 1) {
            for (float x = -3; x <= 3; x += 1) {
                err += test2(y, x, atan2(y,x), tol, [](skvm::F32 y, skvm::F32 x) {
                    return approx_atan2(y,x);
                });
            }
        }
        if ((false)) { SkDebugf("atan2 error %g\n", err); }
    }
}

DEF_TEST(SkVM_min_max, r) {
    // min() and max() have subtle behavior when one argument is NaN and
    // the other isn't.  It's not sound to blindly swap their arguments.
    //
    // All backends must behave like std::min() and std::max(), which are
    //
    //    min(x,y) = y<x ? y : x
    //    max(x,y) = x<y ? y : x

    // ±NaN, ±0, ±1, ±inf
    const uint32_t bits[] = {0x7f80'0001, 0xff80'0001, 0x0000'0000, 0x8000'0000,
                             0x3f80'0000, 0xbf80'0000, 0x7f80'0000, 0xff80'0000};

    float f[8];
    memcpy(f, bits, sizeof(bits));

    auto identical = [&](float x, float y) {
        uint32_t X,Y;
        memcpy(&X, &x, 4);
        memcpy(&Y, &y, 4);
        return X == Y;
    };

    // Test min/max with non-constant x, non-constant y.
    // (Whether x and y are varying or uniform shouldn't make any difference.)
    {
        skvm::Builder b;
        {
            skvm::Ptr src = b.varying<float>(),
                       mn = b.varying<float>(),
                       mx = b.varying<float>();

            skvm::F32 x = b.loadF(src),
                      y = b.uniformF(b.uniform(), 0);

            b.storeF(mn, b.min(x,y));
            b.storeF(mx, b.max(x,y));
        }

        test_interpreter(b, [&](const skvm::Program& program){
            float mn[8], mx[8];
            for (int i = 0; i < 8; i++) {
                // min() and max() everything with f[i].
                program.eval(8, f,mn,mx, &f[i]);

                for (int j = 0; j < 8; j++) {
                    REPORTER_ASSERT(r, identical(mn[j], std::min(f[j], f[i])));
                    REPORTER_ASSERT(r, identical(mx[j], std::max(f[j], f[i])));
                }
            }
        });
    }

    // Test each with constant on the right.
    for (int i = 0; i < 8; i++) {
        skvm::Builder b;
        {
            skvm::Ptr src = b.varying<float>(),
                       mn = b.varying<float>(),
                       mx = b.varying<float>();

            skvm::F32 x = b.loadF(src),
                      y = b.splat(f[i]);

            b.storeF(mn, b.min(x,y));
            b.storeF(mx, b.max(x,y));
        }

        test_interpreter(b, [&](const skvm::Program& program){
            float mn[8], mx[8];
            program.eval(8, f,mn,mx);
            for (int j = 0; j < 8; j++) {
                REPORTER_ASSERT(r, identical(mn[j], std::min(f[j], f[i])));
                REPORTER_ASSERT(r, identical(mx[j], std::max(f[j], f[i])));
            }
        });
    }

    // Test each with constant on the left.
    for (int i = 0; i < 8; i++) {
        skvm::Builder b;
        {
            skvm::Ptr src = b.varying<float>(),
                       mn = b.varying<float>(),
                       mx = b.varying<float>();

            skvm::F32 x = b.splat(f[i]),
                      y = b.loadF(src);

            b.storeF(mn, b.min(x,y));
            b.storeF(mx, b.max(x,y));
        }

        test_interpreter(b, [&](const skvm::Program& program){
            float mn[8], mx[8];
            program.eval(8, f,mn,mx);
            for (int j = 0; j < 8; j++) {
                REPORTER_ASSERT(r, identical(mn[j], std::min(f[i], f[j])));
                REPORTER_ASSERT(r, identical(mx[j], std::max(f[i], f[j])));
            }
        });
    }
}

DEF_TEST(SkVM_halfs, r) {
    const uint16_t hs[] = {0x0000,0x3800,0x3c00,0x4000,
                           0xc400,0xb800,0xbc00,0xc000};
    const float fs[] = {+0.0f,+0.5f,+1.0f,+2.0f,
                        -4.0f,-0.5f,-1.0f,-2.0f};
    {
        skvm::Builder b;
        skvm::Ptr src = b.varying<uint16_t>(),
                  dst = b.varying<float>();
        b.storeF(dst, b.from_fp16(b.load16(src)));

        test_interpreter(b, [&](const skvm::Program& program){
            float dst[8];
            program.eval(8, hs, dst);
            for (int i = 0; i < 8; i++) {
                REPORTER_ASSERT(r, dst[i] == fs[i]);
            }
        });
    }
    {
        skvm::Builder b;
        skvm::Ptr src = b.varying<float>(),
                  dst = b.varying<uint16_t>();
        b.store16(dst, b.to_fp16(b.loadF(src)));

        test_interpreter(b, [&](const skvm::Program& program){
            uint16_t dst[8];
            program.eval(8, fs, dst);
            for (int i = 0; i < 8; i++) {
                REPORTER_ASSERT(r, dst[i] == hs[i]);
            }
        });
    }
}

DEF_TEST(SkVM_64bit, r) {
    uint32_t lo[65],
             hi[65];
    uint64_t wide[65];
    for (int i = 0; i < 65; i++) {
        lo[i] = 2*i+0;
        hi[i] = 2*i+1;
        wide[i] = ((uint64_t)lo[i] <<  0)
                | ((uint64_t)hi[i] << 32);
    }

    {
        skvm::Builder b;
        {
            skvm::Ptr widePtr = b.varying<uint64_t>(),
                        loPtr = b.varying<int>(),
                        hiPtr = b.varying<int>();
            b.store32(loPtr, b.load64(widePtr, 0));
            b.store32(hiPtr, b.load64(widePtr, 1));
        }
        test_interpreter(b, [&](const skvm::Program& program){
            uint32_t l[65], h[65];
            program.eval(65, wide,l,h);
            for (int i = 0; i < 65; i++) {
                REPORTER_ASSERT(r, l[i] == lo[i]);
                REPORTER_ASSERT(r, h[i] == hi[i]);
            }
        });
    }

    {
        skvm::Builder b;
        {
            skvm::Ptr widePtr = b.varying<uint64_t>(),
                        loPtr = b.varying<int>(),
                        hiPtr = b.varying<int>();
            b.store64(widePtr, b.load32(loPtr), b.load32(hiPtr));
        }
        test_interpreter(b, [&](const skvm::Program& program){
            uint64_t w[65];
            program.eval(65, w,lo,hi);
            for (int i = 0; i < 65; i++) {
                REPORTER_ASSERT(r, w[i] == wide[i]);
            }
        });
    }
}

DEF_TEST(SkVM_128bit, r) {
    float   floats[4*63];
    uint8_t packed[4*63];

    for (int i = 0; i < 4*63; i++) {
        floats[i] = i * (1/255.0f);
    }

    skvm::PixelFormat rgba_ffff = skvm::SkColorType_to_PixelFormat(kRGBA_F32_SkColorType),
                      rgba_8888 = skvm::SkColorType_to_PixelFormat(kRGBA_8888_SkColorType);

    {  // Convert RGBA F32 to RGBA 8888, testing 128-bit loads.
        skvm::Builder b;
        {
            skvm::Ptr dst = b.varying(4),
                      src = b.varying(16);

            skvm::Color c = b.load(rgba_ffff, src);
            b.store(rgba_8888, dst, c);
        }
        test_interpreter(b, [&](const skvm::Program& program){
            memset(packed, 0, sizeof(packed));
            program.eval(63, packed, floats);
            for (int i = 0; i < 4*63; i++) {
                REPORTER_ASSERT(r, packed[i] == i);
            }
        });
    }


    {  // Convert RGBA 8888 to RGBA F32, testing 128-bit stores.
        skvm::Builder b;
        {
            skvm::Ptr dst = b.varying(16),
                      src = b.varying(4);

            skvm::Color c = b.load(rgba_8888, src);
            b.store(rgba_ffff, dst, c);
        }
        test_interpreter(b, [&](const skvm::Program& program){
            memset(floats, 0, sizeof(floats));
            program.eval(63, floats, packed);
            for (int i = 0; i < 4*63; i++) {
                REPORTER_ASSERT(r, floats[i] == i * (1/255.0f));
            }
        });
    }

}

DEF_TEST(SkVM_is_NaN_is_finite, r) {
    skvm::Builder b;
    {
        skvm::Ptr src = b.varying<float>(),
                  nan = b.varying<int>(),
                  fin = b.varying<int>();
        b.store32(nan, is_NaN   (b.loadF(src)));
        b.store32(fin, is_finite(b.loadF(src)));
    }
    test_interpreter(b, [&](const skvm::Program& program){
        // ±NaN, ±0, ±1, ±inf
        const uint32_t bits[] = {0x7f80'0001, 0xff80'0001, 0x0000'0000, 0x8000'0000,
                                 0x3f80'0000, 0xbf80'0000, 0x7f80'0000, 0xff80'0000};
        uint32_t nan[8], fin[8];
        program.eval(8, bits, nan,fin);

        for (int i = 0; i < 8; i++) {
            REPORTER_ASSERT(r, nan[i] == ((i == 0 || i == 1) ? 0xffffffff : 0));
            REPORTER_ASSERT(r, fin[i] == ((i == 2 || i == 3 ||
                                           i == 4 || i == 5) ? 0xffffffff : 0));
        }
    });
}

DEF_TEST(SkVM_args, r) {
    // Test we can handle at least six arguments.
    skvm::Builder b;
    {
        skvm::Ptr dst = b.varying<float>(),
                    A = b.varying<float>(),
                    B = b.varying<float>(),
                    C = b.varying<float>(),
                    D = b.varying<float>(),
                    E = b.varying<float>();
        storeF(dst, b.loadF(A)
                  + b.loadF(B)
                  + b.loadF(C)
                  + b.loadF(D)
                  + b.loadF(E));
    }

    test_interpreter(b, [&](const skvm::Program& program){
        float dst[17],A[17],B[17],C[17],D[17],E[17];
        for (int i = 0; i < 17; i++) {
            A[i] = B[i] = C[i] = D[i] = E[i] = (float)i;
        }
        program.eval(17, dst,A,B,C,D,E);
        for (int i = 0; i < 17; i++) {
            REPORTER_ASSERT(r, dst[i] == 5.0f*i);
        }
    });
}

DEF_TEST(SkVM_badpack, reporter) {
    // Test case distilled from actual failing draw,
    // originally with a bad arm64 implementation of pack().
    skvm::Builder p;
    {
        skvm::UPtr uniforms = p.uniform();
        skvm::Ptr dst = p.varying<uint16_t>();

        skvm::I32 r = round(p.uniformF(uniforms, 8) * 15),
                  a = p.splat(0xf);

        skvm::I32 _4444 = p.splat(0);
        _4444 = pack(_4444, r, 12);
        _4444 = pack(_4444, a,  0);
        store16(dst, _4444);
    }

    test_interpreter(p, [&](const skvm::Program& program){
        const float uniforms[] = { 0.0f, 0.0f,
                                   1.0f, 0.0f, 0.0f, 1.0f };

        uint16_t dst[17] = {0};
        program.eval(17, uniforms,dst);
        for (int i = 0; i < 17; i++) {
            REPORTER_ASSERT(reporter, dst[i] == 0xf00f, "got %04x, want %04x\n", dst[i], 0xf00f);
        }
    });
}

DEF_TEST(SkVM_features, r) {
    auto build_program = [](skvm::Builder* b) {
        skvm::F32 x = b->loadF(b->varying<float>());
        b->storeF(b->varying<float>(), x*x+x);
    };

    {   // load-fma-store with FMA available.
        skvm::Features features;
        features.fma = true;
        skvm::Builder b(features);
        build_program(&b);
        REPORTER_ASSERT(r, b.optimize().size() == 3);
    }

    {   // load-mul-add-store without FMA.
        skvm::Features features;
        features.fma = false;
        skvm::Builder b(features);
        build_program(&b);
        REPORTER_ASSERT(r, b.optimize().size() == 4);
    }

    {   // Auto-detected, could be either.
        skvm::Builder b;
        build_program(&b);
        REPORTER_ASSERT(r, b.optimize().size() == 3
                        || b.optimize().size() == 4);
    }
}

DEF_TEST(SkVM_gather_can_hoist, r) {
    // A gather instruction isn't necessarily varying... it's whatever its index is.
    // First a typical gather scenario with varying index.
    {
        skvm::Builder b;
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf = b.varying<int>();
        skvm::I32 ix = b.load32(buf);
        b.store32(buf, b.gather32(uniforms,0, ix));

        skvm::Program p = b.done();

        // ix is varying, so the gather is too.
        //
        // loop:
        //     v0 = load32 buf
        //     v1 = gather32 uniforms+0 v0
        //     store32 buf v1
        REPORTER_ASSERT(r, p.instructions().size() == 3);
        REPORTER_ASSERT(r, p.loop() == 0);
    }

    // Now the same but with a uniform index instead.
    {
        skvm::Builder b;
        skvm::UPtr uniforms = b.uniform();
        skvm::Ptr buf = b.varying<int>();
        skvm::I32 ix = b.uniform32(uniforms,8);
        b.store32(buf, b.gather32(uniforms,0, ix));

        skvm::Program p = b.done();

        // ix is uniform, so the gather is too.
        //
        // v0 = uniform32 uniforms+8
        // v1 = gather32 uniforms+0 v0
        // loop:
        //     store32 buf v1
        REPORTER_ASSERT(r, p.instructions().size() == 3);
        REPORTER_ASSERT(r, p.loop() == 2);
    }
}

DEF_TEST(SkVM_dont_dedup_loads, r) {
    // We've been assuming that all Ops with the same arguments produce the same value
    // and deduplicating them, which results in a simple common subexpression eliminator.
    //
    // But we can't soundly dedup two identical loads with a store between.
    // If we dedup the loads in this test program it will always increment by 1, not K.
    constexpr int K = 2;
    skvm::Builder b;
    {
        skvm::Ptr buf = b.varying<int>();
        for (int i = 0; i < K; i++) {
            b.store32(buf, b.load32(buf) + 1);
        }
    }

    test_interpreter(b, [&](const skvm::Program& program){
        int buf[] = { 0,1,2,3,4 };
        program.eval(std::size(buf), buf);
        for (int i = 0; i < (int)std::size(buf); i++) {
            REPORTER_ASSERT(r, buf[i] == i+K);
        }
    });
}

DEF_TEST(SkVM_dont_dedup_stores, r) {
    // Following a similar line of reasoning to SkVM_dont_dedup_loads,
    // we cannot dedup stores either.  A different store between two identical stores
    // will invalidate the first store, meaning we do need to reissue that store operation.
    skvm::Builder b;
    {
        skvm::Ptr buf = b.varying<int>();
        b.store32(buf, b.splat(4));
        b.store32(buf, b.splat(5));
        b.store32(buf, b.splat(4));   // If we dedup'd, we'd skip this store.
    }

    test_interpreter(b, [&](const skvm::Program& program){
        int buf[42];
        program.eval(std::size(buf), buf);
        for (int x : buf) {
            REPORTER_ASSERT(r, x == 4);
        }
    });
}

DEF_TEST(SkVM_fast_mul, r) {
    skvm::Builder b;
    {
        skvm::Ptr src = b.varying<float>(),
                 fast = b.varying<float>(),
                 slow = b.varying<float>();
        skvm::F32 x = b.loadF(src);
        b.storeF(fast, fast_mul(0.0f, x));
        b.storeF(slow, 0.0f * x);
    }
    test_interpreter(b, [&](const skvm::Program& program){
        const uint32_t bits[] = {
            0x0000'0000, 0x8000'0000, //±0
            0x3f80'0000, 0xbf80'0000, //±1
            0x7f80'0000, 0xff80'0000, //±inf
            0x7f80'0001, 0xff80'0001, //±NaN
        };
        float fast[8],
              slow[8];
        program.eval(8,bits,fast,slow);

        for (int i = 0; i < 8; i++) {
            REPORTER_ASSERT(r, fast[i] == 0.0f);

            if (i < 4) {
                REPORTER_ASSERT(r, slow[i] == 0.0f);
            } else {
                REPORTER_ASSERT(r, isnan(slow[i]));
            }
        }
    });
}

DEF_TEST(SkVM_duplicates, reporter) {
    {
        skvm::Builder p(true);
        auto rptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.splat(1.0f);

        p.unpremul(&r, &g, &b, a);
        p.storeF(rptr, r);

        std::vector<skvm::Instruction> program = b->program();

        auto withDuplicates = skvm::finalize(program);
        int duplicates = 0;
        for (const auto& instr : withDuplicates) {
            if (instr.op == skvm::Op::duplicate) {
                ++duplicates;
            }
        }
        REPORTER_ASSERT(reporter, duplicates > 0);

        auto eliminatedAsDeadCode = skvm::eliminate_dead_code(program);
        for (const auto& instr : eliminatedAsDeadCode) {
            REPORTER_ASSERT(reporter, instr.op != skvm::Op::duplicate);
        }
    }

    {
        skvm::Builder p(false);
        auto rptr = p.varying<int>();

        skvm::F32 r = p.loadF(rptr),
                  g = p.splat(0.0f),
                  b = p.splat(0.0f),
                  a = p.splat(1.0f);

        p.unpremul(&r, &g, &b, a);
        p.storeF(rptr, r);

        auto withoutDuplicates = p.done().instructions();
        for (const auto& instr : withoutDuplicates) {
            REPORTER_ASSERT(reporter, instr.op != skvm::Op::duplicate);
        }
    }
}
