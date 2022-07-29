/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVM_DEFINED
#define SkVM_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkSpan.h"
#include "include/private/SkMacros.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkVM_fwd.h"
#include <vector>      // std::vector

class SkWStream;

namespace skvm {

    // Order matters a little: Ops <=store128 are treated as having side effects.
    #define SKVM_OPS(M)                                              \
        M(assert_true)                                               \
        M(trace_line) M(trace_var)                                   \
        M(trace_enter) M(trace_exit) M(trace_scope)                  \
        M(store8)   M(store16)   M(store32) M(store64) M(store128)   \
        M(load8)    M(load16)    M(load32)  M(load64) M(load128)     \
        M(index)                                                     \
        M(gather8)  M(gather16)  M(gather32)                         \
                                 M(uniform32)                        \
                                 M(array32)                          \
        M(splat)                                                     \
        M(add_f32) M(add_i32)                                        \
        M(sub_f32) M(sub_i32)                                        \
        M(mul_f32) M(mul_i32)                                        \
        M(div_f32)                                                   \
        M(min_f32) M(max_f32)                                        \
        M(fma_f32) M(fms_f32) M(fnma_f32)                            \
        M(sqrt_f32)                                                  \
        M(shl_i32) M(shr_i32) M(sra_i32)                             \
        M(ceil) M(floor) M(trunc) M(round) M(to_fp16) M(from_fp16)   \
        M(to_f32)                                                    \
        M(neq_f32) M(eq_f32) M(eq_i32)                               \
        M(gte_f32) M(gt_f32) M(gt_i32)                               \
        M(bit_and)     M(bit_or)     M(bit_xor)     M(bit_clear)     \
        M(select)                                                    \
        M(duplicate)
    // End of SKVM_OPS

    enum class Op : int {
    #define M(op) op,
        SKVM_OPS(M)
    #undef M
    };

    static inline bool has_side_effect(Op op) {
        return op <= Op::store128;
    }
    static inline bool touches_varying_memory(Op op) {
        return Op::store8 <= op && op <= Op::load128;
    }
    static inline bool is_always_varying(Op op) {
        return Op::store8 <= op && op <= Op::index;
    }
    static inline bool is_trace(Op op) {
        return Op::trace_line <= op && op <= Op::trace_scope;
    }

    using Val = int;
    // We reserve an impossible Val ID as a sentinel
    // NA meaning none, n/a, null, nil, etc.
    static const Val NA = -1;

    // Ptr and UPtr are an index into the registers args[]. The two styles of using args are
    // varyings and uniforms. Varyings use Ptr, have a stride associated with them, and are
    // evaluated everytime through the loop. Uniforms use UPtr, don't have a stride, and are
    // usually hoisted above the loop.
    struct Ptr { int ix; };
    struct UPtr : public Ptr {};

    bool operator!=(Ptr a, Ptr b);

    struct I32 {
        Builder* builder = nullptr;
        Val      id      = NA;
        explicit operator bool() const { return id != NA; }
        Builder* operator->()    const { return builder; }
    };

    struct F32 {
        Builder* builder = nullptr;
        Val      id      = NA;
        explicit operator bool() const { return id != NA; }
        Builder* operator->()    const { return builder; }
    };

    struct Color {
        F32 r,g,b,a;
        explicit operator bool() const { return r && g && b && a; }
        Builder* operator->()    const { return a.operator->(); }
    };

    struct HSLA {
        F32 h,s,l,a;
        explicit operator bool() const { return h && s && l && a; }
        Builder* operator->()    const { return a.operator->(); }
    };

    struct Coord {
        F32 x,y;
        explicit operator bool() const { return x && y; }
        Builder* operator->()    const { return x.operator->(); }
    };

    struct Uniform {
        UPtr ptr;
        int offset;
    };
    struct Uniforms {
        UPtr             base;
        std::vector<int> buf;

        Uniforms(UPtr ptr, int init) : base(ptr), buf(init) {}

        Uniform push(int val) {
            buf.push_back(val);
            return {base, (int)( sizeof(int)*(buf.size() - 1) )};
        }

        Uniform pushF(float val) {
            int bits;
            memcpy(&bits, &val, sizeof(int));
            return this->push(bits);
        }

        Uniform pushPtr(const void* ptr) {
            // Jam the pointer into 1 or 2 ints.
            int ints[sizeof(ptr) / sizeof(int)];
            memcpy(ints, &ptr, sizeof(ptr));
            for (int bits : ints) {
                buf.push_back(bits);
            }
            return {base, (int)( sizeof(int)*(buf.size() - std::size(ints)) )};
        }

        Uniform pushArray(int32_t a[]) {
            return this->pushPtr(a);
        }

        Uniform pushArrayF(float a[]) {
            return this->pushPtr(a);
        }
    };

    struct PixelFormat {
        enum { UNORM, SRGB, FLOAT} encoding;
        int r_bits,  g_bits,  b_bits,  a_bits,
            r_shift, g_shift, b_shift, a_shift;
    };
    PixelFormat SkColorType_to_PixelFormat(SkColorType);

    SK_BEGIN_REQUIRE_DENSE
    struct Instruction {
        Op  op;              // v* = op(x,y,z,w,immA,immB), where * == index of this Instruction.
        Val x,y,z,w;         // Enough arguments for Op::store128.
        int immA,immB,immC;  // Immediate bit pattern, shift count, pointer index, byte offset, etc.
    };
    SK_END_REQUIRE_DENSE

    bool operator==(const Instruction&, const Instruction&);
    struct InstructionHash {
        uint32_t operator()(const Instruction&, uint32_t seed=0) const;
    };

    struct OptimizedInstruction {
        Op op;
        Val x,y,z,w;
        int immA,immB,immC;

