---
name: skia/initialize-alignment-tracking-to-one
description: When tracking or querying alignment requirements, initialize trackers to 1 rather than 0 and omit defensive zero checks or ternary fallbacks at call sites
rubric: readability
severity: should_fix
---

**Rule.** Initialize alignment tracking and requirement variables to 1 rather
than 0, as 1-byte alignment is an identity no-op that satisfies non-zero
power-of-two assertions (such as in SkAlignTo). In callers querying alignment
calculators (such as UniformOffsetCalculator::requiredAlignment()), omit
redundant zero checks or fallback ternaries (like !alignment ? 1 : alignment),
relying on the calculator's guaranteed non-zero minimum alignment.

**Why.** Zero is an invalid power of two in alignment helpers like SkAlignTo and
causes assertion failures. Initializing tracking variables to 1 satisfies
non-zero assertions and keeps progressive alignment operations (like std::max)
valid without branching; conversely, adding defensive zero checks or fallback
ternaries when querying helpers guaranteed to return at least 1 introduces
redundant code clutter.

## Examples

Bad:

```cpp
// Bad: Initializing to 0 requires defensive branches before aligning.
int fReqAlignment = 0;
...
if (fReqAlignment > 0) {
    this->alignTo(fReqAlignment);
}

// Bad: Redundant fallback ternary when querying a calculator guaranteed to return >= 1.
fUniformAlignment = calculator.requiredAlignment();
fUniformAlignment = !fUniformAlignment ? 1 : fUniformAlignment;
```

Good:

```cpp
// Good: Initializing to 1 satisfies power-of-two checks; aligning to 1 is a no-op.
int fReqAlignment = 1;
...
this->alignTo(fReqAlignment);

// Good: Querying requiredAlignment() is guaranteed to return at least 1.
fUniformAlignment = calculator.requiredAlignment();
```
