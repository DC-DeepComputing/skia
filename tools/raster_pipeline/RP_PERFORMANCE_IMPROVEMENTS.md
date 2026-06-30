# SkRasterPipeline Incremental Performance Optimization Manual

This document provides a rigorous, step-by-step methodology for an optimization agent to incrementally implement, verify, and document the performance ideas listed in `RP_PERFORMANCE_IDEAS.md`. 

Each optimization task must follow a strict **Research -> Strategy -> Static Analysis -> Implementation -> Re-Verification -> Benchmarking** cycle. Follow these instructions verbatim to ensure maximum reliability and technical integrity.

---

## 1. Overview of the Optimization Process

For each optimization idea (Task $N$, from 1 to 20):
1. **Setup Workspace**: Create a dedicated workspace directory `tools/raster_pipeline/taskN`.
2. **Benchmark Strategy**: Identify or create a microbenchmark (e.g. similar to `bench/SkRPDivBench.cpp`) to exercise the target stage.
3. **Compile Baseline Controls**: Use the "SkOpts Hack" to compile and preserve baseline control binaries for each target architecture level (SSE/Default, AVX2/ML3, AVX-512/ML4).
4. **Extract Assembly**: Use the Gemini skill `find-asm-symbol` to disassemble and isolate the target stage.
5. **Static Analysis (llvm-mca)**: Run LLVM Machine Code Analyzer to predict baseline instruction cycles, IPC, and port contention.
6. **Hypothesize and Code**: Apply targeted mathematical or structural changes in `src/opts/SkRasterPipeline_opts.h`.
7. **Recompile & Verify Static Improvements (FIRST)**: Inspect disassembly and run `llvm-mca` first to verify that instruction scheduling and bottlenecks have improved.
8. **Run Benchmark & Verify**: Execute `nanobench` to get empirical timings and confirm exact numerical correctness.
9. **Archive & Document**: Save final assemblies, control vs. optimized binaries, and document the improvements in this file.

---

## 2. Environment & Isolating CPU Architectures

Your host machine supports AVX-512 (`ml4`), which means it can natively execute and test all AVX-512, AVX2 (`ml3`), AVX, and SSE vector paths.

### The "SkOpts Hack" for Target Isolation
When you compile Skia, the runtime dynamically detects CPU support and overrides default/fallback stages with the highest available instruction set. To force Skia to compile and run specific lower architectural paths for verification, utilize the following hack in `src/core/SkOpts.cpp`:

1. **AVX-512 (ML4) Path**:
   Ensure `Init_ml4()` is enabled:
   ```cpp
   #if (SK_CPU_X64_LEVEL < SK_CPU_X64_LEVEL_ML4)
       #if defined(SK_ENABLE_AVX512_OPTS) && !defined(SK_DISABLE_AVX512_OPTS)
           if (SkCpu::Supports(SkX64::ML4)) { Init_ml4(); }
       #endif
   #endif
   ```
2. **AVX2 (ML3) Path**:
   To test the AVX2 path, disable/comment out the `Init_ml4()` block in `src/core/SkOpts.cpp`:
   ```cpp
   // Comment out to prevent AVX-512 initialization:
   // if (SkCpu::Supports(SkX64::ML4)) { Init_ml4(); }
   ```
   This forces Skia to fall back to `Init_ml3()`.
3. **SSE/SSE2 (Default) Path**:
   To test the SSE path, comment out both `Init_ml4()` and `Init_ml3()` blocks in `src/core/SkOpts.cpp`:
   ```cpp
   // Comment out to force SSE/Default path:
   // if (SkCpu::Supports(SkX64::ML3)) { Init_ml3(); }
   // if (SkCpu::Supports(SkX64::ML4)) { Init_ml4(); }
   ```

---

## 3. Step-by-Step Execution Guide

### Step 1: Discover / Create Microbenchmark
Identify or create a benchmark that isolates the stage you are optimizing.
* **Inspect existing benchmarks** under `bench/` (e.g. `bench/SkRPDivBench.cpp` for division).
* **Create a new benchmark** if none exists. Save it as `bench/SkRP<Name>Bench.cpp`.
* Example structure of a custom benchmark targeting a stage (such as `bilerp_clamp_8888` or `hue`):
  ```cpp
  #include "bench/Benchmark.h"
  #include "include/core/SkString.h"
  #include "src/core/SkRasterPipeline.h"
  #include <memory>

  class SkRPBilerpClampBench : public Benchmark {
  public:
      SkRPBilerpClampBench() {
          fName.printf("skrp_bilerp_clamp_8888");
          // Initialize aligned mock texture data/context ...
      }
  protected:
      const char* onGetName() override { return fName.c_str(); }
      bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }
      void onDraw(int loops, SkCanvas*) override {
          SkArenaAlloc alloc(1024);
          SkRasterPipeline p(&alloc);
          // Append the specific target op (e.g., bilerp_clamp_8888) with mock context
          p.append(SkRasterPipelineOp::bilerp_clamp_8888, &fCtx);
          for (int i = 0; i < loops; i++) {
              p.run(0, 0, 1024, 1);
          }
      }
  private:
      SkString fName;
      // Define context parameters here
  };
  DEF_BENCH(return new SkRPBilerpClampBench();)
  ```