        Val  death;
        bool can_hoist;
    };

    struct Features {
        bool fma   = false;
        bool fp16  = false;
    };

    class TraceHook {
    public:
        virtual ~TraceHook() = default;
        virtual void line(int lineNum) = 0;
        virtual void var(int slot, int32_t val) = 0;
        virtual void enter(int fnIdx) = 0;
        virtual void exit(int fnIdx) = 0;
        virtual void scope(int delta) = 0;
    };

    class Builder {
    public:
        Builder(bool createDuplicates = false);
        Builder(Features, bool createDuplicates = false);

        Program done() const;

        // Mostly for debugging, tests, etc.
        std::vector<Instruction> program() const { return fProgram; }
        std::vector<OptimizedInstruction> optimize() const;

        // Returns a trace-hook ID which must be passed to the trace opcodes.
        int attachTraceHook(TraceHook*);

        // Convenience arg() wrappers for most common strides, sizeof(T) and 0.
        template <typename T>
        Ptr varying() { return this->arg(sizeof(T)); }
        Ptr varying(int stride) { SkASSERT(stride > 0); return this->arg(stride); }
        UPtr uniform() { Ptr p = this->arg(0); return UPtr{{p.ix}}; }

        // TODO: allow uniform (i.e. Ptr) offsets to store* and load*?
        // TODO: sign extension (signed types) for <32-bit loads?
        // TODO: unsigned integer operations where relevant (just comparisons?)?

        // Assert cond is true, printing debug when not.
        void assert_true(I32 cond, I32 debug);
        void assert_true(I32 cond, F32 debug) { assert_true(cond, pun_to_I32(debug)); }
        void assert_true(I32 cond)            { assert_true(cond, cond); }

        // Insert debug traces into the instruction stream
        bool mergeMasks(I32& mask, I32& traceMask);
        void trace_line (int traceHookID, I32 mask, I32 traceMask, int line);
        void trace_var  (int traceHookID, I32 mask, I32 traceMask, int slot, I32 val);
        void trace_enter(int traceHookID, I32 mask, I32 traceMask, int fnIdx);
        void trace_exit (int traceHookID, I32 mask, I32 traceMask, int fnIdx);
        void trace_scope(int traceHookID, I32 mask, I32 traceMask, int delta);

        // Store {8,16,32,64,128}-bit varying.
        void store8  (Ptr ptr, I32 val);
        void store16 (Ptr ptr, I32 val);
        void store32 (Ptr ptr, I32 val);
        void storeF  (Ptr ptr, F32 val) { store32(ptr, pun_to_I32(val)); }
        void store64 (Ptr ptr, I32 lo, I32 hi);              // *ptr = lo|(hi<<32)
        void store128(Ptr ptr, I32 x, I32 y, I32 z, I32 w);  // *ptr = x|(y<<32)|(z<<64)|(w<<96)

        // Returns varying {n, n-1, n-2, ..., 1}, where n is the argument to Program::eval().
        I32 index();

        // Load {8,16,32,64,128}-bit varying.
        I32 load8  (Ptr ptr);
        I32 load16 (Ptr ptr);
        I32 load32 (Ptr ptr);
        F32 loadF  (Ptr ptr) { return pun_to_F32(load32(ptr)); }
        I32 load64 (Ptr ptr, int lane);  // Load 32-bit lane 0-1 of  64-bit value.
        I32 load128(Ptr ptr, int lane);  // Load 32-bit lane 0-3 of 128-bit value.

        // Load i32/f32 uniform with byte-count offset.
        I32 uniform32(UPtr ptr, int offset);
        F32 uniformF (UPtr ptr, int offset) { return pun_to_F32(uniform32(ptr,offset)); }

        // Load i32/f32 uniform with byte-count offset and an c-style array index. The address of
        // the element is (*(ptr + byte-count offset))[index].
        I32 array32  (UPtr ptr, int offset, int index);
        F32 arrayF   (UPtr ptr, int offset, int index) {
            return pun_to_F32(array32(ptr, offset, index));
        }

        // Push and load this color as a uniform.
        Color uniformColor(SkColor4f, Uniforms*);

        // Gather u8,u16,i32 with varying element-count index from *(ptr + byte-count offset).
        I32 gather8 (UPtr ptr, int offset, I32 index);
        I32 gather16(UPtr ptr, int offset, I32 index);
        I32 gather32(UPtr ptr, int offset, I32 index);
        F32 gatherF (UPtr ptr, int offset, I32 index) {
            return pun_to_F32(gather32(ptr, offset, index));
        }

        // Convenience methods for working with skvm::Uniform(s).
        I32 uniform32(Uniform u)            { return this->uniform32(u.ptr, u.offset); }
        F32 uniformF (Uniform u)            { return this->uniformF (u.ptr, u.offset); }
        I32 gather8  (Uniform u, I32 index) { return this->gather8  (u.ptr, u.offset, index); }
        I32 gather16 (Uniform u, I32 index) { return this->gather16 (u.ptr, u.offset, index); }
        I32 gather32 (Uniform u, I32 index) { return this->gather32 (u.ptr, u.offset, index); }
        F32 gatherF  (Uniform u, I32 index) { return this->gatherF  (u.ptr, u.offset, index); }

        // Convenience methods for working with array pointers in skvm::Uniforms. Index is an
        // array index and not a byte offset. The array pointer is stored at u.
        I32 array32  (Uniform a, int index) { return this->array32  (a.ptr, a.offset, index); }
        F32 arrayF   (Uniform a, int index) { return this->arrayF   (a.ptr, a.offset, index); }

        // Load an immediate constant.
        I32 splat(int      n);
        I32 splat(unsigned u) { return splat((int)u); }
        F32 splat(float    f) {
            int bits;
            memcpy(&bits, &f, 4);
            return pun_to_F32(splat(bits));
        }

