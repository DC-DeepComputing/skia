# SkRasterPipeline Performance Analysis & Optimization Ideas

Based on static analysis of generated assembly files under `out/perf/skia/control` and performance results in `out/perf/skia/analysis.tsv`, several high-overhead raster pipeline stages have been audited. Below is a detailed breakdown of 20 high-overhead stages, outlining low-hanging fruits, tactical ideas, technical rationale, implementation plans, and associated tradeoffs.

---

## 1. Low-Precision Bilinear Clamp (`lowp::bilerp_clamp_8888`)
* **Performance Impact**: SSE2: 245.20 cyc/iter | AVX2/ML3: 134.13 cyc/iter | AVX-512/ML4: 122.11 cyc/iter
* **Root Cause**: Redundant bounds clamping and row stride multiplications inside loop unrolling.
* **Details**:
  The common implementation of `LOWP_STAGE_GP(bilerp_clamp_8888, ...)` in `src/opts/SkRasterPipeline_opts.h` calls `ix_and_ptr` 4 separate times to resolve the indices of the 4 neighboring pixel queries:
  1. `ix_and_ptr(&ptr, ctx, sx, sy)`
  2. `ix_and_ptr(&ptr, ctx, sx+1, sy)`
  3. `ix_and_ptr(&ptr, ctx, sx, sy+1)`
  4. `ix_and_ptr(&ptr, ctx, sx+1, sy+1)`
  
  Each call to `ix_and_ptr` independently evaluates:
  ```cpp
  U32 ax = cast<U32>(min(max(0, x), w)),
      ay = cast<U32>(min(max(0, y), h));
  return ay * ctx->stride + ax;
  ```
  This performs **4x bounds checks for X, 4x bounds checks for Y, and 4x stride multiplications**. Furthermore, it reassigns the `ptr` pointer 4 times redundantly.
  
* **Optimization Idea**:
  Rewrite the coordinate indexing inside `bilerp_clamp_8888` to precalculate only the unique elements:
  - 2 unique clamped X coordinates: `ax = clamp(sx, 0, w)` and `ax1 = clamp(sx+1, 0, w)`.
  - 2 unique clamped Y coordinates: `ay = clamp(sy, 0, h)` and `ay1 = clamp(sy+1, 0, h)`.
  - 2 unique stride multiplications: `stride0 = ay * stride` and `stride1 = ay1 * stride`.
  
  We can then construct the 4 flat indices directly:
  ```cpp
  U32 ix00 = stride0 + ax,
      ix10 = stride0 + ax1,
      ix01 = stride1 + ax,
      ix11 = stride1 + ax1;
  ```
* **Tradeoffs & Risks**:
  * **Pros**: Reduces bounds checking by **50%** and multiplication operations by **50%**, producing cleaner vectorization and dramatically smaller assembly footprint (especially on SSE2 where 16-bit vector operations are emulated).
  * **Cons/Risks**: None. Since this relies on simple, standard vector operations, it is highly portable and doesn't change color precision or accuracy.

---

## 2. Integer Division Vectorization (`div_4_ints` / `div_4_uints`)
* After manual review, this was deemed not worth.

---

## 3. High-Stop Count Gradients (`gradient` / `evenly_spaced_gradient` lowp)
* **Performance Impact**: AVX2/ML3: ~275 - 339 cyc/iter
* **Root Cause**: Excessive hardware gather instructions when `stopCount > 8`.
* **Details**:
  In `gradient_lookup` inside `src/opts/SkRasterPipeline_opts.h`, if `stopCount <= 8`, it uses in-register permutation `_mm256_permutevar8x32_ps` to lookup factors and biases, which is incredibly fast.
  
  If `stopCount > 8`, it falls back to 8 separate calls to `gather<F>`, each of which executes two `_mm256_i32gather_ps` operations, totaling **16 very slow hardware gather instructions** per iteration.
  
* **Optimization Idea**:
  - **Permute Fast-path Expansion**: Expand the permute-based fast path to handle up to 16 stops (using two loads and two permutes, combined with a blend instruction based on index range). This completely avoids slow `i32gather` instructions for standard gradients between 9 and 16 stops.
  - **Data Interleaving**: Interleave factors and biases in memory such that adjacent channels can be loaded together, reducing the number of individual gather operations.