### Step 2: Compile and Archive Baseline Control Binaries
You must build under **Release mode** using the configured GN arguments inside `out/Release`. Always use quiet flags (`-q` or `--quiet`) with ninja to avoid polluting output log buffers.
1. Compile the baseline binary quietly:
   ```bash
   ninja -q -C out/Release nanobench
   ```
2. Isolate and save control binaries for each target architecture you intend to optimize:
   - For AVX-512: Copy `out/Release/nanobench` to `tools/raster_pipeline/taskN/nanobench_ml4_control`.
   - Comment out `Init_ml4()` in `src/core/SkOpts.cpp`, recompile quietly (`ninja -q -C out/Release nanobench`), and copy to `tools/raster_pipeline/taskN/nanobench_ml3_control`.
   - Comment out both `Init_ml4()` and `Init_ml3()`, recompile quietly, and copy to `tools/raster_pipeline/taskN/nanobench_default_control`.
   *Remember to revert any temporary modifications to `src/core/SkOpts.cpp` when switching stages.*

### Step 3: Extract and Clean Baseline Assembly
Identify the mangled stage symbol in the compiled binary and extract it.
1. Use `nm` and `c++filt` to locate the target function symbol inside the control binary:
   ```bash
   nm tools/raster_pipeline/taskN/nanobench_ml4_control | grep -i bilerp_clamp_8888 | c++filt
   ```
   *Note*: Stage functions compiled under the `ml4` namespace are prefixed with `ml4::`, and under `ml3` with `ml3::`. For example, `ml4::lowp::bilerp_clamp_8888` or `ml4::bilerp_clamp_8888`.
2. Use the `find-asm-symbol` skill script to disassemble the function and clean the output:
   ```bash
   python3 /usr/local/google/home/kjlubick/.gemini/skills/find-asm-symbol/scripts/find_asm.py tools/raster_pipeline/taskN/nanobench_ml4_control "ml4::lowp::bilerp_clamp_8888" > tools/raster_pipeline/taskN/bilerp_clamp_ml4_control.s
   ```

### Step 4: Run Static Performance Analysis (llvm-mca)
Analyze the control assembly statically before editing any code:
1. First, use `clean_asm.py` to strip out debugging annotations, addresses, and comments:
   ```bash
   python3 /usr/local/google/home/kjlubick/.gemini/skills/llvm-mca-analysis/scripts/clean_asm.py < tools/raster_pipeline/taskN/bilerp_clamp_ml4_control.s > tools/raster_pipeline/taskN/bilerp_clamp_ml4_control_clean.s
   ```
2. Run `llvm-mca` using the appropriate target microarchitecture CPU model:
   * **For AVX-512 (ML4)**: Use `-mcpu=x86-64-v4` (Intel Skylake-AVX512/IceLake) or `-mcpu=znver4` (AMD Zen 4).
   * **For AVX2 (ML3)**: Use `-mcpu=x86-64-v3` (Haswell/Broadwell).
   * **For SSE/SSE4**: Use `-mcpu=x86-64-v2`.
   ```bash
   llvm-mca -mcpu=x86-64-v4 tools/raster_pipeline/taskN/bilerp_clamp_ml4_control_clean.s > tools/raster_pipeline/taskN/mca_ml4_control.txt
   ```
3. Record the high-level metrics in your progress log:
   - Estimated Cycles: (e.g. `Total Cycles: 122`)
   - Instruction Throughput: (e.g. `Block RThroughput: 12.5`)
   - IPC: (e.g. `IPC: 1.82`)

### Step 5: Formulate Hypothesis & Apply Code Edits
Develop a clear optimization plan based on `RP_PERFORMANCE_IDEAS.md` and the static bottlenecks (e.g., port congestion on execution ports 0/1/5, high-latency division/gather instructions).
* Open `src/opts/SkRasterPipeline_opts.h` and locate the stage function.
* Implement the proposed optimization surgically (e.g., pre-compute common stride factors, use BMI2 instructions, fuse/simplify equations).
* Ensure there are **no functional regressions** or precision errors.