        // Some operations make sense with immediate arguments,
        // so we provide overloads inline to make that seamless.
        //
        // We omit overloads that may indicate a bug or performance issue.
        // In general it does not make sense to pass immediates to unary operations,
        // and even sometimes not for binary operations, e.g.
        //
        //   div(x, y)    -- normal every day divide
        //   div(3.0f, y) -- yep, makes sense
        //   div(x, 3.0f) -- omitted as a reminder you probably want mul(x, 1/3.0f).
        //
        // You can of course always splat() to override these opinions.

        // float math, comparisons, etc.
        F32 add(F32, F32);
        F32 add(F32 x, float y) { return add(x, splat(y)); }
        F32 add(float x, F32 y) { return add(splat(x), y); }

        F32 sub(F32, F32);
        F32 sub(F32 x, float y) { return sub(x, splat(y)); }
        F32 sub(float x, F32 y) { return sub(splat(x), y); }

        F32 mul(F32, F32);
        F32 mul(F32 x, float y) { return mul(x, splat(y)); }
        F32 mul(float x, F32 y) { return mul(splat(x), y); }

        // mul(), but allowing optimizations not strictly legal under IEEE-754 rules.
        F32 fast_mul(F32, F32);
        F32 fast_mul(F32 x, float y) { return fast_mul(x, splat(y)); }
        F32 fast_mul(float x, F32 y) { return fast_mul(splat(x), y); }

        F32 div(F32, F32);
        F32 div(float x, F32 y) { return div(splat(x), y); }

        F32 min(F32, F32);
        F32 min(F32 x, float y) { return min(x, splat(y)); }
        F32 min(float x, F32 y) { return min(splat(x), y); }

        F32 max(F32, F32);
        F32 max(F32 x, float y) { return max(x, splat(y)); }
        F32 max(float x, F32 y) { return max(splat(x), y); }

        // TODO: remove mad()?  It's just sugar.
        F32 mad(F32   x, F32   y, F32   z) { return add(mul(x,y), z); }
        F32 mad(F32   x, F32   y, float z) { return mad(      x ,       y , splat(z)); }
        F32 mad(F32   x, float y, F32   z) { return mad(      x , splat(y),       z ); }
        F32 mad(F32   x, float y, float z) { return mad(      x , splat(y), splat(z)); }
        F32 mad(float x, F32   y, F32   z) { return mad(splat(x),       y ,       z ); }
        F32 mad(float x, F32   y, float z) { return mad(splat(x),       y , splat(z)); }
        F32 mad(float x, float y, F32   z) { return mad(splat(x), splat(y),       z ); }

        F32        sqrt(F32);
        F32 approx_log2(F32);
        F32 approx_pow2(F32);
        F32 approx_log (F32 x) { return mul(0.69314718f, approx_log2(x)); }
        F32 approx_exp (F32 x) { return approx_pow2(mul(x, 1.4426950408889634074f)); }

        F32 approx_powf(F32 base, F32 exp);
        F32 approx_powf(F32 base, float exp) { return approx_powf(base, splat(exp)); }
        F32 approx_powf(float base, F32 exp) { return approx_powf(splat(base), exp); }


        F32 approx_sin(F32 radians);
        F32 approx_cos(F32 radians) { return approx_sin(add(radians, SK_ScalarPI/2)); }
        F32 approx_tan(F32 radians);

        F32 approx_asin(F32 x);
        F32 approx_acos(F32 x) { return sub(SK_ScalarPI/2, approx_asin(x)); }
        F32 approx_atan(F32 x);
        F32 approx_atan2(F32 y, F32 x);

        F32 lerp(F32   lo, F32   hi, F32   t);
        F32 lerp(F32   lo, F32   hi, float t) { return lerp(      lo ,       hi , splat(t)); }
        F32 lerp(F32   lo, float hi, float t) { return lerp(      lo , splat(hi), splat(t)); }
        F32 lerp(F32   lo, float hi, F32   t) { return lerp(      lo , splat(hi),       t ); }
        F32 lerp(float lo, F32   hi, F32   t) { return lerp(splat(lo),       hi ,       t ); }
        F32 lerp(float lo, F32   hi, float t) { return lerp(splat(lo),       hi , splat(t)); }
        F32 lerp(float lo, float hi, F32   t) { return lerp(splat(lo), splat(hi),       t ); }

        F32 clamp(F32   x, F32   lo, F32   hi) { return max(lo, min(x, hi)); }
        F32 clamp(F32   x, F32   lo, float hi) { return clamp(      x ,       lo , splat(hi)); }
        F32 clamp(F32   x, float lo, float hi) { return clamp(      x , splat(lo), splat(hi)); }
        F32 clamp(F32   x, float lo, F32   hi) { return clamp(      x , splat(lo),       hi ); }
        F32 clamp(float x, F32   lo, F32   hi) { return clamp(splat(x),       lo ,       hi ); }
        F32 clamp(float x, F32   lo, float hi) { return clamp(splat(x),       lo , splat(hi)); }
        F32 clamp(float x, float lo, F32   hi) { return clamp(splat(x), splat(lo),       hi ); }

        F32 clamp01(F32 x) { return clamp(x, 0.0f, 1.0f); }

