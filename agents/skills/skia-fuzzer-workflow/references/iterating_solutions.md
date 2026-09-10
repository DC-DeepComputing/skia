# Iterating on Solutions & Performance Benchmarking

A fuzzer crash usually has multiple possible fixes. Choosing the optimal solution requires evaluating trade-offs between correctness, performance overhead on hot paths, architectural cleanliness, and cross-backend consistency.

---

## 1. Solution Archetypes

When diagnosing a fuzzer issue, formulate at least two distinct approaches from the following archetypes:

### Approach A: Root-Cause Mathematical / Algorithmic Fix
*Fix the underlying mathematical formulation to handle degenerate inputs gracefully.*
- **Examples**:
  - Rewriting quadratic or cubic solvers using stable numerical formulations (e.g. avoiding catastrophic cancellation `b*b - 4*a*c`).
  - Clamping discriminant values to `>= 0.0f` when precision drift makes them slightly negative.
  - Adding epsilon thresholds (`sk_floats_almost_equal`) for nearly collinear points.
- **Pros**: Solves the actual flaw; improves overall robustness for both fuzzer and realistic extreme geometries.
- **Cons**: Requires rigorous mathematical verification; can subtly alter output curves if tolerances are too broad.

### Approach B: Upstream Validation / Early Rejection
*Reject or sanitize invalid data as early as possible before it reaches core geometry or rendering algorithms.*
- **Examples**:
  - Rejecting non-finite (`!SkScalarIsFinite()`) coordinates or dimensions during deserialization (`readFromMemory`).
  - Returning false or empty early if bounding boxes exceed valid coordinate limits.
- **Pros**: Zero performance overhead on downstream rendering; low blast radius.
- **Cons**: Only protects callers passing through that specific API or deserializer; does not protect internal callers.

### Approach C: Defensive Local Guard
*Add an early return, fallback, or clamp at the immediate point of failure.*
- **Examples**:
  - Guarding against zero division: `if (SkScalarNearlyZero(divisor)) { return fallback; }`.
  - Checking array indices or pointer bounds before dereferencing.
- **Pros**: Minimal code changes, immediate bug resolution, virtually zero regression risk to existing code.
- **Cons**: Might paper over an invariant violation created earlier in the pipeline, masking deeper structural flaws.

---

## 2. Evaluation Matrix

Evaluate each candidate solution against these five criteria:

| Criterion | Key Questions |
| :--- | :--- |
| **1. Correctness** | Does the fix completely eliminate the crash/sanitizer failure under ASAN/MSAN/UBSAN? Does it properly handle NaN, infinity, denormals, and edge values? |
| **2. Performance Impact** | Does the fix sit on a hot rendering path (e.g. scanline rasterization, vertex generation, inner loops)? Does it introduce branches, divisions, or square roots? |
| **3. Blast Radius** | Does the change alter rendering output for valid paths? Run GMs (`dm --src gm`) to ensure zero visual regression. |
| **4. Architectural Cleanliness** | Does it preserve Skia's class invariants and design idioms? Is the code self-documenting with clear explanatory comments? |
| **5. Cross-Backend Consistency** | Does the fix ensure consistent behavior across CPU Raster, Ganesh (legacy GPU), and Graphite (modern GPU)? |

---

## 3. Performance Benchmarking with `nanobench`

If any candidate solution modifies a hot path (e.g. `src/core/SkPath.cpp`, `src/core/SkScan_Path.cpp`, `src/core/SkGeometry.cpp`, or blitters), benchmark the performance impact.

> [!TIP]
> **Always use an optimized Release build for benchmarking.** Debug builds contain assertions and unoptimized code that distort timings.

### Step 1: Set Up Release Build

```bash
./bin/gn gen -q out/Release --args='is_debug=false'
ninja --quiet -C out/Release nanobench
```

### Step 2: Establish a Baseline Binary

Copy the unpatched release binary to a control location outside the build directory:

```bash
cp out/Release/nanobench ./nanobench_control
```

### Step 3: Apply the Candidate Fix and Rebuild

Apply the patch and recompile:

```bash
ninja --quiet -C out/Release nanobench
```

### Step 4: Run Comparative Benchmarks

Filter benchmarks relevant to the touched subsystem (e.g. path operations, clipping, lines):

```bash
# Run baseline
./nanobench_control --match path_ --samples 15 --ms 100 > baseline.txt

# Run candidate patch
out/Release/nanobench --match path_ --samples 15 --ms 100 > candidate.txt
```

Compare the mean and median timings between `baseline.txt` and `candidate.txt` to confirm no regression.

---

## 4. Full Regression Verification Checklist

Before finalizing any candidate solution:

1. **Reproduction Unit Test**:
   ```bash
   ninja --quiet -C out/Debug dm
   out/Debug/dm --src tests --match <your_test_name>
   ```
   *Expected: PASS (exits 0 with no errors reported).*

2. **Original Fuzzer Tool**:
   ```bash
   ninja --quiet -C out/ASAN fuzz
   out/ASAN/fuzz -b /path/to/testcase
   ```
   *Expected: Completes cleanly without crashing or sanitizer violation.*

3. **Subsystem Unit Tests & GMs**:
   ```bash
   out/Debug/dm --src tests gm --match <affected_subsystem>
   ```
   *Expected: All existing tests and GMs pass without regression.*
