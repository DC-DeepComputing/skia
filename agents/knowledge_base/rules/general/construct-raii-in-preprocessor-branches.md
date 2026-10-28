---
name: skia/construct-raii-in-preprocessor-branches
description: When acquiring resources across preprocessor branches, construct RAII smart pointers directly within each branch instead of using temporary raw pointers
rubric: safety
severity: should_fix
---

**Rule.** Construct RAII smart pointers or resource handles directly within each
branch of mutually exclusive #if / #else preprocessor conditionals rather than
assigning to a temporary unowned raw pointer across the preprocessor block. Wrap
allocating C API calls into their owning RAII handles at the exact point of
creation.

**Why.** Holding an allocated resource in an unowned raw pointer creates an
unmanaged lifetime window that risks leaking resources on early exits, condition
checks, or future refactoring. Because preprocessor branches are mutually
exclusive at compile time, declaring the RAII handle directly within both
branches safely initializes the variable in the outer lexical scope immediately
upon acquisition.

## Examples

Bad:

```cpp
#if defined(SK_HARFBUZZ_SUBSET_NEW_API)
    hb_face_t* subset_face = hb_subset_or_fail(face.get(), input.get());
#else
    hb_face_t* subset_face = hb_subset(face.get(), input.get());
#endif
    if (!subset_face) {
        return nullptr;
    }
    HBFace subset(subset_face);
```

Good:

```cpp
#if defined(SK_HARFBUZZ_SUBSET_NEW_API)
    HBFace subset(hb_subset_or_fail(face.get(), input.get()));
#else
    HBFace subset(hb_subset(face.get(), input.get()));
#endif
    if (!subset) {
        return nullptr;
    }
```
