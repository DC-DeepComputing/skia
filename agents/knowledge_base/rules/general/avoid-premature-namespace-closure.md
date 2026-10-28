---
name: skia/avoid-premature-namespace-closure
description: When defining C++ namespace members, keep definitions inside open namespaces, omit redundant namespace prefixes on calls and types, and use nested namespaces for sub-components
rubric: readability
severity: should_fix
---

**Rule.** Do not prematurely close and reopen a named namespace in C++ source or
header files when defining consecutive functions or helpers belonging to that
namespace; keep definitions directly inside the open namespace block.
Additionally, when inside a namespace definition block, do not qualify member
function calls, helper invocations, or types with the enclosing namespace prefix
unless explicit disambiguation is required. Do not prefix declared type names
with their enclosing namespace (e.g., use struct TraceInfo rather than
SkSLTraceInfo inside SkSL), and avoid introducing namespace-prefixed
near-duplicates to resolve collisions; place specialized tools or sub-components
in a dedicated nested namespace instead (e.g., skvm::viz).

**Why.** Closing and reopening the same namespace fragments implementation
structure and forces function definitions to redundantly qualify identifiers
out-of-line. Repeating the enclosing namespace prefix on calls, types, or
declared names causes redundant stuttering (such as SkSL::SkSLTraceInfo) and
visual noise. Grouping secondary tools or sub-components into a dedicated nested
namespace cleanly resolves name collisions with parent symbols while preserving
concise, idiomatic type names.

## Examples

Bad:

```cpp
// Bad: Prematurely closing and reopening namespace forces out-of-line qualification.
    return ((c3 * t + c2) * t + c1) * t + c0;
}
}  // namespace Helpers

SkColor4f Helpers::EvaluateColorGainFunction(const ColorGain& g, const SkColor4f& c) {
    return Helpers::EvaluateMixingFunction(g.fMixing, c);
}
namespace Helpers {

// Bad: Redundantly qualifying calls with the enclosing namespace prefix.
namespace SkPngRustEncoder {
sk_sp<SkData> Encode(const SkPixmap& src, const Options& options) {
    SkDynamicMemoryWStream stream;
    if (!SkPngRustEncoder::Encode(&stream, src, options)) {
        return nullptr;
    }
    return stream.detachAsData();
}
}  // namespace SkPngRustEncoder

// Bad: Prefixing types with enclosing namespace or creating near-duplicates to avoid collisions.
namespace skvm {
struct SkVMInstruction { ... };
class SkVMVisualizer { ... };
}  // namespace skvm
```

Good:

```cpp
// Good: Keep consecutive definitions grouped inside the open namespace.
    return ((c3 * t + c2) * t + c1) * t + c0;
}

SkColor4f EvaluateColorGainFunction(const ColorGain& g, const SkColor4f& c) {
    return EvaluateMixingFunction(g.fMixing, c);
}

// Good: Call namespace members unqualified when already inside the namespace.
namespace SkPngRustEncoder {
sk_sp<SkData> Encode(const SkPixmap& src, const Options& options) {
    if (!Encode(&stream, src, options)) {
        return nullptr;
    }
    return stream.detachAsData();
}
}  // namespace SkPngRustEncoder

// Good: Omit prefix and use a dedicated nested namespace for sub-components or tools.
namespace skvm::viz {
struct Instruction { ... };
class Visualizer { ... };
}  // namespace skvm::viz
```
