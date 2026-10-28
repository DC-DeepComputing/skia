---
name: skia/use-const-ref-for-read-only-local-aliases
description: When inspecting container elements, lookup results, or struct fields without mutating them, declare loop variables, local aliases, and pointers as const references or pointers-to-const
rubric: safety
severity: should_fix
---

**Rule.** When iterating over, inspecting, or aliasing container elements,
lookup results (such as map.find(...)), and struct fields without modifying
them, declare the loop variable, local alias, or pointer as a const reference
(const auto&, const T&) or pointer-to-const (const auto*, const T*). Avoid
binding read-only objects by mutable reference or pointer (auto&, T*) or copying
elements by value (auto / T), even when container iterators synthesize and
return temporaries by value.

**Why.** Binding read-only elements and container lookup results by const
reference or pointer-to-const communicates immutability, prevents accidental
mutation of referenced data, and enables compiler enforcement of
const-correctness. For range-for loops, const references eliminate redundant
copies when iterating over composite types; C++ reference lifetime extension
ensures that temporaries returned by value from iterators remain valid
throughout each loop iteration body without dangling.

## Examples

Bad:

```cpp
// Range-for loop copying elements by value:
for (auto pt : points) {
    SkRect r = SkRect::MakeLTRB(pt.fX - radius, pt.fY - radius,
                                pt.fX + radius, pt.fY + radius);
    device->drawOval(r, newPaint);
}

// Range-for loop copying by value due to mistaken lifetime concerns on value iterators:
for (const Event event : eventQueue) {
    sweepLine.handleEvent(event);
}

// Local alias declared as a mutable reference for read-only inspection:
for (int i = 0; i < cachedArray->size(); ++i) {
    MaskHashEntry& entry = (*cachedArray)[i];
    if (entry.fBounds.contains(iBounds)) {
        return entry.fLocator;
    }
}

// Container lookup result bound to a mutable pointer when only read:
SpvId* uniformId = fVariableMap.find((*p)->fSampler.get());
SkASSERT(uniformId);
fVariableMap.set(sampler, *uniformId);
```

Good:

```cpp
// Range-for loop binding elements by const reference:
for (const auto& pt : points) {
    SkRect r = SkRect::MakeLTRB(pt.fX - radius, pt.fY - radius,
                                pt.fX + radius, pt.fY + radius);
    device->drawOval(r, newPaint);
}

// Lifetime extension safely keeps iterator value temporaries alive across the iteration:
for (const Event& event : eventQueue) {
    sweepLine.handleEvent(event);
}

// Local alias declared as a const reference:
for (int i = 0; i < cachedArray->size(); ++i) {
    const MaskHashEntry& entry = (*cachedArray)[i];
    if (entry.fBounds.contains(iBounds)) {
        return entry.fLocator;
    }
}

// Container lookup result bound to a pointer-to-const:
const SpvId* uniformId = fVariableMap.find((*p)->fSampler.get());
SkASSERT(uniformId);
fVariableMap.set(sampler, *uniformId);
```
