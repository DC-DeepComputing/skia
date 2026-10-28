---
name: skia/prefer-optional-over-validity-flag
description: When declaring member variables for optional state, prefer std::optional<T> over validity flags or std::unique_ptr<T>, but keep always-present or default-seeded fields direct members
rubric: design
severity: should_fix
---

**Rule.** When declaring member variables for optional or conditionally
initialized state, prefer std::optional<T> over pairing separate or parallel
boolean status/validity flags (such as fIsValid, fIsFinalized, or fHasData) with
a value, and over heap-allocating via std::unique_ptr<T> when T is a complete,
non-polymorphic type. Conversely, do not wrap member variables in
std::optional<T> or lazy containers (such as SkTLazy<T>) if the field is always
present (never std::nullopt) or can be seeded directly with a known base or
default initial value (such as an accumulator or pipeline context); use a direct
member initialized with the default value or reset via an explicit
method/assignment.

**Why.** Using std::unique_ptr<T> for optional state introduces dynamic heap
allocation overhead and pointer indirection, while separate validity or status
flags create coupled, redundant state that requires manual synchronization and
risks uninitialized reads. Conversely, wrapping always-present or
default-seedable member variables in std::optional or SkTLazy adds unnecessary
abstraction, forces repetitive validity checks, and misleads readers into
expecting empty states.

## Examples

Bad:

```cpp
// Boolean validity or parallel status flags paired with value.
class Blitter {
    bool fCanDirectBlit, fDirectBlitValueIsValid;
    uint64_t fDirectBlitValue;
};
class PipelineData {
    BindBufferInfo fBufferInfo;
    bool fIsFinalized = false;
    bool fHasData = false;
};

// Heap-allocating optional state when T is complete and non-polymorphic.
class Codec {
    std::unique_ptr<DecodingState> fDecodingState;
};

// Wrapping always-present state in std::optional solely for reinitialization.
class Reader {
    std::optional<DecodingState> fDecodingState; // Always present, never nullopt
};

// Wrapping accumulator/context in optional/lazy container when default initial value exists.
class SkSVGFilterContext {
    SkTLazy<Result> fPreviousResult; // Checked with isValid() at every access site
};
```

Good:

```cpp
// Encapsulate optional validity and value together in std::optional<T>.
class Blitter {
    bool fCanDirectBlit;
    std::optional<uint64_t> fDirectBlitValue;
};
class PipelineData {
    std::optional<BindBufferInfo> fBufferInfo = std::nullopt;
};

// Store optional state inline with std::optional to avoid heap allocations.
class Codec {
    std::optional<DecodingState> fDecodingState;
};

// Direct member variable; reinitialize via reset() method or assignment.
class Reader {
    DecodingState fDecodingState; // Reset via fDecodingState.reset() or = {}
};

// Direct member seeded with initial/base value in constructor; accessed directly.
class SkSVGFilterContext {
    SkSVGFilterContext(...) : fPreviousResult({nullptr, region, SkSVGColorspace::kSRGB}) {}
    Result fPreviousResult;
};
```
