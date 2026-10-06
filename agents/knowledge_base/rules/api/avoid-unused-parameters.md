---
name: skia/api/avoid-unused-parameters
rubric: design
severity: should_fix
---

**Rule.** Do not declare or retain unused parameters, unconsumed out-parameters,
speculative plumbing, obsolete configuration flags, dead shader inputs, or
unreferenced macro parameters in functions, constructors, internal helpers,
shader routines, or function-like macros; remove dead parameters from
signatures, macro definitions, call sites, and attribute layouts along with
calculations that solely produce unread outputs. When an unused parameter is
required by a virtual interface, method override, callback signature, or
unimplemented stub, omit or comment out its name in the definition (such as
/*param*/ or [[maybe_unused]]) to prevent -Wunused-parameter compiler warnings.

**Why.** Unused parameters, unconsumed out-parameters, dead shader attributes,
and speculative plumbing mislead readers about active state or data
dependencies, waste cycles computing discarded outputs, and clutter call sites
with dummy arguments. In shader routines, unused parameters unnecessarily
consume attribute registers or uniform buffer bandwidth. In macro definitions,
unreferenced parameters force callers to pass dummy values and can mask
scope-capture bugs. In interface implementations, method overrides, or stub
definitions, retaining unused parameter names triggers compiler warnings
(-Wunused-parameter) across toolchains.

## Examples

Bad:

```cpp
// Dead parameter in helper, macro, or shader routine:
void processItem(Item* item, bool unusedFlag) {
    item->process();
}

#define TRACE_STEP(step, scanlines, pixels) \
    step.add(/*scanlines=*/0, /*pixels=*/pixels)

float4 vertex_fn(float2 pos, float2 center /* unused */, float depth) {
    return float4(pos, depth, 1.0);
}

// Named unused parameter in stub or method override:
std::unique_ptr<SkScalerContext> SkTypeface::onCreateScalerContext(
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc) const override {
    SK_ABORT("Not implemented.");
}
```

Good:

```cpp
// Remove dead parameter from helper, macro, shader routine, and call sites:
void processItem(Item* item) {
    item->process();
}

#define TRACE_STEP(step, pixels) \
    step.add(/*scanlines=*/0, /*pixels=*/pixels)

float4 vertex_fn(float2 pos, float depth) {
    return float4(pos, depth, 1.0);
}

// Omit parameter names in stub or method override:
std::unique_ptr<SkScalerContext> SkTypeface::onCreateScalerContext(
        const SkScalerContextEffects&,
        const SkDescriptor*) const override {
    SK_ABORT("Not implemented.");
}
```
