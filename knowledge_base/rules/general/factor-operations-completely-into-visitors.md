---
name: skia/factor-operations-completely-into-visitors
description: When using visitors or double dispatch to factor operations off data structures, implement operations entirely in external functions rather than keeping hybrid member methods on select containers
rubric: design
severity: should_fix
---

**Rule.** When using visitor patterns, double dispatch, or external overloads to
factor operations (such as serialization, emission, or formatting) off of a data
hierarchy or variant type, implement the operation completely through external
free functions or visitor overloads. Do not retain operation-specific member
methods on select composite or container classes while visiting other types
externally; keep data structures as pure containers.

**Why.** Retaining member methods on some container classes while dispatching
operations externally for others creates an inconsistent hybrid design and
violates separation of concerns. Factoring operations completely into external
visitor overloads keeps data classes decoupled from specific operational
concerns and ensures uniform dispatch semantics across all types in the
hierarchy.

## Examples

Bad:

```cpp
class SkPDFArray {
public:
    void emit(SkWStream* stream) const;
};

// Inside visitor:
if constexpr (std::is_same_v<T, std::unique_ptr<SkPDFArray>>) {
    arg->emit(stream);
}
```

Good:

```cpp
class SkPDFArray {
public:
    const std::vector<SkPDFObject>& elements() const;
};

void SkPDFEmit(const SkPDFArray& array, SkWStream* stream);

// Inside visitor:
skstd::visit([stream](const auto& v) { emit(v, stream); }, obj);
```