### Step 6: Recompile, Inspect Assembly, and Verify MCA (FIRST)
Do not jump straight to running benchmarks. You must first statically verify that your code change compiled into superior, clean assembly code:
1. Recompile `nanobench` quietly with the optimization applied:
   ```bash
   ninja -q -C out/Release nanobench
   ```
2. Extract the optimized assembly:
   ```bash
   python3 /usr/local/google/home/kjlubick/.gemini/skills/find-asm-symbol/scripts/find_asm.py out/Release/nanobench "ml4::lowp::bilerp_clamp_8888" > tools/raster_pipeline/taskN/bilerp_clamp_ml4_optimized.s
   ```
3. **Manually Inspect and Analyze the Assembly**:
   Read the disassembly of both the control and optimized assembly side-by-side. Think critically and verify:
   * **Has instruction count decreased?**
   * **Is the vector register usage more efficient?** Look for spills (unnecessary push/pop/move to memory) and redundant loads.
   * **Are expensive operations minimized?** Confirm that loops are unrolled properly, divisions are avoided or vectorized, and redundant coordinate clamps are eliminated.
   * **Does the compiler generate optimal vector instructions** (e.g., FMA instructions `vfmadd213ps`, or single-cycle shuffles instead of general gathers)?
4. Run `llvm-mca` on the optimized assembly to get simulated cycle numbers:
   ```bash
   python3 /usr/local/google/home/kjlubick/.gemini/skills/llvm-mca-analysis/scripts/clean_asm.py < tools/raster_pipeline/taskN/bilerp_clamp_ml4_optimized.s > tools/raster_pipeline/taskN/bilerp_clamp_ml4_optimized_clean.s
   llvm-mca -mcpu=x86-64-v4 tools/raster_pipeline/taskN/bilerp_clamp_ml4_optimized_clean.s > tools/raster_pipeline/taskN/mca_ml4_optimized.txt
   ```
5. Compare `mca_ml4_optimized.txt` with `mca_ml4_control.txt`.
   * **Is the predicted cycle count lower?**
   * **Are there fewer execution port bottlenecks?**
   * **If NO / Regressed**: Re-evaluate your optimization strategy, identify where the compiler failed to optimize, adjust your C++ code, and repeat Step 5-6.
   * **If YES**: Proceed to Step 7.

### Step 7: Run Benchmarks and Verify Speedup
Once static analysis and manual assembly inspection confirm a speedup:
1. Run `nanobench` targeting only your specific benchmark to get empirical numbers:
   ```bash
   out/Release/nanobench --match skrp_bilerp_clamp_8888 --nonrendering
   ```
2. Verify correctness. Run tests or draw commands to make sure output pixels are 100% accurate and match reference values.
3. If everything is correct and faster, compile and archive the final optimized binary for users to verify:
   - For AVX-512: Copy `out/Release/nanobench` to `tools/raster_pipeline/taskN/nanobench_ml4_with_final_opts`.
   - Comment out `Init_ml4()` in `src/core/SkOpts.cpp`, recompile quietly (`ninja -q -C out/Release nanobench`), and copy to `tools/raster_pipeline/taskN/nanobench_ml3_with_final_opts`.

### Step 8: Document Results
Fill out the progress register below with your findings, describing the exact changes, llvm-mca metrics, and empirical nanobench timings.

---

## 4. Optimization Progress Register

Use the following table template to record findings as each task is tackled.

| Task # | Stage Name | Target SIMD | Control Cycles (mca) | Optimized Cycles (mca) | MCA Speedup (%) | Nanobench Speedup (%) | Key Technique Used |
|---|---|---|---|---|---|---|---|
| **1** | `lowp::bilerp_clamp_8888` | `ml4` / `ml3` | 121.11 (ml4) | 106.50 (ml4) | **12.1%** | **27.2%** (micro)<br>**14.0%-15.6%** (draw) | Hardware rounding (`vcvtps2dq`), power-of-two stride shifts globally in `ix_and_ptr`, and native 512-bit Fused Weight Bilinear Blend (Strategy 1). |
| **3** | `gradient_lookup` | | | | | | |
| **4** | `PQish` / `parametric` | | | | | | |
| **5** | `perlin_noise` | | | | | | |
| **6** | `hue`/`saturation`/`color` | | | | | | |
| **7** | `matrix_multiply` | | | | | | |
| **8** | `byte_tables` | | | | | | |
| **9** | `gamma_` | | | | | | |
| **10**| `dither` | | | | | | |
| **11**| `smoothstep_n_floats`| | | | | | |

*(Add other tasks as they are incrementally completed.)*