* **Tradeoffs & Risks**:
  * **Pros**: Drastic reduction in pipeline stalls from memory gather latency.
  * **Cons/Risks**: Slightly increases code size for gradient setup due to dual-permute logic, but the performance payoff of avoiding gathers is substantial.

---

## 4. Transfer Functions (`PQish` / `parametric`)
* **Performance Impact**: SSE2: 301.92 cyc/iter | AVX-512/ML4: 220.83 cyc/iter | AVX2/ML3: 200.93 cyc/iter
* **Root Cause**: Heavy nested polynomial evaluations inside `approx_powf`.
* **Details**:
  `PQish` uses multiple nested `approx_powf` operations to evaluate exponential curves:
  ```cpp
  F r = approx_powf(max(mad(ctx->b, approx_powf(v, ctx->c), ctx->a), 0.0f)
                     / (mad(ctx->e, approx_powf(v, ctx->c), ctx->d)),
                  ctx->f);
  ```
  Each `approx_powf` evaluates polynomial approximations of `log2` and `exp2`.
  
* **Optimization Idea**:
  - **Padé/Chebyshev Approximations**: Instead of evaluating multiple nested general `approx_powf` functions, approximate the entire `PQish` curve directly using a single high-degree rational polynomial or Chebyshev polynomial. This replaces exp/log/pow calls with simple additions and multiplications.
  - **Specialized Fast Paths**: Provide customized, compile-time specialized stages for the most common transfer functions (e.g., sRGB, Rec. 709, PQ, HLG) with pre-baked coefficients and simplified math, rather than running a generic parametric curve.
* **Tradeoffs & Risks**:
  * **Pros**: Substantial latency savings by replacing expensive log/exp-based power calculations with basic arithmetic (adds and muls).
  * **Cons/Risks**: Small potential for color drift/precision loss compared to standard transfer functions. Requires thorough verification against reference color spaces.

---

## 5. Perlin Noise Stage (`perlin_noise`)
* **Performance Impact**: AVX2/ML3: 344.04 cyc/iter | AVX-512/ML4: 257.47 cyc/iter
* **Root Cause**: Serial nested loops over color channels and heavy lattice gathers.
* **Details**:
  The `perlin_noise` stage has an outer loop iterating `numOctaves` times, inside of which there is a nested loop iterating 4 times (for channels R, G, B, A), making multiple lattice gathers and Perlin vector computations.
  
* **Optimization Idea**:
  - **SIMD Channel Parallelism**: Since Perlin noise computes 4 independent channels, we can compute all 4 channels simultaneously by packing the channel coefficients into a single SIMD vector register and applying vector operations across the lanes, completely eliminating the nested 4-iteration channel loop.
  - **Lattice Pre-unrolling**: Pre-unroll and align the noise tables and lattice selector to allow wider, aligned loads, avoiding scalar-to-vector gathering overhead.
* **Tradeoffs & Risks**:
  * **Pros**: Up to 4x reduction in calculations per octave.
  * **Cons/Risks**: Highly complex refactoring of the internal vector structure of the Perlin stage, requiring careful handling of noise data memory layouts.

---

## 6. Color blending (`hue` / `saturation` / `color` / `luminosity`)
* **Performance Impact**: SSE2: ~80-145 cyc/iter | AVX2/ML3: ~76-105 cyc/iter
* **Root Cause**: Redundant color conversion computations in sequence (`set_sat` + `set_lum` + `clip_color`).
* **Details**:
  For example, `hue` performs:
  ```cpp
  set_sat(&R, &G, &B, sat(dr,dg,db)*a);
  set_lum(&R, &G, &B, lum(dr,dg,db)*a);
  clip_color(&R,&G,&B, a*da);
  ```
  `set_sat` performs expensive scaling on `R, G, B`. Then `set_lum` immediately calculates `lum(*r, *g, *b)` on the *newly modified* `R, G, B` values to adjust luminance:
  ```cpp
  F diff = l - lum(*r, *g, *b);
  ```
  This requires a full vector dot product (`lum(*r, *g, *b)`).
  
