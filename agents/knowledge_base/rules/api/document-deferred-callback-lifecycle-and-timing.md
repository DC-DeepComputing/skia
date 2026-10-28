---
name: skia/api/document-deferred-callback-lifecycle-and-timing
description: When introducing APIs accepting deferred or lazy callbacks, document their invocation lifecycle, execution timing, and ordering guarantees
rubric: readability
severity: nit
---

**Rule.** When declaring APIs or factory functions that accept deferred or lazy
callbacks, document their invocation lifecycle, execution timing, and ordering
guarantees in header comments. Specify when and how frequently the callback
executes, whether invocation order is preserved, and whether auxiliary
operations (such as serialization) trigger eager evaluation.

**Why.** Callers of deferred callback APIs cannot deduce from type signatures
alone when or how often a callback executes, or whether operations like
serialization will trigger eager evaluation. Omitting these execution contracts
leads callers to rely on accidental implementation details or encounter bugs
from invalid captured state.

## Examples

Bad:

```cpp
// No comments describing when or how often the callback is executed:
using UniformsCallback = std::function<sk_sp<const SkData>(const UniformsCallbackContext&)>;
static sk_sp<SkShader> MakeDeferredShader(const SkRuntimeEffect* effect,
                                          UniformsCallback uniformsCallback,
                                          SkSpan<SkRuntimeEffect::ChildPtr> children,
                                          const SkMatrix* localMatrix = nullptr);
```

Good:

```cpp
// Document invocation lifecycle, execution timing, and serialization side effects:
// Private (experimental) API for creating runtime shaders with late-bound uniforms.
// The callback must produce a uniform data blob of the correct size for the effect.
// It is invoked at "draw" time (essentially, when a draw call is made against the canvas
// using the resulting shader). There are no strong guarantees about timing.
// Serializing the resulting shader will immediately invoke the callback (and record the
// resulting uniforms).
using UniformsCallback = std::function<sk_sp<const SkData>(const UniformsCallbackContext&)>;
```
