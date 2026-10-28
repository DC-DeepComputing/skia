---
name: skia/prefer-aggregate-initialization-in-push-back
description: When appending elements to containers, construct them directly using braced initialization ({...} or {}) in push_back/push_back_n instead of separate member assignments or single-use temporaries
rubric: readability
severity: nit
---

**Rule.** When appending or inserting elements into a container (such as
skia_private::TArray, STArray, or std::vector via push_back or push_back_n),
construct the element inline using braced initialization directly in the
call—passing {field1, ...} (with std::move on sink arguments) for aggregate
structs, or {} directly for default-constructed placeholders. Do not append an
uninitialized element to assign its members across separate statements, and
avoid declaring single-use named temporary variables or local constants purely
to supply a default-constructed or initialized instance.

**Why.** Inlining braced initialization directly into push_back and push_back_n
eliminates redundant temporary variable and constant declarations, reduces
boilerplate type repetition, and keeps insertions concise. Constructing members
directly also avoids leaving uninitialized or partially initialized objects in
the container across multiple assignments, and passing {} directly makes
placeholder intent clear without intermediate clutter.

## Examples

Bad:

```cpp
// Member-by-member assignment across multiple statements
Instruction& inst = fInstructions.push_back();
inst.fOp = op;
inst.fSlotA = slotA;

// Redundant single-use named temporary variable
ValuePair<SkColor*> p = ValuePair<SkColor*>{colorValue, node};
fColorMap[slotID].push_back(p);

// Redundant local constant for a default-constructed placeholder
static constexpr SamplerDesc kUninitializedSamplerDesc {};
outDescs->push_back_n(renderStepSamplerCount, kUninitializedSamplerDesc);
```

Good:

```cpp
// Inline aggregate initialization directly in push_back
fInstructions.push_back({op, slotA, slotB});

// Inline aggregate construction with std::move on sink arguments
fColorMap[slotID].push_back({colorValue, std::move(node)});

// Pass '{}' directly for default-constructed placeholder elements
outDescs->push_back_n(renderStepSamplerCount, {});
```
