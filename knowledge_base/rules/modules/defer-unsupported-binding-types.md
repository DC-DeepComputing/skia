---
name: skia/modules/defer-unsupported-binding-types
description: When binding C++ APIs in CanvasKit, do not add binding functions or core overloads for types not yet cleanly supported; keep bindings minimal
rubric: design
severity: should_fix
---

**Rule.** When exposing Skia features in CanvasKit Emscripten bindings, do not
introduce binding functions or one-off core Skia C++ overloads for features
whose underlying types (such as polymorphic SkRuntimeEffect::ChildPtr or child
effect arrays) are not yet cleanly supported in the binding layer. Defer complex
binding methods until type mapping is deliberately designed, and keep bindings
minimal to the subset of functionality that is fully and cleanly supported.

**Why.** Adding wrapper methods before the binding layer's type system can
properly represent them produces awkward, unidiomatic JavaScript APIs and often
forces narrow, redundant overloads into core Skia C++ headers. Keeping the
binding surface minimal avoids premature API commitments and allows proper type
support to be designed holistically.

## Examples

Bad:

```cpp
// Adding a narrow C++ overload to SkRuntimeEffect.h and exposing _makeBlenderWithChildren:
sk_sp<SkBlender> makeBlender(sk_sp<const SkData> uniforms,
                             sk_sp<SkBlender> children[],
                             size_t childCount) const;

.function("_makeBlenderWithChildren", optional_override([](SkRuntimeEffect& self, ...) {
    return self.makeBlender(uniforms, children, count);
}));
```

Good:

```cpp
// Keeping CanvasKit bindings minimal to cleanly supported types; deferring child effects:
.function("_makeBlender", optional_override([](SkRuntimeEffect& self,
                                               WASMPointerF32 uniformPtr,
                                               int uniformCount) {
    auto uniforms = copyUniforms(self, uniformPtr, uniformCount);
    return self.makeBlender(std::move(uniforms), /*children=*/{});
}));
```