* **Optimization Idea**:
  Mathematically fuse `set_sat` and `set_lum`! 
  We can analytically prove that:
  $$\text{lum}(R_{new}, G_{new}, B_{new}) = s \cdot (\text{lum}(R_{old}, G_{old}, B_{old}) - \text{mn}_{old})$$
  This is 100% mathematically exact! By using this analytical formula, we can compute the new luminance with **just a single subtraction and multiplication**, completely skipping the expensive `lum(*r, *g, *b)` dot-product calculation on the modified registers.
* **Tradeoffs & Risks**:
  * **Pros**: Fuses two complex helper stages, eliminating redundant dot-product calculations entirely. Extremely high-value and mathematically pure.
  * **Cons/Risks**: None. It is mathematically identical, meaning exact precision parity with existing tests.

---

## 7. Dynamic Matrix Multiply (`matrix_multiply_4` / `matrix_multiply_3` / `matrix_multiply_2`)
* **Performance Impact**: AVX2/ML3: 77.28 cyc/iter | SSE2: 79.27 cyc/iter
* **Root Cause**: Run-time dynamic loop bounds for matrix columns and rows preventing compiler unrolling.
* **Details**:
  In `matrix_multiply`, columns and rows (`outColumns` and `outRows`) are read from the runtime context (`ctx.rightColumns`, `ctx.leftRows`), causing the compiler to emit generic loop structures with dynamic loop branching, pointer math, and serial dependencies. This blocks SIMD out-of-order execution pipelines.
  
* **Optimization Idea**:
  Specialize `matrix_multiply` for common matrix-vector and matrix-matrix operations:
  ```cpp
  if (outColumns == 1 && outRows == 4) {
      // Direct, fully unrolled matrix-vector multiply (very common for transform)
  } else if (outColumns == 4 && outRows == 4) {
      // Direct, fully unrolled 4x4 matrix-matrix multiply
  } else {
      // Fallback dynamic loop
  }
  ```
* **Tradeoffs & Risks**:
  * **Pros**: Zero loop branches and dynamic pointer calculations for standard transforms, allowing Clang to generate direct, pipelined, back-to-back `vfmadd` FMA instructions.
  * **Cons/Risks**: Slightly increases compiled stage code size.

---

## 8. Byte Table Lookups (`byte_tables`)
* **Performance Impact**: AVX-512/ML4: 143.26 cyc/iter | AVX2/ML3: 80.23 cyc/iter
* **Root Cause**: Lack of SIMD hardware gather support for 8-bit types.
* **Details**:
  Because x86 SIMD instruction sets do not support vector gathers for 8-bit `uint8_t` values, `byte_tables` has to be scalarized into **16 separate scalar memory lookups** on ML4, or **8 lookups** on ML3, which are then packed into vector registers. This makes ML4 twice as slow as ML3 due to twice as many scalar loads.
  
* **Optimization Idea**:
  - **In-Register Byte Permutation**: Since each table is only 256 bytes, we can fit the table into 4 vector registers and perform lookups using the AVX-512 VBMI `vpermi2b` (or AVX2 byte shuffles and blends) entirely in-register, bypassing memory tables.
  - **Float Table Promotion**: Expand/promote tables to 32-bit floats during shader compilation. Doing gathers on float tables allows using native hardware `_mm512_i32gather_ps` or `_mm256_i32gather_ps` which execute efficiently in hardware.
* **Tradeoffs & Risks**:
  * **Pros**: Up to 4-10x speedup by replacing slow scalar memory loads with fast vectorized hardware operations.
  * **Cons/Risks**: Promoting tables to `float` increases table size from 256 bytes to 1 KB (but this easily fits in L1 cache).

---

## 9. Color Space Gamma curves (`gamma_`)
* **Performance Impact**: AVX-512/ML4: 90.34 cyc/iter | AVX2/ML3: 77.26 cyc/iter
* **Root Cause**: Generic exp/log-based generic exponentiation.
* **Details**:
  Generic `gamma_` stage calls `approx_powf(v, *G)`, which evaluates polynomial log2 and exp2 approximations.
  
