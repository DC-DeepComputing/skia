---
name: skia/prefix-data-members-with-f
description: When declaring data members in classes, internal helper structs, or spec-mirroring structs, prefix fields with 'f' rather than bare names, snake_case, or 'k'
rubric: readability
severity: should_fix
---

**Rule.** Prefix data member names in classes, structs with methods, and
internal helper structs with a lowercase 'f' followed by CamelCase (e.g.,
fDirectContext, fMaskRowLut, fDualSrcBlend), rather than bare identifiers,
snake_case, or trailing underscores. Do not use the 'k' constant prefix for
instance member fields even if declared const, as 'k' is reserved for
program-duration constants. Adhere to the 'f' prefix convention even when struct
fields represent or mirror external API or specification feature names (such as
Vulkan physical device features). Standalone passive data-holder structs
predominantly intended for direct field access (such as public POD types) may
omit the 'f' prefix, but internal helper structs must maintain 'f' for
consistency.

**Why.** Skia's naming convention distinguishes member fields from local
variables, arguments, and global variables at the point of use without requiring
this-> qualification. The 'k' prefix is strictly reserved for program-duration
constants; using 'k' on instance fields misleadingly implies compile-time or
static immutability. Adhering to the 'f' prefix even when mirroring external
specifications maintains uniform Skia style across internal capability and
helper structs.

## Examples

Bad:

```cpp
class SkSharingSerialContext {
    GrDirectContext* dContext;
    const uint32_t kMaxImageSize;
};

struct EnabledFeatures {
    bool dualSrcBlend = false;
    bool samplerYcbcrConversion = false;
};

struct LineStepParams {
    const uint8_t* maskRowLut;
    int32_t stepXFixed;
    bool xDir;
};
```

Good:

```cpp
class SkSharingSerialContext {
    GrDirectContext* fDirectContext;
    const uint32_t fMaxImageSize;
};

struct EnabledFeatures {
    bool fDualSrcBlend = false;
    bool fSamplerYcbcrConversion = false;
};

struct LineStepParams {
    const uint8_t* fMaskRowLut;
    int32_t fStepXFixed;
    bool fSortedXDir;
};
```
