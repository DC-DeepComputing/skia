---
name: skia/gpu/gpu-name-explicit-draw-state-enum-modes
description: When defining GPU pipeline and draw state enum flags, name distinct modes explicitly instead of using ambiguous identifiers like kIsDynamic or relying on implicit negative states
rubric: readability
severity: should_fix
---

**Rule.** When defining enum flags or bitmasks for GPU draw and pipeline states,
avoid ambiguous flag names like `kIsDynamic` when multiple concepts of dynamism
exist (such as dynamic data appends versus flexible instance vertex counts).
Define explicit, descriptive enum values for every distinct mode (such as
`kFixed`, `kAppendVertices`, `kAppendInstances`, and `kAppendDynamicInstances`)
rather than relying on the absence of a flag as an implicit negative fallback.

**Why.** In GPU draw and state management pipelines, terms like 'dynamic' can
refer to competing concepts: runtime data appending versus varying geometry per
instance. Furthermore, encoding mutually exclusive modes with binary toggles
where an unset bit implicitly represents an alternative mode obscures dispatch
logic and prevents adding new modes (such as fixed non-appending draws).

## Examples

Bad:

```cpp
enum class RenderStateFlags : unsigned {
    kNone           = 0b00,
    kAppendVertices = 0b01,  // Absence of flag implicitly implies instances
    kIsDynamic      = 0b10,  // Ambiguous: dynamic data appends or dynamic instances?
};
```

Good:

```cpp
enum class RenderStateFlags : unsigned {
    kNone                   = 0b0000,
    kFixed                  = 0b0001,   // Uses explicit DrawWriter::draw functions
    kAppendVertices         = 0b0010,   // Appends vertices
    kAppendInstances        = 0b0100,   // Appends instances with static vertex count
    kAppendDynamicInstances = 0b1000,   // Appends instances with a flexible vertex count
};
SK_MAKE_BITMASK_OPS(RenderStateFlags)
```
