---
name: skia/avoid-function-local-static-for-parameterized-results
description: When computing or caching results that depend on runtime parameters, avoid single function-local static variables; use an explicit lookup table, cache, or dynamic evaluation
rubric: correctness
severity: should_fix
---

**Rule.** Do not use a single function-local static variable to memoize or cache
results in a function whose output depends on runtime parameters. When
converting a templated function to accept runtime arguments or adding
parameter-dependent initialization, replace the single static variable with an
explicit cache structure (such as an array or lookup map indexed by the
parameter values) or evaluate the result dynamically on each invocation.

**Why.** In C++, a function-local static variable is initialized only once on
first execution and retains its value across all subsequent invocations. In a
templated function, each template specialization possesses a distinct static
variable; converting template parameters to runtime arguments while leaving a
single function-local static variable causes subsequent calls with different
arguments to erroneously reuse the first call's cached result.

## Examples

Bad:

```cpp
sk_sp<SkRuntimeEffect> noise_effect(unsigned loops, NoiseFilter filter, NoiseFractal fractal) {
    // BUG: Evaluated once on first invocation; ignores arguments on all subsequent calls.
    static const SkRuntimeEffect* effect =
            make_noise_effect(loops,
                              gFilters[static_cast<size_t>(filter)],
                              gFractals[static_cast<size_t>(fractal)])
            .release();
    return sk_ref_sp(effect);
}
```

Good:

```cpp
sk_sp<SkRuntimeEffect> noise_effect(float octaves, NoiseFilter filter, NoiseFractal fractal) {
    static SkRuntimeEffect* kEffectCache[std::size(gLoopBins)]
                                        [std::size(gFilters)]
                                        [std::size(gFractals)];
    const size_t bin = bin_index(octaves);
    auto& effect = kEffectCache[bin]
                               [static_cast<size_t>(filter)]
                               [static_cast<size_t>(fractal)];
    if (!effect) {
        effect = make_noise_effect(gLoopBins[bin].loops,
                                   gFilters[static_cast<size_t>(filter)],
                                   gFractals[static_cast<size_t>(fractal)])
                 .release();
    }
    return sk_ref_sp(effect);
}
```