        F32    abs(F32 x) { return pun_to_F32(bit_and(pun_to_I32(x), 0x7fff'ffff)); }
        F32  fract(F32 x) { return sub(x, floor(x)); }
        F32   ceil(F32);
        F32  floor(F32);
        I32 is_NaN   (F32 x) { return neq(x,x); }
        I32 is_finite(F32 x) { return lt(bit_and(pun_to_I32(x), 0x7f80'0000), 0x7f80'0000); }

        I32 trunc(F32 x);
        I32 round(F32 x);  // Round to int using current rounding mode (as if lrintf()).
        I32 pun_to_I32(F32 x) { return {x.builder, x.id}; }

        I32   to_fp16(F32 x);
        F32 from_fp16(I32 x);

        I32 eq(F32, F32);
        I32 eq(F32 x, float y) { return eq(x, splat(y)); }
        I32 eq(float x, F32 y) { return eq(splat(x), y); }

        I32 neq(F32, F32);
        I32 neq(F32 x, float y) { return neq(x, splat(y)); }
        I32 neq(float x, F32 y) { return neq(splat(x), y); }

        I32 lt(F32, F32);
        I32 lt(F32 x, float y) { return lt(x, splat(y)); }
        I32 lt(float x, F32 y) { return lt(splat(x), y); }

        I32 lte(F32, F32);
        I32 lte(F32 x, float y) { return lte(x, splat(y)); }
        I32 lte(float x, F32 y) { return lte(splat(x), y); }

        I32 gt(F32, F32);
        I32 gt(F32 x, float y) { return gt(x, splat(y)); }
        I32 gt(float x, F32 y) { return gt(splat(x), y); }

        I32 gte(F32, F32);
        I32 gte(F32 x, float y) { return gte(x, splat(y)); }
        I32 gte(float x, F32 y) { return gte(splat(x), y); }

        // int math, comparisons, etc.
        I32 add(I32, I32);
        I32 add(I32 x, int y) { return add(x, splat(y)); }
        I32 add(int x, I32 y) { return add(splat(x), y); }

        I32 sub(I32, I32);
        I32 sub(I32 x, int y) { return sub(x, splat(y)); }
        I32 sub(int x, I32 y) { return sub(splat(x), y); }

        I32 mul(I32, I32);
        I32 mul(I32 x, int y) { return mul(x, splat(y)); }
        I32 mul(int x, I32 y) { return mul(splat(x), y); }

        I32 shl(I32 x, int bits);
        I32 shr(I32 x, int bits);
        I32 sra(I32 x, int bits);

        I32 eq(I32, I32);
        I32 eq(I32 x, int y) { return eq(x, splat(y)); }
        I32 eq(int x, I32 y) { return eq(splat(x), y); }

        I32 neq(I32, I32);
        I32 neq(I32 x, int y) { return neq(x, splat(y)); }
        I32 neq(int x, I32 y) { return neq(splat(x), y); }

        I32 lt(I32, I32);
        I32 lt(I32 x, int y) { return lt(x, splat(y)); }
        I32 lt(int x, I32 y) { return lt(splat(x), y); }

        I32 lte(I32, I32);
        I32 lte(I32 x, int y) { return lte(x, splat(y)); }
        I32 lte(int x, I32 y) { return lte(splat(x), y); }

        I32 gt(I32, I32);
        I32 gt(I32 x, int y) { return gt(x, splat(y)); }
        I32 gt(int x, I32 y) { return gt(splat(x), y); }

        I32 gte(I32, I32);
        I32 gte(I32 x, int y) { return gte(x, splat(y)); }
        I32 gte(int x, I32 y) { return gte(splat(x), y); }

        F32 to_F32(I32 x);
        F32 pun_to_F32(I32 x) { return {x.builder, x.id}; }

        // Bitwise operations.
        I32 bit_and(I32, I32);
        I32 bit_and(I32 x, int y) { return bit_and(x, splat(y)); }
        I32 bit_and(int x, I32 y) { return bit_and(splat(x), y); }

        I32 bit_or(I32, I32);
        I32 bit_or(I32 x, int y) { return bit_or(x, splat(y)); }
        I32 bit_or(int x, I32 y) { return bit_or(splat(x), y); }

        I32 bit_xor(I32, I32);
        I32 bit_xor(I32 x, int y) { return bit_xor(x, splat(y)); }
        I32 bit_xor(int x, I32 y) { return bit_xor(splat(x), y); }

        I32 bit_clear(I32, I32);
        I32 bit_clear(I32 x, int y) { return bit_clear(x, splat(y)); }
        I32 bit_clear(int x, I32 y) { return bit_clear(splat(x), y); }

        I32 min(I32 x, I32 y) { return select(lte(x,y), x, y); }
        I32 min(I32 x, int y) { return min(x, splat(y)); }
        I32 min(int x, I32 y) { return min(splat(x), y); }

        I32 max(I32 x, I32 y) { return select(gte(x,y), x, y); }
        I32 max(I32 x, int y) { return max(x, splat(y)); }
        I32 max(int x, I32 y) { return max(splat(x), y); }

        I32 select(I32 cond, I32 t, I32 f);  // cond ? t : f
        I32 select(I32 cond, int t, I32 f) { return select(cond, splat(t),       f ); }
        I32 select(I32 cond, I32 t, int f) { return select(cond,       t , splat(f)); }
        I32 select(I32 cond, int t, int f) { return select(cond, splat(t), splat(f)); }

        F32 select(I32 cond, F32 t, F32 f) {
            return pun_to_F32(select(cond, pun_to_I32(t)
                                         , pun_to_I32(f)));
        }
        F32 select(I32 cond, float t, F32   f) { return select(cond, splat(t),       f ); }
        F32 select(I32 cond, F32   t, float f) { return select(cond,       t , splat(f)); }
        F32 select(I32 cond, float t, float f) { return select(cond, splat(t), splat(f)); }

        I32 extract(I32 x, int bits, I32 z);   // (x>>bits) & z
        I32 extract(I32 x, int bits, int z) { return extract(x, bits, splat(z)); }
        I32 extract(int x, int bits, I32 z) { return extract(splat(x), bits, z); }

        I32 pack(I32 x, I32 y, int bits);   // x | (y<<bits)
        I32 pack(I32 x, int y, int bits) { return pack(x, splat(y), bits); }
        I32 pack(int x, I32 y, int bits) { return pack(splat(x), y, bits); }


        // Common idioms used in several places, worth centralizing for consistency.
        F32 from_unorm(int bits, I32);   // E.g. from_unorm(8, x) -> x * (1/255.0f)
        I32   to_unorm(int bits, F32);   // E.g.   to_unorm(8, x) -> round(x * 255)

        Color   load(PixelFormat, Ptr ptr);
        void   store(PixelFormat, Ptr ptr, Color);
        Color gather(PixelFormat, UPtr ptr, int offset, I32 index);
        Color gather(PixelFormat f, Uniform u, I32 index) {
            return gather(f, u.ptr, u.offset, index);
        }

        void   premul(F32* r, F32* g, F32* b, F32 a);
        void unpremul(F32* r, F32* g, F32* b, F32 a);

        Color   premul(Color c) {   this->premul(&c.r, &c.g, &c.b, c.a); return c; }
        Color unpremul(Color c) { this->unpremul(&c.r, &c.g, &c.b, c.a); return c; }

        Color lerp(Color lo, Color hi, F32 t);
        Color blend(SkBlendMode, Color src, Color dst);

        Color clamp01(Color c) {
            return { clamp01(c.r), clamp01(c.g), clamp01(c.b), clamp01(c.a) };
        }

        HSLA  to_hsla(Color);
        Color to_rgba(HSLA);

        void dump(SkWStream* = nullptr) const;

        uint64_t hash() const;

        Val push(Instruction);

        bool allImm() const { return true; }

        template <typename T, typename... Rest>
        bool allImm(Val id, T* imm, Rest... rest) const {
            if (fProgram[id].op == Op::splat) {
                static_assert(sizeof(T) == 4);
                memcpy(imm, &fProgram[id].immA, 4);
                return this->allImm(rest...);
            }
            return false;
        }

        bool allUniform() const { return true; }

        template <typename... Rest>
        bool allUniform(Val id, Uniform* uni, Rest... rest) const {
            if (fProgram[id].op == Op::uniform32) {
                uni->ptr.ix = fProgram[id].immA;
                uni->offset = fProgram[id].immB;
                return this->allUniform(rest...);
            }
            return false;
        }

    private:
        // Declare an argument with given stride (use stride=0 for uniforms).
        Ptr arg(int stride);

        Val push(
                Op op, Val x=NA, Val y=NA, Val z=NA, Val w=NA, int immA=0, int immB=0, int immC=0) {
            return this->push(Instruction{op, x,y,z,w, immA,immB,immC});
        }

        template <typename T>
        bool isImm(Val id, T want) const {
            T imm = 0;
            return this->allImm(id, &imm) && imm == want;
        }

        // `canonicalizeIdOrder` and has two rules:
        // - Immediate values go last; that is, `x + 1` is preferred over `1 + x`.
        // - If both/neither of x and y are immediate, lower IDs go before higher IDs.
        // Canonicalizing the IDs helps with opcode deduplication. Putting immediates in a
        // consistent position makes it easier to detect no-op arithmetic like `x + 0`.
        template <typename F32_or_I32>
        void canonicalizeIdOrder(F32_or_I32& x, F32_or_I32& y);

        // If the passed in ID is a bit-not, return the value being bit-notted. Otherwise, NA.
        Val holdsBitNot(Val id);

        SkTHashMap<Instruction, Val, InstructionHash> fIndex;
        std::vector<Instruction>                      fProgram;
        std::vector<TraceHook*>                       fTraceHooks;
        std::vector<int>                              fStrides;
        const Features                                fFeatures;
        bool                                          fCreateDuplicates;
    };

    // Optimization passes and data structures normally used by Builder::optimize(),
    // extracted here so they can be unit tested.
    std::vector<Instruction> eliminate_dead_code(std::vector<Instruction>);
    std::vector<OptimizedInstruction> finalize(std::vector<Instruction>);

    using Reg = int;

    // d = op(x,y,z,w, immA,immB)
    struct InterpreterInstruction {
        Op  op;
        Reg d,x,y,z,w;
        int immA,immB,immC;
    };

    class Program {
    public:
        Program(const std::vector<OptimizedInstruction>& instructions,
                const std::vector<int>& strides,
                const std::vector<TraceHook*>& traceHooks);

        Program();
        ~Program();

        Program(Program&&);
        Program& operator=(Program&&);

        Program(const Program&) = delete;
        Program& operator=(const Program&) = delete;

        void eval(int n, void* args[]) const;

        template <typename... T>
        void eval(int n, T*... arg) const {
            SkASSERT(sizeof...(arg) == this->nargs());
            // This nullptr isn't important except that it makes args[] non-empty if you pass none.
            void* args[] = { (void*)arg..., nullptr };
            this->eval(n, args);
        }

        std::vector<InterpreterInstruction> instructions() const;
        int  nargs() const;
        int  nregs() const;
        int  loop () const;
        bool empty() const;

        bool hasTraceHooks() const;  // Is this program instrumented for debugging?

        void dump(SkWStream* = nullptr) const;

    private:
        void setupInterpreter(const std::vector<OptimizedInstruction>&);

        struct Impl;
        std::unique_ptr<Impl> fImpl;
    };

    // TODO: control flow
    // TODO: 64-bit values?

#define SI static inline

    SI I32 operator+(I32 x, I32 y) { return x->add(x,y); }
    SI I32 operator+(I32 x, int y) { return x->add(x,y); }
    SI I32 operator+(int x, I32 y) { return y->add(x,y); }

    SI I32 operator-(I32 x, I32 y) { return x->sub(x,y); }
    SI I32 operator-(I32 x, int y) { return x->sub(x,y); }
    SI I32 operator-(int x, I32 y) { return y->sub(x,y); }

    SI I32 operator*(I32 x, I32 y) { return x->mul(x,y); }
    SI I32 operator*(I32 x, int y) { return x->mul(x,y); }
    SI I32 operator*(int x, I32 y) { return y->mul(x,y); }

    SI I32 min(I32 x, I32 y) { return x->min(x,y); }
    SI I32 min(I32 x, int y) { return x->min(x,y); }
    SI I32 min(int x, I32 y) { return y->min(x,y); }

    SI I32 max(I32 x, I32 y) { return x->max(x,y); }
    SI I32 max(I32 x, int y) { return x->max(x,y); }
    SI I32 max(int x, I32 y) { return y->max(x,y); }

    SI I32 operator==(I32 x, I32 y) { return x->eq(x,y); }
    SI I32 operator==(I32 x, int y) { return x->eq(x,y); }
    SI I32 operator==(int x, I32 y) { return y->eq(x,y); }

    SI I32 operator!=(I32 x, I32 y) { return x->neq(x,y); }
    SI I32 operator!=(I32 x, int y) { return x->neq(x,y); }
    SI I32 operator!=(int x, I32 y) { return y->neq(x,y); }

    SI I32 operator< (I32 x, I32 y) { return x->lt(x,y); }
    SI I32 operator< (I32 x, int y) { return x->lt(x,y); }
    SI I32 operator< (int x, I32 y) { return y->lt(x,y); }

    SI I32 operator<=(I32 x, I32 y) { return x->lte(x,y); }
    SI I32 operator<=(I32 x, int y) { return x->lte(x,y); }
    SI I32 operator<=(int x, I32 y) { return y->lte(x,y); }

    SI I32 operator> (I32 x, I32 y) { return x->gt(x,y); }
    SI I32 operator> (I32 x, int y) { return x->gt(x,y); }
    SI I32 operator> (int x, I32 y) { return y->gt(x,y); }

    SI I32 operator>=(I32 x, I32 y) { return x->gte(x,y); }
    SI I32 operator>=(I32 x, int y) { return x->gte(x,y); }
    SI I32 operator>=(int x, I32 y) { return y->gte(x,y); }


    SI F32 operator+(F32   x, F32   y) { return x->add(x,y); }
    SI F32 operator+(F32   x, float y) { return x->add(x,y); }
    SI F32 operator+(float x, F32   y) { return y->add(x,y); }

    SI F32 operator-(F32   x, F32   y) { return x->sub(x,y); }
    SI F32 operator-(F32   x, float y) { return x->sub(x,y); }
    SI F32 operator-(float x, F32   y) { return y->sub(x,y); }

    SI F32 operator*(F32   x, F32   y) { return x->mul(x,y); }
    SI F32 operator*(F32   x, float y) { return x->mul(x,y); }
    SI F32 operator*(float x, F32   y) { return y->mul(x,y); }

    SI F32 fast_mul(F32   x, F32   y) { return x->fast_mul(x,y); }
    SI F32 fast_mul(F32   x, float y) { return x->fast_mul(x,y); }
    SI F32 fast_mul(float x, F32   y) { return y->fast_mul(x,y); }

    SI F32 operator/(F32   x, F32  y) { return x->div(x,y); }
    SI F32 operator/(float x, F32  y) { return y->div(x,y); }

    SI F32 min(F32   x, F32   y) { return x->min(x,y); }
    SI F32 min(F32   x, float y) { return x->min(x,y); }
    SI F32 min(float x, F32   y) { return y->min(x,y); }

    SI F32 max(F32   x, F32   y) { return x->max(x,y); }
    SI F32 max(F32   x, float y) { return x->max(x,y); }
    SI F32 max(float x, F32   y) { return y->max(x,y); }

    SI I32 operator==(F32   x, F32   y) { return x->eq(x,y); }
    SI I32 operator==(F32   x, float y) { return x->eq(x,y); }
    SI I32 operator==(float x, F32   y) { return y->eq(x,y); }

    SI I32 operator!=(F32   x, F32   y) { return x->neq(x,y); }
    SI I32 operator!=(F32   x, float y) { return x->neq(x,y); }
    SI I32 operator!=(float x, F32   y) { return y->neq(x,y); }

    SI I32 operator< (F32   x, F32   y) { return x->lt(x,y); }
    SI I32 operator< (F32   x, float y) { return x->lt(x,y); }
    SI I32 operator< (float x, F32   y) { return y->lt(x,y); }

    SI I32 operator<=(F32   x, F32   y) { return x->lte(x,y); }
    SI I32 operator<=(F32   x, float y) { return x->lte(x,y); }
    SI I32 operator<=(float x, F32   y) { return y->lte(x,y); }

    SI I32 operator> (F32   x, F32   y) { return x->gt(x,y); }
    SI I32 operator> (F32   x, float y) { return x->gt(x,y); }
    SI I32 operator> (float x, F32   y) { return y->gt(x,y); }

    SI I32 operator>=(F32   x, F32   y) { return x->gte(x,y); }
    SI I32 operator>=(F32   x, float y) { return x->gte(x,y); }
    SI I32 operator>=(float x, F32   y) { return y->gte(x,y); }

    SI I32& operator+=(I32& x, I32 y) { return (x = x + y); }
    SI I32& operator+=(I32& x, int y) { return (x = x + y); }

    SI I32& operator-=(I32& x, I32 y) { return (x = x - y); }
    SI I32& operator-=(I32& x, int y) { return (x = x - y); }

    SI I32& operator*=(I32& x, I32 y) { return (x = x * y); }
    SI I32& operator*=(I32& x, int y) { return (x = x * y); }

    SI F32& operator+=(F32& x, F32   y) { return (x = x + y); }
    SI F32& operator+=(F32& x, float y) { return (x = x + y); }

    SI F32& operator-=(F32& x, F32   y) { return (x = x - y); }
    SI F32& operator-=(F32& x, float y) { return (x = x - y); }

    SI F32& operator*=(F32& x, F32   y) { return (x = x * y); }
    SI F32& operator*=(F32& x, float y) { return (x = x * y); }

    SI F32& operator/=(F32& x, F32   y) { return (x = x / y); }

    SI void assert_true(I32 cond, I32 debug) { cond->assert_true(cond,debug); }
    SI void assert_true(I32 cond, F32 debug) { cond->assert_true(cond,debug); }
    SI void assert_true(I32 cond)            { cond->assert_true(cond); }

    SI void store8  (Ptr ptr, I32 val)                    { val->store8  (ptr, val); }
    SI void store16 (Ptr ptr, I32 val)                    { val->store16 (ptr, val); }
    SI void store32 (Ptr ptr, I32 val)                    { val->store32 (ptr, val); }
    SI void storeF  (Ptr ptr, F32 val)                    { val->storeF  (ptr, val); }
    SI void store64 (Ptr ptr, I32 lo, I32 hi)             { lo ->store64 (ptr, lo,hi); }
    SI void store128(Ptr ptr, I32 x, I32 y, I32 z, I32 w) { x  ->store128(ptr, x,y,z,w); }

    SI I32 gather8 (UPtr ptr, int off, I32 ix) { return ix->gather8 (ptr, off, ix); }
    SI I32 gather16(UPtr ptr, int off, I32 ix) { return ix->gather16(ptr, off, ix); }
    SI I32 gather32(UPtr ptr, int off, I32 ix) { return ix->gather32(ptr, off, ix); }
    SI F32 gatherF (UPtr ptr, int off, I32 ix) { return ix->gatherF (ptr, off, ix); }

    SI I32 gather8 (Uniform u, I32 ix) { return ix->gather8 (u, ix); }
    SI I32 gather16(Uniform u, I32 ix) { return ix->gather16(u, ix); }
    SI I32 gather32(Uniform u, I32 ix) { return ix->gather32(u, ix); }
    SI F32 gatherF (Uniform u, I32 ix) { return ix->gatherF (u, ix); }

    SI F32        sqrt(F32 x) { return x->       sqrt(x); }
    SI F32 approx_log2(F32 x) { return x->approx_log2(x); }
    SI F32 approx_pow2(F32 x) { return x->approx_pow2(x); }
    SI F32 approx_log (F32 x) { return x->approx_log (x); }
    SI F32 approx_exp (F32 x) { return x->approx_exp (x); }

    SI F32 approx_powf(F32   base, F32   exp) { return base->approx_powf(base, exp); }
    SI F32 approx_powf(F32   base, float exp) { return base->approx_powf(base, exp); }
    SI F32 approx_powf(float base, F32   exp) { return  exp->approx_powf(base, exp); }

    SI F32 approx_sin(F32 radians) { return radians->approx_sin(radians); }
    SI F32 approx_cos(F32 radians) { return radians->approx_cos(radians); }
    SI F32 approx_tan(F32 radians) { return radians->approx_tan(radians); }

    SI F32 approx_asin(F32 x) { return x->approx_asin(x); }
    SI F32 approx_acos(F32 x) { return x->approx_acos(x); }
    SI F32 approx_atan(F32 x) { return x->approx_atan(x); }
    SI F32 approx_atan2(F32 y, F32 x) { return x->approx_atan2(y, x); }

    SI F32   clamp01(F32 x) { return x->  clamp01(x); }
    SI F32       abs(F32 x) { return x->      abs(x); }
    SI F32      ceil(F32 x) { return x->     ceil(x); }
    SI F32     fract(F32 x) { return x->    fract(x); }
    SI F32     floor(F32 x) { return x->    floor(x); }
    SI I32    is_NaN(F32 x) { return x->   is_NaN(x); }
    SI I32 is_finite(F32 x) { return x->is_finite(x); }

    SI I32      trunc(F32 x) { return x->      trunc(x); }
    SI I32      round(F32 x) { return x->      round(x); }
    SI I32 pun_to_I32(F32 x) { return x-> pun_to_I32(x); }
    SI F32 pun_to_F32(I32 x) { return x-> pun_to_F32(x); }
    SI F32     to_F32(I32 x) { return x->     to_F32(x); }
    SI I32    to_fp16(F32 x) { return x->    to_fp16(x); }
    SI F32  from_fp16(I32 x) { return x->  from_fp16(x); }

    SI F32 lerp(F32   lo, F32   hi, F32   t) { return lo->lerp(lo,hi,t); }
    SI F32 lerp(F32   lo, F32   hi, float t) { return lo->lerp(lo,hi,t); }
    SI F32 lerp(F32   lo, float hi, F32   t) { return lo->lerp(lo,hi,t); }
    SI F32 lerp(F32   lo, float hi, float t) { return lo->lerp(lo,hi,t); }
    SI F32 lerp(float lo, F32   hi, F32   t) { return hi->lerp(lo,hi,t); }
    SI F32 lerp(float lo, F32   hi, float t) { return hi->lerp(lo,hi,t); }
    SI F32 lerp(float lo, float hi, F32   t) { return  t->lerp(lo,hi,t); }

    SI F32 clamp(F32   x, F32   lo, F32   hi) { return  x->clamp(x,lo,hi); }
    SI F32 clamp(F32   x, F32   lo, float hi) { return  x->clamp(x,lo,hi); }
    SI F32 clamp(F32   x, float lo, F32   hi) { return  x->clamp(x,lo,hi); }
    SI F32 clamp(F32   x, float lo, float hi) { return  x->clamp(x,lo,hi); }
    SI F32 clamp(float x, F32   lo, F32   hi) { return lo->clamp(x,lo,hi); }
    SI F32 clamp(float x, F32   lo, float hi) { return lo->clamp(x,lo,hi); }
    SI F32 clamp(float x, float lo, F32   hi) { return hi->clamp(x,lo,hi); }

    SI I32 operator<<(I32 x, int bits) { return x->shl(x, bits); }
    SI I32        shl(I32 x, int bits) { return x->shl(x, bits); }
    SI I32        shr(I32 x, int bits) { return x->shr(x, bits); }
    SI I32        sra(I32 x, int bits) { return x->sra(x, bits); }

    SI I32 operator&(I32 x, I32 y) { return x->bit_and(x,y); }
    SI I32 operator&(I32 x, int y) { return x->bit_and(x,y); }
    SI I32 operator&(int x, I32 y) { return y->bit_and(x,y); }

    SI I32 operator|(I32 x, I32 y) { return x->bit_or (x,y); }
    SI I32 operator|(I32 x, int y) { return x->bit_or (x,y); }
    SI I32 operator|(int x, I32 y) { return y->bit_or (x,y); }

    SI I32 operator^(I32 x, I32 y) { return x->bit_xor(x,y); }
    SI I32 operator^(I32 x, int y) { return x->bit_xor(x,y); }
    SI I32 operator^(int x, I32 y) { return y->bit_xor(x,y); }

    SI I32& operator&=(I32& x, I32 y) { return (x = x & y); }
    SI I32& operator&=(I32& x, int y) { return (x = x & y); }
    SI I32& operator|=(I32& x, I32 y) { return (x = x | y); }
    SI I32& operator|=(I32& x, int y) { return (x = x | y); }
    SI I32& operator^=(I32& x, I32 y) { return (x = x ^ y); }
    SI I32& operator^=(I32& x, int y) { return (x = x ^ y); }

    SI I32 bit_clear(I32 x, I32 y) { return x->bit_clear(x,y); }
    SI I32 bit_clear(I32 x, int y) { return x->bit_clear(x,y); }
    SI I32 bit_clear(int x, I32 y) { return y->bit_clear(x,y); }

    SI I32 select(I32 c, I32 t, I32 f) { return c->select(c,          t ,          f ); }
    SI I32 select(I32 c, I32 t, int f) { return c->select(c,          t , c->splat(f)); }
    SI I32 select(I32 c, int t, I32 f) { return c->select(c, c->splat(t),          f ); }
    SI I32 select(I32 c, int t, int f) { return c->select(c, c->splat(t), c->splat(f)); }

    SI F32 select(I32 c, F32   t, F32   f) { return c->select(c,          t ,          f ); }
    SI F32 select(I32 c, F32   t, float f) { return c->select(c,          t , c->splat(f)); }
    SI F32 select(I32 c, float t, F32   f) { return c->select(c, c->splat(t),          f ); }
    SI F32 select(I32 c, float t, float f) { return c->select(c, c->splat(t), c->splat(f)); }

    SI I32 extract(I32 x, int bits, I32 z) { return x->extract(x,bits,z); }
    SI I32 extract(I32 x, int bits, int z) { return x->extract(x,bits,z); }
    SI I32 extract(int x, int bits, I32 z) { return z->extract(x,bits,z); }

    SI I32 pack(I32 x, I32 y, int bits) { return x->pack   (x,y,bits); }
    SI I32 pack(I32 x, int y, int bits) { return x->pack   (x,y,bits); }
    SI I32 pack(int x, I32 y, int bits) { return y->pack   (x,y,bits); }

    SI I32 operator~(I32 x) { return ~0 ^ x; }
    SI I32 operator-(I32 x) { return  0 - x; }
    SI F32 operator-(F32 x) { return 0.0f - x; }

    SI F32 from_unorm(int bits, I32 x) { return x->from_unorm(bits,x); }
    SI I32   to_unorm(int bits, F32 x) { return x->  to_unorm(bits,x); }

    SI void store(PixelFormat f, Ptr p, Color c) { return c->store(f,p,c); }

    SI Color gather(PixelFormat f, UPtr p, int off, I32 ix) { return ix->gather(f,p,off,ix); }
    SI Color gather(PixelFormat f, Uniform u     , I32 ix)  { return ix->gather(f,u,ix); }

    SI void   premul(F32* r, F32* g, F32* b, F32 a) { a->  premul(r,g,b,a); }
    SI void unpremul(F32* r, F32* g, F32* b, F32 a) { a->unpremul(r,g,b,a); }

    SI Color   premul(Color c) { return c->  premul(c); }
    SI Color unpremul(Color c) { return c->unpremul(c); }

    SI Color lerp(Color lo, Color hi, F32 t) { return t->lerp(lo,hi,t); }

    SI Color blend(SkBlendMode m, Color s, Color d) { return s->blend(m,s,d); }

    SI Color clamp01(Color c) { return c->clamp01(c); }

    SI HSLA  to_hsla(Color c) { return c->to_hsla(c); }
    SI Color to_rgba(HSLA  c) { return c->to_rgba(c); }

    // Evaluate polynomials: ax^n + bx^(n-1) + ... for n >= 1
    template <typename F32_or_float, typename... Rest>
    SI F32 poly(F32 x, F32_or_float a, float b, Rest... rest) {
        if constexpr (sizeof...(rest) == 0) {
            return x*a+b;
        } else {
            return poly(x, x*a+b, rest...);
        }
    }
#undef SI
}  // namespace skvm

#endif//SkVM_DEFINED