* **Optimization Idea**:
  Identify and specialize common gamma exponents at runtime or shader compilation:
  - If `G == 2.0f`, simplify to `v * v` (a single multiplication!).
  - If `G == 1.0f`, compile as a no-op.
  - If `G == 0.5f` (square root), use the native hardware vector square root instruction `_mm256_sqrt_ps` or `_mm512_sqrt_ps`, which has extremely high hardware throughput.
* **Tradeoffs & Risks**:
  * **Pros**: Dramatic speedup (up to 10x) for standard gamma 2.0 or 0.5 transforms.
  * **Cons/Risks**: None.

---

## 10. Ordered Dithering (`dither`)
* **Performance Impact**: SSE2: ~18 cyc/iter | AVX2/ML3: ~12 cyc/iter
* **Root Cause**: Heavy bitwise permutation math per pixel.
* **Details**:
  Dithering performs 6 bitwise AND, shift-left, and shift-right operations, followed by ORs to shuffle the low-order bits of X and Y coordinate vectors:
  ```cpp
  U32 M = (Y & 1) << 5 | (X & 1) << 4
        | (Y & 2) << 2 | (X & 2) << 1
        | (Y & 4) >> 1 | (X & 4) >> 2;
  ```
* **Optimization Idea**:
  Since ML3 and ML4 support the BMI2 instruction set, use parallel bit deposit (`_pdep_u32`) to shuffle coordinate bits in a single CPU clock cycle!
* **Tradeoffs & Risks**:
  * **Pros**: Replaces 11 bitwise operations with 2 parallel bit deposit operations.
  * **Cons/Risks**: Limited to CPU architectures supporting BMI2 (which are guaranteed for ML3/ML4 targets).

---

## 11. Ternary Smoothstep (`smoothstep_n_floats`)
* **Performance Impact**: ~35 cyc/iter
* **Root Cause**: Division operation inside ternary evaluation.
* **Details**:
  `smoothstep_fn` performs division to normalize coordinates:
  ```cpp
  F t = clamp_01_((*x - *edge0) / (*edge1 - *edge0));
  ```
  Vector division has high latency on modern CPUs.
  
* **Optimization Idea**:
  If `edge0` and `edge1` are constant uniforms, precalculate `1.0f / (edge1 - edge0)` in the CPU and pass it to the shader, replacing division with multiplication!
* **Tradeoffs & Risks**:
  * **Pros**: Substantial throughput improvement.
  * **Cons/Risks**: Requires uniform tracking at the pipeline building stage.

---

## 12. Bicubic Filtering (`bicubic_clamp_8888` / `bicubic_setup`)
* **Performance Impact**: SSE2: 156.13 cyc/iter | ML4: 119.83 cyc/iter
* **Root Cause**: Heavy weight calculations and redundant bounds clamping.
* **Details**:
  The bicubic shader clamps coordinates for 16 separate coordinate taps, invoking complex boundaries checking.
  
* **Optimization Idea**:
  Fuse bicubic setup weight calculations and use coordinate offsetting relative to clamped center coordinates.
* **Tradeoffs & Risks**:
  * **Pros**: Reduces redundant bounds checks from 16 to 4.
  * **Cons/Risks**: High coordinate indexing complexity.

---

## 13. HLG Curve Mapping (`HLGish` / `HLGinvish`)
* **Performance Impact**: SSE2: ~132 cyc/iter | AVX2/ML3: ~104 cyc/iter
* **Root Cause**: Generalized power approximation curves.
* **Details**:
  Uses `approx_powf` and `approx_exp` inside branch-selection helpers.
  
* **Optimization Idea**:
  Use custom rational Padé approximations to map HLG color values, avoiding general log/exp.
* **Tradeoffs & Risks**:
  * **Pros**: Significant instruction count savings.
  * **Cons/Risks**: Potential color precision drift.

---

