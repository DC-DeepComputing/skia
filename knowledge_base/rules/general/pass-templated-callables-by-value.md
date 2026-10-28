---
name: skia/pass-templated-callables-by-value
description: When declaring functions that accept templated callable or lambda parameters, pass them by value or forwarding reference rather than const reference
rubric: readability
severity: should_fix
---

**Rule.** When declaring functions that accept templated callables, lambdas, or
visitor functors, pass the callable parameters by value (e.g., `Fn fn`) or
forwarding reference (`Fn&& fn`) rather than by const reference (`const Fn&`).
Reserve `const&` parameter passing for heavy, non-inlined wrappers such as
`std::function`.

**Why.** Closures and custom functors passed as template arguments are typically
small, lightweight, and inlined directly by the compiler. Passing them by value
or forwarding reference keeps function signatures clean, enables moving or
mutating captured state when invoked, and avoids unnecessary pointer indirection
without incurring copying overhead.

## Examples

Bad:

```cpp
template <typename AddToBlendKeyT, typename AddSrcToKeyT, typename AddDstToKeyT>
void Blend(const KeyContext& keyContext,
           PaintParamsKeyBuilder* keyBuilder,
           PipelineDataGatherer* gatherer,
           const AddToBlendKeyT& addBlendToKey,
           const AddSrcToKeyT& addSrcToKey,
           const AddDstToKeyT& addDstToKey);
```

Good:

```cpp
template <typename AddToBlendKeyT, typename AddSrcToKeyT, typename AddDstToKeyT>
void Blend(const KeyContext& keyContext,
           PaintParamsKeyBuilder* keyBuilder,
           PipelineDataGatherer* gatherer,
           AddToBlendKeyT addBlendToKey,
           AddSrcToKeyT addSrcToKey,
           AddDstToKeyT addDstToKey);
```
