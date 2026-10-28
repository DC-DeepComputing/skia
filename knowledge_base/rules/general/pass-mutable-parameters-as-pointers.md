---
name: skia/pass-mutable-parameters-as-pointers
description: When declaring function parameters, pass mandatory objects by const reference, optional objects as const pointers (avoiding const std::optional<T>&), and mutable parameters as pointers
rubric: readability
severity: should_fix
---

**Rule.** Pass mandatory constant object parameters by const reference (const
T&) and primitive scalars by value. Pass optional constant objects as raw const
pointers (const T* = nullptr) rather than const std::optional<T>&; never pass
optional objects by reference or test &ref for null (which is undefined behavior
that compilers optimize away). Pass mutable parameters and optional outputs as
pointers (such as T* or T** defaulting to nullptr), avoiding non-const
references and std::optional<std::reference_wrapper>. For mutable arrays, pass
base skia_private::TArray<T>*. For optional value types (scalars, chrono
durations), use std::optional<T>.

**Why.** Const references prevent copying while scalars pass efficiently in
registers. In C++, references are assumed non-null; binding a null reference and
checking &ref for null is undefined behavior that compilers optimize away. Using
const pointers for optional objects communicates nullability safely and avoids
temporary std::optional wrapper construction at call sites, while mutable raw
pointers make call-site side effects explicit (&). Using std::optional<T> for
optional value types avoids temporary lvalues at call sites.

## Examples

Bad:

```cpp
// Bad: pass non-trivial object by value; const ref for scalar; mutable ref
void trackLayerInfo(LayerInfo info) const;
static double AdaptiveEpsilon(const double& a, const double& b);
void populateProxies(EndCaps<kTileHeight>& ends);

// Bad: optional object by reference with &ref null check; non-const vector ref for out-param
AutoPropertyTracker(const AnimationBuilder* b, const skjson::ObjectValue& obj) { if (&obj) {} }
void collectExpressions(const Expression& expr, std::vector<const Expression*>& out);

// Bad: const optional ref for object; optional reference_wrapper for out-param; const pointer for optional value
void writeFields(const std::vector<Type::Field>& fields, const std::optional<MemoryLayout>& layout);
void recordDraw(..., std::optional<std::reference_wrapper<Layer*>> layer = {});
void performDeferredCleanup(std::chrono::milliseconds msNotUsed,
                            const std::chrono::milliseconds* stopPurging = nullptr);
```

Good:

```cpp
// Good: const ref for objects; scalar by value; raw pointer for mutable param
void trackLayerInfo(const LayerInfo& info) const;
static double AdaptiveEpsilon(const double a, const double b);
void populateProxies(EndCaps* ends);

// Good: optional object as const pointer; pointer to base TArray for out-param
AutoPropertyTracker(const AnimationBuilder* b, const skjson::ObjectValue* obj = nullptr) { if (obj) {} }
void collectExpressions(const Expression& expr, skia_private::TArray<const Expression*>* out);

// Good: const pointer defaulting to nullptr; pointer defaulting to nullptr for out-param; std::optional for value
void writeFields(const std::vector<Type::Field>& fields, const MemoryLayout* layout = nullptr);
void recordDraw(..., Layer** layer = nullptr);
void performDeferredCleanup(\n        std::chrono::milliseconds msNotUsed,\n        std::optional<std::chrono::milliseconds> stopPurging = std::nullopt);
```
