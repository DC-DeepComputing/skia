---
name: skia/prefer-make-empty-factory-for-empty-instances
description: When constructing empty instances or fallbacks for types supporting empty states, provide and use a static MakeEmpty() factory rather than bare default constructors
rubric: readability
severity: should_fix
---

**Rule.** When defining or instantiating types that support a distinct empty
state (such as geometric primitives, data buffers, or metadata containers),
provide and use a static MakeEmpty() factory method instead of calling a bare
default constructor Type() or empty braces {}. Use Type::MakeEmpty() at call
sites (including fallback branches, default parameters, and ternary expressions)
to explicitly communicate that the object is intentionally empty.

**Why.** Invoking a bare default constructor Type() or {} at call sites leaves
it ambiguous whether an object is uninitialized, default-configured, or
intentionally empty. Providing and calling a named MakeEmpty() static factory
method conforms to standard Skia conventions and makes intent explicit and
self-documenting.

## Examples

Bad:

```cpp
// Ambiguous empty construction in fallback branch
Metadata meta = fChunkReader ? fChunkReader->getHdrMetadata() : skhdr::Metadata();

// Ambiguous return of empty geometry or data
return SkRect();
```

Good:

```cpp
// Explicit static factory clarifies empty intent
Metadata meta = fChunkReader ? fChunkReader->getHdrMetadata() : skhdr::Metadata::MakeEmpty();

// Idiomatic empty instance construction
return SkRect::MakeEmpty();
```
