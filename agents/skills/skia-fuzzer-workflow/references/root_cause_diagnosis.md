# Root Cause Diagnosis & Numerical Precision

This guide covers diagnosing crashes, assertion failures, and memory errors produced by Skia fuzzers, with specific focus on precision handling and debugging techniques.

---

## 1. Stack Trace Analysis & Interactive Debugging

### Running under GDB / LLDB

When the crash stack trace from ClusterFuzz is ambiguous, run the testcase under a debugger:

```bash
# Debugging with LLDB
lldb -- out/DebugFuzz/fuzz -b /path/to/testcase
(lldb) run
(lldb) bt
(lldb) frame select 0

# Debugging with GDB
gdb --args out/DebugFuzz/fuzz -b /path/to/testcase
(gdb) run
(gdb) backtrace
```

### Common Bug Categories in Skia

1. **Floating-Point Instability / Denormals**:
   - Extreme, subnormal, or NaN/Inf coordinates leading to division-by-zero, invalid square roots (`SkScalarSqrt`), or overflow in conic/cubic root solving.
   - Example reference: [ClipCubicTest.cpp:L175-L211](file:///usr/local/google/home/alexisdavidc/ai/stpulcheria/skia/skia/tests/ClipCubicTest.cpp#L175-L211) within `DEF_TEST(test_fuzz_crbug_698714, reporter)`.
2. **Buffer Overrun & Memory Safety (ASAN)**:
   - Reading past allocated bounds during serialized stream decoding or array indexing in raster blitters.
   - Example reference: [RoundRectTest.cpp:L1652-L1680](file:///usr/local/google/home/alexisdavidc/ai/stpulcheria/skia/skia/tests/RoundRectTest.cpp#L1652-L1680) within `DEF_TEST(RRect_fuzzer_regressions, r)`.
3. **Assertion Failures (`SkASSERT`)**:
   - Preconditions or invariants assuming normalized values (e.g. `SkASSERT(weight > 0)`, `SkASSERT(fBounds.isFinite())`, or monotone cubic segments).
4. **Decoder / Parser Failures**:
   - Truncated chunks, infinite loops in parsing headers, or unhandled image color formats.
   - Example reference: [CodecTest.cpp:L1878-L1890](file:///usr/local/google/home/alexisdavidc/ai/stpulcheria/skia/skia/tests/CodecTest.cpp#L1878-L1890) within `DEF_TEST(Codec_ossfuzz6274, r)`.

---

## 2. Capturing Exact Precision: `dumpHex()` and `SkBits2Float`

Fuzzers discover bugs by crafting coordinates that lie directly on edge-case boundaries (e.g. `1e-45`, `0x7f7fffff`, `0x80000000`).

> [!WARNING]
> **Never use standard string formatting (`%f` or `std::cout`) to inspect failing floats.**
> Standard printing rounds small, denormalized values to `0.0000` or truncates mantissas, destroying the exact conditions required to trigger the bug.

### Step 1: Dump Paths as Hexadecimal Bit Patterns

If a crash involves an `SkPath`, call `path.dumpHex()` in temporary debug code:

```cpp
// Insert temporarily near the crash site
path.dumpHex();
```

`dumpHex()` prints executable C++ code using `SkBits2Float(0x...)`:
```cpp
SkPath path = SkPathBuilder()
    .moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000))
    .cubicTo(SkBits2Float(0x43434343), SkBits2Float(0x43434341),
             SkBits2Float(0xb74343bd), SkBits2Float(0x01434343),
             SkBits2Float(0x10434343), SkBits2Float(0x00144332))
    .detach();
```

### Step 2: Logging Matrices and Scalars Scientifically

For transformation matrices or standalone scalars, log values with scientific notation (`%e` or `%.10e`) or print raw integer bits (`SkFloat2Bits`):

```cpp
#include "src/core/SkFloatBits.h"

SkDebugf("Matrix [%.10e, %.10e, %.10e] [bits: 0x%08x, 0x%08x, 0x%08x]\n",
         matrix[0], matrix[1], matrix[2],
         SkFloat2Bits(matrix[0]), SkFloat2Bits(matrix[1]), SkFloat2Bits(matrix[2]));
```

---

## 3. Minimizing the Reproduction

Before writing the unit test, reduce the fuzzer input to the smallest reproducible operation:

1. **Extract Core API Sequence**: Determine which specific call triggers the crash (e.g. `canvas->drawPath()`, `path.computeTightBounds()`, or `SkCubicClipper::clipCubic()`).
2. **Remove Extraneous Verbs**: If the path has 50 verbs, bisect by commenting out segments until only the offending verb (e.g. a single degenerate `cubicTo`) remains.
3. **Isolate Canvas / Clip Setup**: Check if the crash needs a specific clip rect, stroke width, or blend mode. If a default `SkPaint` triggers it, omit all non-essential paint configuration.
