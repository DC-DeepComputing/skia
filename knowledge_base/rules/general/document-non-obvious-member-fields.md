---
name: skia/document-non-obvious-member-fields
description: When declaring struct fields, member variables, or configuration options, document their purpose, units, coordinate spaces, non-obvious flags, and structural deviations directly at declaration
rubric: readability
severity: should_fix
---

**Rule.** Document non-obvious struct fields, member variables, configuration
options, and sorting records with comments explaining their purpose, valid
ranges, coordinate spaces, measurement units, orientation conventions, and
ownership semantics. On public structs, document field semantics, units, and
non-obvious flags directly on the struct declaration rather than relying on
consuming APIs. When fields diverge from enclosing type conventions (such as
placing a boolean flag in a proc table), are reinterpreted for non-standard
semantics, or participate in sorted records, document the design rationale,
encoding invariants, or key-versus-payload roles. Omit redundant comments that
merely restate field names.

**Why.** Undocumented, vaguely described, or cryptic field comments force
callers and maintainers to guess units, coordinate spaces, orientation
conventions, ownership relationships, or key ordering across multiple call
sites. Documenting semantics, units, and structural design rationales directly
on struct fields prevents callers from assuming default conventions or
misunderstanding atypical members, while explicit field roles prevent bugs in
comparators and refactoring.

## Examples

Bad:

```cpp
// include/core/SkArc.h
struct SkArc {
    SkRect   fOval;
    SkScalar fStartAngle;
    SkScalar fSweepAngle;
    bool     fUseCenter;
};

// include/core/SkSerialProcs.h (deviates from proc-table convention without explanation)
struct SK_API SkDeserialProcs {
    SkDeserialPictureProc fPictureProc = nullptr;
    void*                 fPictureCtx  = nullptr;
    bool                  fAllowSkSL   = true;
};

// src/gpu/graphite/DrawListBase.h (repurposed field without documenting non-standard semantics)
BindBufferInfo fLastBinding;

// modules/bentleyottmann/src/Myers.cpp
struct SetupTuple {
    int32_t yOrdering;
    Segment originalSegment; // Along to be sorted.
};
```

Good:

```cpp
// include/core/SkArc.h
struct SkArc {
    // Bounds of oval containing the arc.
    SkRect   fOval;
    // Angle in degrees where the arc begins; zero is horizontally to the right.
    SkScalar fStartAngle;
    // Sweep angle in degrees; positive is clockwise.
    SkScalar fSweepAngle;
    // True draws lines from oval center to arc ends (wedge); false draws arc perimeter only.
    bool     fUseCenter;
};

// include/core/SkSerialProcs.h
struct SK_API SkDeserialProcs {
    SkDeserialPictureProc fPictureProc = nullptr;
    void*                 fPictureCtx  = nullptr;
    // Represents a parameterless proc returning bool; direct bool avoids callback overhead.
    bool                  fAllowSkSL   = true;
};

// src/gpu/graphite/DrawListBase.h
// Tracks last binding; under SSBOs, fOffset holds storage buffer index rather than byte offset.
BindBufferInfo fLastBinding;

// modules/bentleyottmann/src/Myers.cpp
struct SetupTuple {
    int32_t yOrdering;
    // Carried payload; sorted alongside tuples but does not participate in comparison key.
    Segment originalSegment;
};
```