## 14. Color Transform Operations (`ootf`)
* **Performance Impact**: ML4: 130.11 cyc/iter | SSE2: 106.18 cyc/iter
* **Root Cause**: Multiple back-to-back transfer function evaluations.
* **Details**:
  Evaluates PQ-to-linear, multiplies by matrix, then maps back.
  
* **Optimization Idea**:
  Pre-combine and bake color conversion curves inside the pipeline builder to fuse operations.
* **Tradeoffs & Risks**:
  * **Pros**: Eliminates multiple intermediate stages.
  * **Cons/Risks**: Increases pipeline builder complexity.

---

## 15. CSS Color Spaces (`css_hsl_to_srgb` / `css_hwb_to_srgb`)
* **Performance Impact**: SSE2: ~95-106 cyc/iter
* **Root Cause**: Dynamic float modulos and branch selections.
* **Details**:
  Calls `mod_(h, 360)` which does heavy float conversions and divisions.
  
* **Optimization Idea**:
  Avoid float modulo using custom reciprocal multipliers or range-bound additions when `h` is known to be positive.
* **Tradeoffs & Risks**:
  * **Pros**: High performance gains in HSL converters.
  * **Cons/Risks**: None.

---

## 16. Matrix Inversion (`inverse_mat4` / `inverse_mat3` / `inverse_mat2`)
* **Performance Impact**: SSE2: ~103 cyc/iter | ML4: ~82 cyc/iter
* **Root Cause**: Cramer's rule/cofactor expansion leading to severe register pressure and serial chains.
* **Details**:
  Computes 12 sub-determinants and a main determinant in a highly serial fashion.
  
* **Optimization Idea**:
  Use row-reduction or PLU decomposition for 4x4 matrix inversion to reduce the raw floating-point instruction count.
* **Tradeoffs & Risks**:
  * **Pros**: Reduces instruction count and register pressure.
  * **Cons/Risks**: Minor loss of numeric precision for near-singular matrices.

---

## 17. Unaligned Texture Gathering (`gather_4444` / `gather_565` / `gather_8888`)
* **Performance Impact**: ~80-91 cyc/iter
* **Root Cause**: Alignment penalties on SIMD memory loads.
* **Details**:
  Performs unaligned loads when reading pixels from textures.
  
* **Optimization Idea**:
  Use aligned SIMD loads when texture strides are multiples of 32/64 bytes.
* **Tradeoffs & Risks**:
  * **Pros**: Speedup in texture sampling.
  * **Cons/Risks**: Requires padding texture allocations.

---

## 18. Boundary Coordinate Clamping (`clamp_x_and_y`)
* **Performance Impact**: ~45 cyc/iter
* **Root Cause**: Serial evaluation of `min` and `max` constraints.
* **Details**:
  Independently clamps coordinate ranges.
  
* **Optimization Idea**:
  Leverage FMA instructions to scale and clamp coordinates in a single step or use vectorized SIMD bounds instructions.
* **Tradeoffs & Risks**:
  * **Pros**: Highly compact code.
  * **Cons/Risks**: None.

---

## 19. Interpolation Accumulation (`accumulate`)
* **Performance Impact**: ~51 cyc/iter
* **Root Cause**: Instruction latency on serial additions.
* **Details**:
  Iteratively accumulates bicubic filtering results.
  
* **Optimization Idea**:
  Unroll accumulation loops and use associative reordering to enable pipelined out-of-order execution.
* **Tradeoffs & Risks**:
  * **Pros**: Maximizes execution ports.
  * **Cons/Risks**: None.

---

## 20. Gaussian Alpha Approximation (`gauss_a_to_rgba`)
* **Performance Impact**: ~14 cyc/iter
* **Root Cause**: Quartic polynomial evaluation.
* **Details**:
  Uses a quartic polynomial approximation for Gaussian blur alpha.
  
* **Optimization Idea**:
  Use fused multiply-add (FMA) instructions to evaluate the Horner's scheme, and reuse intermediate registers.
* **Tradeoffs & Risks**:
  * **Pros**: Extremely fast evaluation.
  * **Cons/Risks**: None.
